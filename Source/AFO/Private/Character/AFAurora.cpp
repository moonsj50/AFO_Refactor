#include "Character/AFAurora.h"
#include "GameFramework/Controller.h"
#include "Player/AFPlayerState.h"
#include "Components/AFSkillComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/OverlapResult.h"
#include "Player/AFPlayerController.h"
#include "Components/AFAttributeComponent.h"

AAFAurora::AAFAurora()
{
	PrimaryActorTick.bCanEverTick = true;

	AimYaw   = 0.f;
	AimPitch = 0.f;
	AimAlpha = 1.f;
	SkillComponent = CreateDefaultSubobject<UAFSkillComponent>(TEXT("SkillComponent"));
}

void AAFAurora::StartSprint(const FInputActionValue& Value)
{
	Super::StartSprint(Value);

	bIsSprinting = true;
}

void AAFAurora::StopSprint(const FInputActionValue& Value)
{
	Super::StartSprint(Value);

	bIsSprinting = false;
}

void AAFAurora::BeginPlay()
{
	Super::BeginPlay();
	LoadAuroraData();
}

void AAFAurora::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!Controller) return;

	const FRotator ControlRot = Controller->GetControlRotation();
	const FRotator ActorRot   = GetActorRotation();
	const FRotator DeltaRot  = (ControlRot - ActorRot).GetNormalized();

	AimYaw   = DeltaRot.Yaw;
	AimPitch = FMath::Clamp(DeltaRot.Pitch, -90.f, 90.f);
	AimAlpha = 1.f;   // Idle 상태라 항상 켜둠
}



void AAFAurora::LoadAuroraData()
{
	// 서버/클라이언트 모두에서 실행되어야 함
	static const FString ContextString(TEXT("AuroraDataContext"));
	UE_LOG(LogTemp, Log, TEXT("@@@ LoadAuroraData Started (NetMode: %d)"), GetNetMode());

	if (SkillDataTable)
	{
		if (FAFSkillInfo* QRow = SkillDataTable->FindRow<FAFSkillInfo>(TEXT("Aurora_Q"), ContextString))
		{
			QSkillData = *QRow;
		}
		else { UE_LOG(LogTemp, Error, TEXT("@@@ [Data] Failed to find Mage_Q in DataTable!")); }
	}
	else { UE_LOG(LogTemp, Error, TEXT("@@@ [Data] SkillDataTable is NOT Assigned!")); }

	if (SkillDataTable)
	{
		if (FAFSkillInfo* ERow = SkillDataTable->FindRow<FAFSkillInfo>(TEXT("Aurora_E"), ContextString))
		{
			ESkillData = *ERow;
			UE_LOG(LogTemp, Log, TEXT("@@@ [Data] Mage_E Loaded - Cooldown: %f"), ESkillData.Cooldown);
		}
	}

	if (FAFSkillInfo* HeavyRow = SkillDataTable->FindRow<FAFSkillInfo>(TEXT("Aurora_Right"), ContextString))
	{
		HeavyAttackData = *HeavyRow;
	}

	// 2. 서버일 때만 PlayerState 스탯 설정
	if (HasAuthority())
	{
		AAFPlayerState* AFPS = GetPlayerState<AAFPlayerState>();
		if (AFPS && StatDataTable)
		{
			FAFPlayerCharacterStatRow* StatRow = StatDataTable->FindRow<FAFPlayerCharacterStatRow>(TEXT("Aurora"), ContextString);
			if (StatRow)
			{
				AFPS->SetHealth(StatRow->Maxhp, StatRow->Maxhp);
				AFPS->SetMana(StatRow->Maxmana, StatRow->Maxmana);
				UE_LOG(LogTemp, Warning, TEXT("MaxHP Mana Binding!!") );
			}
		}
	}
}

void AAFAurora::ServerRPC_SkillE_Implementation()
{
	if (bIsAttacking || bIsUsingSkill) return;

	if (SkillComponent == nullptr)
	{
		SkillComponent = FindComponentByClass<UAFSkillComponent>();
	}

	if (SkillComponent && SkillComponent->CanUseSkill(TEXT("Aurora_E")))
	{
		AAFPlayerState* PS = GetPlayerState<AAFPlayerState>();
		if (PS && PS->ConsumeMana(ESkillData.ManaCost))
		{
			bIsUsingSkill = true;
			SkillComponent->StartCooldown(TEXT("Aurora_E"), ESkillData.Cooldown);

			Multicast_PlaySkillEMontage();
		}
	}
}

void AAFAurora::ServerRPC_SkillQ_Implementation()
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
		bool bCanUse = SkillComponent->CanUseSkill(TEXT("Aurora_Q"));
		UE_LOG(LogTemp, Log, TEXT("@@@ [Server] CanUseSkill(Aurora_Q): %s"), bCanUse ? TEXT("True") : TEXT("False"));

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
					SkillComponent->StartCooldown(TEXT("Aurora_Q"), QSkillData.Cooldown);
					Multicast_PlaySkillQMontage();
					UE_LOG(LogTemp, Log, TEXT("@@@ [Server] Skill Q Success!"));
				}
				else { UE_LOG(LogTemp, Error, TEXT("@@@ [Server] Mana Shortage!")); }
			}
			else { UE_LOG(LogTemp, Error, TEXT("@@@ [Server] PlayerState is NULL!")); }
		}
	}
	else
	{
		UE_LOG(LogTemp, Fatal, TEXT("@@@ [Server] CRITICAL: SkillComponent not found on Aurora!"));
	}
}

void AAFAurora::HandleOnCheckHit()
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
		HandleSkillHitCheck(ESkillData.SkillRange, ESkillData.Damage, 0.f);
	}

	// 3) Q 스킬 판정 + 이펙트
	else if (Anim->Montage_IsPlaying(SkillQMontage))
	{
		{
			HandleSkillHitCheck(QSkillData.SkillRange, QSkillData.Damage, 0.f);
		}
	}

	// 4) 강공격
	else if (HeavyAttackMontage && Anim->Montage_IsPlaying(HeavyAttackMontage))
	{
		// 핵심: 이미 이번 휘두르기에서 판정을 했다면 통과
		if (bHeavyHitChecked) return;

		if (SkillComponent)
		{
			if (SkillComponent->CanUseSkill(TEXT("Aurora_Right")))
			{
				// 1. 쿨타임 시작 (딱 한 번만 실행됨)
				SkillComponent->StartCooldown(TEXT("Aurora_Right"), HeavyAttackData.Cooldown);

				// 2. 데미지 판정
				HandleSkillHitCheck(HeavyAttackData.SkillRange, HeavyAttackData.Damage, 0.f);

				// 3. 플래그 세팅
				bHeavyHitChecked = true;
				bIsUsingSkill = true;

			}
		}
		}
}

void AAFAurora::HandleSkillHitCheck(float Radius, float Damage, float RotationOffset)
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

void AAFAurora::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	LoadAuroraData();

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
