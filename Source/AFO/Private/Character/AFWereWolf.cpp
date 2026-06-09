#include "Character/AFWereWolf.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "Components/AFAttributeComponent.h"
#include "Player/AFPlayerState.h"
#include "Components/AFSkillComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/OverlapResult.h"
#include "Player/AFPlayerController.h"


void AAFWereWolf::BeginPlay()
{
	Super::BeginPlay();
	LoadWereWolfData();
	
}

AAFWereWolf::AAFWereWolf()
{
	PrimaryActorTick.bCanEverTick = false;
	SkillComponent = CreateDefaultSubobject<UAFSkillComponent>(TEXT("SkillComponent"));
}

// 패시브: 평타 피흡 (공격 적중 시 외부에서 호출)
void AAFWereWolf::ApplyLifeSteal()
{
	if (!HasAuthority()) return;

	if (UAFAttributeComponent* Attr = FindComponentByClass<UAFAttributeComponent>())
	{
		Attr->ApplyHealthChange(10);	// 10 회복
	}
}

// Q 스킬: 이속 증가
void AAFWereWolf::Server_ApplySpeedBoost_Implementation(float Ratio, float Duration)
{
	if (!HasAuthority()) return;

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!MoveComp) return;

	// 기존 속도 저장
	if (OriginalWalkSpeed == 0.f)
		OriginalWalkSpeed = MoveComp->MaxWalkSpeed;

	// 30% 증가 → *1.3f
	MoveComp->MaxWalkSpeed = OriginalWalkSpeed * Ratio;

	// 타이머 초기화 후 만료 시 원복
	GetWorld()->GetTimerManager().ClearTimer(SpeedBoostTimer);
	GetWorld()->GetTimerManager().SetTimer(
		SpeedBoostTimer,
		this,
		&AAFWereWolf::ResetSpeedBoost,
		Duration,
		false
	);

	UE_LOG(LogTemp, Warning, TEXT("[WereWolf] SpeedBoost: %.2f for %.2fs"), Ratio, Duration);
}

void AAFWereWolf::ResetSpeedBoost()
{
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = OriginalWalkSpeed;
	}
}


// E 스킬: 출혈 데미지 (초당 반복)

void AAFWereWolf::Server_ApplyBleeding_Implementation(AActor* Target, float DamagePerSec, float Duration)
{
	if (!HasAuthority() || !Target) return;

	int32 TickCount = FMath::FloorToInt(Duration); // Duration=3 → 3초

	GetWorld()->GetTimerManager().SetTimer(
		BleedingTimer,
		FTimerDelegate::CreateLambda([=,this]()
		{
			ApplyBleedingTick(Target, DamagePerSec);
		}),
		1.0f,
		true
	);

	// Duration 후 정지
	FTimerHandle StopHandle;
	GetWorld()->GetTimerManager().SetTimer(
		StopHandle,
		[this]()
		{
			GetWorld()->GetTimerManager().ClearTimer(BleedingTimer);
		},
		Duration,
		false
	);
}

void AAFWereWolf::ApplyBleedingTick(AActor* Target, float DamagePerSec)
{
	if (!HasAuthority() || !Target) return;

	if (UAFAttributeComponent* Attr = Target->FindComponentByClass<UAFAttributeComponent>())
	{
		Attr->ApplyDamage(DamagePerSec, GetController());
	}

	UE_LOG(LogTemp, Warning, TEXT("[WereWolf] Bleeding: -%f"), DamagePerSec);
}






void AAFWereWolf::LoadWereWolfData()
{
	// 서버/클라이언트 모두에서 실행되어야 함
	static const FString ContextString(TEXT("WereWolfDataContext"));
	UE_LOG(LogTemp, Log, TEXT("@@@ LoadWereWolfData Started (NetMode: %d)"), GetNetMode());

	if (SkillDataTable)
	{
		if (FAFSkillInfo* QRow = SkillDataTable->FindRow<FAFSkillInfo>(TEXT("WereWolf_Q"), ContextString))
		{
			QSkillData = *QRow;
		}
		else { UE_LOG(LogTemp, Error, TEXT("@@@ [Data] Failed to find Mage_Q in DataTable!")); }
	}
	else { UE_LOG(LogTemp, Error, TEXT("@@@ [Data] SkillDataTable is NOT Assigned!")); }

	if (SkillDataTable)
	{
		if (FAFSkillInfo* ERow = SkillDataTable->FindRow<FAFSkillInfo>(TEXT("WereWolf_E"), ContextString))
		{
			ESkillData = *ERow;
			UE_LOG(LogTemp, Log, TEXT("@@@ [Data] Mage_E Loaded - Cooldown: %f"), ESkillData.Cooldown);
		}
	}

	if (FAFSkillInfo* HeavyRow = SkillDataTable->FindRow<FAFSkillInfo>(TEXT("WereWolf_Right"), ContextString))
	{
		HeavyAttackData = *HeavyRow;
	}

	// 2. 서버일 때만 PlayerState 스탯 설정
	if (HasAuthority())
	{
		AAFPlayerState* AFPS = GetPlayerState<AAFPlayerState>();
		if (AFPS && StatDataTable)
		{
			FAFPlayerCharacterStatRow* StatRow = StatDataTable->FindRow<FAFPlayerCharacterStatRow>(TEXT("WereWolf"), ContextString);
			if (StatRow)
			{
				AFPS->SetHealth(StatRow->Maxhp, StatRow->Maxhp);
				AFPS->SetMana(StatRow->Maxmana, StatRow->Maxmana);
				UE_LOG(LogTemp, Warning, TEXT("MaxHP Mana Binding!!"));
			}
		}
	}
}

void AAFWereWolf::ServerRPC_SkillE_Implementation()
{
	if (bIsAttacking || bIsUsingSkill) return;

	if (SkillComponent == nullptr)
	{
		SkillComponent = FindComponentByClass<UAFSkillComponent>();
	}

	if (SkillComponent && SkillComponent->CanUseSkill(TEXT("WereWolf_E")))
	{
		AAFPlayerState* PS = GetPlayerState<AAFPlayerState>();
		if (PS && PS->ConsumeMana(ESkillData.ManaCost))
		{
			bIsUsingSkill = true;
			SkillComponent->StartCooldown(TEXT("WereWolf_E"), ESkillData.Cooldown);

			Multicast_PlaySkillEMontage();
		}
	}
}

void AAFWereWolf::ServerRPC_SkillQ_Implementation()
{
	// 1. 컴포넌트 존재 확인 로그
	if (SkillComponent == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("@@@ [Server] SkillComponent is NULL. Trying to find..."));
		SkillComponent = FindComponentByClass<UAFSkillComponent>();
	}

	if (SkillComponent)
	{
		// 2. 쿨타임 가능 여부 확인 로그
		bool bCanUse = SkillComponent->CanUseSkill(TEXT("WereWolf_Q"));
		UE_LOG(LogTemp, Log, TEXT("@@@ [Server] CanUseSkill(WereWolf_Q): %s"), bCanUse ? TEXT("True") : TEXT("False"));

		if (bCanUse)
		{
			AAFPlayerState* PS = GetPlayerState<AAFPlayerState>();
			if (PS)
			{
				// 3. 마나 및 데이터 확인 로그
				UE_LOG(LogTemp, Log, TEXT("@@@ [Server] Attempting Mana Consume: %f / Current: %f"), QSkillData.ManaCost, PS->GetMaxMana());

				if (PS->ConsumeMana(QSkillData.ManaCost))
				{
					bIsUsingSkill = true;
					SkillComponent->StartCooldown(TEXT("WereWolf_Q"), QSkillData.Cooldown);
					UE_LOG(LogTemp, Log, TEXT("@@@ [Server] Skill Q Success!"));
				}
				else { UE_LOG(LogTemp, Error, TEXT("@@@ [Server] Mana Shortage!")); }
			}
			else { UE_LOG(LogTemp, Error, TEXT("@@@ [Server] PlayerState is NULL!")); }
		}
	}
	else
	{
		UE_LOG(LogTemp, Fatal, TEXT("@@@ [Server] CRITICAL: SkillComponent not found on WereWolf!"));
	}
}

void AAFWereWolf::HandleOnCheckHit()
{
	if (!HasAuthority()) return;

	UAnimInstance* Anim = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (!Anim) return;

	// 1) 일반 공격
	if (Anim->Montage_IsPlaying(AttackMontage))
	{
		TArray<FOverlapResult> OverlapResults;
		FCollisionShape Sphere = FCollisionShape::MakeSphere(100.f);
		FVector TraceLocation = GetActorLocation() + GetActorForwardVector() * 100.f;

		if (GetWorld()->OverlapMultiByChannel(OverlapResults, TraceLocation, FQuat::Identity, ECC_Pawn, Sphere))
		{
			for (auto& Result : OverlapResults)
			{
				AActor* HitActor = Result.GetActor();
				if (HitActor && HitActor != this && !IsAlly(HitActor))
				{
					if (UAFAttributeComponent* Attr = HitActor->FindComponentByClass<UAFAttributeComponent>())
					{
						Attr->ApplyDamage(AttackDamage, GetController());
					}

				}
			}
		}
	}


	// 2) E 스킬 판정(필요 시)
	else if (Anim->Montage_IsPlaying(SkillEMontage))
	{
		// 필요하면 추가
	}

	// 3) Q 스킬 판정 + 이펙트
	else if (Anim->Montage_IsPlaying(SkillQMontage))
	{
		{
			// HandleSkillHitCheck(QSkillData.SkillRange * 0.5f, QSkillData.DaWereWolf * 0.2f, 0.f);
		}
	}

	// 4) 강공격
	else if (HeavyAttackMontage && Anim->Montage_IsPlaying(HeavyAttackMontage))
	{
		if (SkillComponent == nullptr)
		{
			SkillComponent = FindComponentByClass<UAFSkillComponent>();
		}

		if (SkillComponent)
		{
			bool bCanUse = SkillComponent->CanUseSkill(TEXT("WereWolf_Right"));

			if (bCanUse)
			{
				bIsUsingSkill = true;
				SkillComponent->StartCooldown(TEXT("WereWolf_Right"), HeavyAttackData.Cooldown);
				HandleSkillHitCheck(150, 60.f, 0.f);
			}
		}
		if (Anim->Montage_IsPlaying(HeavyAttackMontage))
		{
			TArray<FOverlapResult> OverlapResults;
			FCollisionShape Sphere = FCollisionShape::MakeSphere(100.f);
			FVector TraceLocation = GetActorLocation() + GetActorForwardVector() * 100.f;
			if (GetWorld()->OverlapMultiByChannel(OverlapResults, TraceLocation, FQuat::Identity, ECC_Pawn, Sphere))
			{
				for (auto& Result : OverlapResults)
				{
					AActor* HitActor = Result.GetActor();
					if (HitActor && HitActor != this && !IsAlly(HitActor))
					{
						if (UAFAttributeComponent* Attr = HitActor->FindComponentByClass<UAFAttributeComponent>())
						{
							Attr->ApplyDamage(100.f, GetController());
						}
					}
				}
			}
		}

		

	}
}

void AAFWereWolf::HandleSkillHitCheck(float Radius, float Damage, float RotationOffset)
{
	if (!HasAuthority()) return;

	FVector SkillDirection = GetActorForwardVector().RotateAngleAxis(RotationOffset, FVector(0, 0, 1));
	FVector SphereLocation = GetActorLocation() + (SkillDirection * 150.f) + FVector(0, 0, 60.f);

	TArray<FOverlapResult> OverlapResults;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(Radius);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->OverlapMultiByChannel(OverlapResults, SphereLocation, FQuat::Identity, ECC_Pawn, Sphere, Params);

	if (bHit)
	{
		for (const FOverlapResult& Result : OverlapResults)
		{
			AActor* HitActor = Result.GetActor();
			if (HitActor && !IsAlly(HitActor))
			{
				if (UAFAttributeComponent* Attr = HitActor->FindComponentByClass<UAFAttributeComponent>())
				{
					Attr->ApplyDamage(Damage, GetController());
				}

				if (AAFPlayerCharacter* Victim = Cast<AAFPlayerCharacter>(HitActor))
				{
					Victim->TriggerHitReact_FromAttacker(this);
				}
			}
		}
	}
}

void AAFWereWolf::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	LoadWereWolfData();

	if (SkillComponent == nullptr)
	{
		SkillComponent = FindComponentByClass<UAFSkillComponent>();
	}

	// [추가] 로컬 컨트롤러라면 UI에 내 컴포넌트를 주입
	if (IsLocallyControlled())
	{
		// PlayerController를 통해 메인 위젯을 가져오는 로직 (본인 프로젝트 구조에 맞게 수정)
		AAFPlayerController* PC = Cast<AAFPlayerController>(GetController());
		if (PC)
		{
			// 위젯의 초기화 함수를 다시 호출하여 SkillComponent를 전달
			PC->RefreshSkillUI(SkillComponent);
		}
	}
}




