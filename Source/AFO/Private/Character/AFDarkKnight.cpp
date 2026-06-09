#include "Character/AFDarkKnight.h"
#include "Components/AFAttributeComponent.h"
#include "TimerManager.h"
#include "Player/AFPlayerState.h"
#include "Components/AFSkillComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/OverlapResult.h"
#include "Player/AFPlayerController.h"

AAFDarkKnight::AAFDarkKnight()
{
	SkillComponent = CreateDefaultSubobject<UAFSkillComponent>(TEXT("SkillComponent"));
}

void AAFDarkKnight::BeginPlay()
{
	Super::BeginPlay();
	LoadDarkKnightData();
	
}

void AAFDarkKnight::ApplyPassiveBleed(AActor* Target)
{
	if (!HasAuthority()) return;
	if (!Target) return;

	if (UAFAttributeComponent* Attr = Target->FindComponentByClass<UAFAttributeComponent>())
	{
		Attr->ApplyHealthChange(-5);
	}
}

void AAFDarkKnight::UseSkillE_Implementation()
{
	HandleSkillHitCheck(200.f, 30.f, 0.f);
}

void AAFDarkKnight::ServerRPC_SkillQ_Implementation()
{
	if (!HasAuthority()) return;

	// 다른 캐릭터들처럼 막고 싶으면 유지
	if (bIsAttacking || bIsUsingSkill) return;
	if (bIsQBuffActive || bIsQCharging) return;

	// 1. 컴포넌트 존재 확인 로그
	if (SkillComponent == nullptr)
	{
		SkillComponent = FindComponentByClass<UAFSkillComponent>();
	}

	if (SkillComponent)
	{
		// 2. 쿨타임 가능 여부 확인 로그
		bool bCanUse = SkillComponent->CanUseSkill(TEXT("DarkKnight_Q"));
		UE_LOG(LogTemp, Log, TEXT("@@@ [Server] CanUseSkill(DarkKnight_Q): %s"), bCanUse ? TEXT("True") : TEXT("False"));

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
					SkillComponent->StartCooldown(TEXT("DarkKnight_Q"), QSkillData.Cooldown);
					Multicast_PlaySkillQMontage();
					UE_LOG(LogTemp, Log, TEXT("@@@ [Server] Skill Q Success!"));
				}
			}
		}
	}

	bIsUsingSkill = true;
	bIsQCharging = true;

	UE_LOG(LogTemp, Warning, TEXT("[DK Q] ServerRPC_SkillQ_Implementation CALLED"));

	Multicast_PlayQChargeMontage();

	GetWorldTimerManager().ClearTimer(QChargeDelayTimer);
	GetWorldTimerManager().SetTimer(
		QChargeDelayTimer,
		this,
		&AAFDarkKnight::ApplyQBuff_Server,
		QChargeFxDelay,
		false
	);
}

void AAFDarkKnight::ApplyQBuff_Server()
{
	if (!HasAuthority()) return;
	if (!bIsQCharging) return;      // 차징 중일 때만
	if (bIsQBuffActive) return;     // 중복 방지

	bIsQCharging = false;
	bIsQBuffActive = true;

	UE_LOG(LogTemp, Warning, TEXT("[DK Q] ApplyQBuff_Server -> START FX + BUFF"));

	Multicast_StartQChargeFX();

	if (UAFAttributeComponent* Attr = FindComponentByClass<UAFAttributeComponent>())
	{
		Attr->ModifyMaxHealth(QBuffMultiplier);
	}

	GetWorldTimerManager().ClearTimer(QBuffTimer);
	GetWorldTimerManager().SetTimer(
		QBuffTimer,
		this,
		&AAFDarkKnight::EndQBuff,
		QBuffDuration,
		false
	);
}

void AAFDarkKnight::EndQBuff()
{
	if (!HasAuthority()) return;
	if (!bIsQBuffActive) return;

	UE_LOG(LogTemp, Warning, TEXT("[DK Q] EndQBuff -> STOP FX + RESET"));

	if (UAFAttributeComponent* Attr = FindComponentByClass<UAFAttributeComponent>())
	{
		Attr->ResetMaxHealth();
	}

	bIsQBuffActive = false;
	bIsQCharging = false;
	bIsUsingSkill = false;

	Multicast_StopQChargeFX();
}

void AAFDarkKnight::Multicast_PlayQChargeMontage_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("[DK Q] Multicast_PlayQChargeMontage"));

	if (SkillQChargeMontage)
	{
		PlayAnimMontage(SkillQChargeMontage);
	}
}

void AAFDarkKnight::Multicast_StartQChargeFX_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("[DK Q] Multicast_StartQChargeFX CALLED"));

	if (!QChargeFXBP || !GetMesh())
	{
		UE_LOG(LogTemp, Warning, TEXT("[DK Q] FXBP or Mesh is NULL"));
		return;
	}

	if (QChargeFXActor)
	{
		QChargeFXActor->Destroy();
		QChargeFXActor = nullptr;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	QChargeFXActor = GetWorld()->SpawnActor<AActor>(QChargeFXBP, GetActorLocation(), GetActorRotation(), Params);
	if (!QChargeFXActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DK Q] SpawnActor FAILED"));
		return;
	}

	QChargeFXActor->AttachToComponent(
		GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		QChargeAttachSocket
	);

	QChargeFXActor->SetActorRelativeLocation(QChargeFXLocationOffset);
	QChargeFXActor->SetActorRelativeRotation(QChargeFXRotationOffset);

	UE_LOG(LogTemp, Warning, TEXT("[DK Q] FX SPAWN OK"));
}

void AAFDarkKnight::Multicast_StopQChargeFX_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("[DK Q] Multicast_StopQChargeFX"));

	if (QChargeFXActor)
	{
		QChargeFXActor->Destroy();
		QChargeFXActor = nullptr;
	}
}

void AAFDarkKnight::LoadDarkKnightData()
{
	// 서버/클라이언트 모두에서 실행되어야 함
	static const FString ContextString(TEXT("DarkKnightDataContext"));
	UE_LOG(LogTemp, Log, TEXT("@@@ LoadDarkKnightData Started (NetMode: %d)"), GetNetMode());

	if (SkillDataTable)
	{
		if (FAFSkillInfo* QRow = SkillDataTable->FindRow<FAFSkillInfo>(TEXT("DarkKnight_Q"), ContextString))
		{
			QSkillData = *QRow;
		}
		else { UE_LOG(LogTemp, Error, TEXT("@@@ [Data] Failed to find Mage_Q in DataTable!")); }
	}
	else { UE_LOG(LogTemp, Error, TEXT("@@@ [Data] SkillDataTable is NOT Assigned!")); }

	if (SkillDataTable)
	{
		if (FAFSkillInfo* ERow = SkillDataTable->FindRow<FAFSkillInfo>(TEXT("DarkKnight_E"), ContextString))
		{
			ESkillData = *ERow;
			UE_LOG(LogTemp, Log, TEXT("@@@ [Data] Mage_E Loaded - Cooldown: %f"), ESkillData.Cooldown);
		}
	}

	if (FAFSkillInfo* HeavyRow = SkillDataTable->FindRow<FAFSkillInfo>(TEXT("DarkKnight_Right"), ContextString))
	{
		HeavyAttackData = *HeavyRow;
	}

	// 2. 서버일 때만 PlayerState 스탯 설정
	if (HasAuthority())
	{
		AAFPlayerState* AFPS = GetPlayerState<AAFPlayerState>();
		if (AFPS && StatDataTable)
		{
			FAFPlayerCharacterStatRow* StatRow = StatDataTable->FindRow<FAFPlayerCharacterStatRow>(TEXT("DarkKnight"), ContextString);
			if (StatRow)
			{
				AFPS->SetHealth(StatRow->Maxhp, StatRow->Maxhp);
				AFPS->SetMana(StatRow->Maxmana, StatRow->Maxmana);
				UE_LOG(LogTemp, Warning, TEXT("MaxHP Mana Binding!!") );
			}
		}
	}
}


void AAFDarkKnight::ServerRPC_SkillE_Implementation()
{
	if (bIsAttacking || bIsUsingSkill) return;

	if (SkillComponent == nullptr)
	{
		SkillComponent = FindComponentByClass<UAFSkillComponent>();
	}

	if (SkillComponent && SkillComponent->CanUseSkill(TEXT("DarkKnight_E")))
	{
		AAFPlayerState* PS = GetPlayerState<AAFPlayerState>();
		if (PS && PS->ConsumeMana(ESkillData.ManaCost))
		{
			bIsUsingSkill = true;
			SkillComponent->StartCooldown(TEXT("DarkKnight_E"), ESkillData.Cooldown);

			Multicast_PlaySkillEMontage();
		}
	}
}


void AAFDarkKnight::HandleOnCheckHit()
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
		if (SkillComponent == nullptr)
		{
			SkillComponent = FindComponentByClass<UAFSkillComponent>();
		}

		if (SkillComponent)
		{
			bool bCanUse = SkillComponent->CanUseSkill(TEXT("DarkKnight_Right"));

			if (bCanUse)
			{
				bIsUsingSkill = true;
				SkillComponent->StartCooldown(TEXT("DarkKnight_Right"), HeavyAttackData.Cooldown);
			}
		}

		HandleSkillHitCheck(HeavyAttackData.SkillRange, HeavyAttackData.Damage, 0.f);

	}
}

void AAFDarkKnight::HandleSkillHitCheck(float Radius, float Damage, float RotationOffset)
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

void AAFDarkKnight::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	LoadDarkKnightData();

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