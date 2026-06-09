// AFMage.cpp

#include "Character/AFMage.h"
#include "GameFramework/Controller.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/AFAttributeComponent.h"
#include "Components/AFSkillComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Player/AFPlayerState.h"
#include "Engine/OverlapResult.h"
#include "DrawDebugHelpers.h"
#include "Player/AFPlayerController.h"

AAFMage::AAFMage()
{
	PrimaryActorTick.bCanEverTick = true;

	AimYaw   = 0.f;
	AimPitch = 0.f;
	AimAlpha = 1.f;

	// 기본 이동 속도
	GetCharacterMovement()->MaxWalkSpeed = 500.f;

    // 생성자에서 부모의 마나 소모량 변수 등을 수정
    SkillEManaCost = 40.f;
    SkillQManaCost = 100.f;

	SkillComponent = CreateDefaultSubobject<UAFSkillComponent>(TEXT("SkillComponent"));
	
	UAnimInstance* Anim = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (Anim && HeavyAttackMontage && !Anim->Montage_IsPlaying(HeavyAttackMontage))
	{
		bHeavyFxPlayed = false;
	}
}

void AAFMage::BeginPlay()
{
	Super::BeginPlay();
	LoadMageData();
}

void AAFMage::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!Controller) return;

	const FRotator ControlRot = Controller->GetControlRotation();
	const FRotator ActorRot   = GetActorRotation();
	const FRotator DeltaRot  = (ControlRot - ActorRot).GetNormalized();

	AimYaw   = DeltaRot.Yaw;
	AimPitch = FMath::Clamp(DeltaRot.Pitch, -90.f, 90.f);
	AimAlpha = 1.f;
	
	UAnimInstance* Anim = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (Anim && HeavyAttackMontage)
	{
		const bool bNowHeavyPlaying = Anim->Montage_IsPlaying(HeavyAttackMontage);

		// (중요) 강공격이 새로 시작되는 프레임에 1회 가드 리셋
		if (bNowHeavyPlaying && !bWasHeavyMontagePlaying)
		{
			bHeavyFxPlayed = false;
		}

		// (중요) 강공격이 끝나는 프레임에 상태 플래그/이동 복구
		if (!bNowHeavyPlaying && bWasHeavyMontagePlaying)
		{
			bHeavyFxPlayed = false;

			// 부모에서 강공격 상태를 잠갔을 가능성까지 같이 풀어줌
			bIsHeavyAttacking = false;
			bIsAttacking = false;
			bIsUsingSkill = false;

			if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
			{
				MoveComp->SetMovementMode(MOVE_Walking);
			}
		}

		bWasHeavyMontagePlaying = bNowHeavyPlaying;
	}
}

#pragma region Movement
void AAFMage::StartSprint(const FInputActionValue& Value)
{
	Super::StartSprint(Value);
	bIsSprinting = true;
}
void AAFMage::StopSprint(const FInputActionValue& Value)
{
	Super::StopSprint(Value);
	bIsSprinting = false;
}
void AAFMage::Jump()
{
	// 마법사는 점프 불가
}
void AAFMage::StopJumping()
{
	// 마법사는 점프 불가
}
#pragma endregion

// E 스킬: 보호막 (공격 판정이 아니라 아군 서포트)
void AAFMage::ServerRPC_SkillE_Implementation()
{
	if (bIsAttacking || bIsUsingSkill) return;

	if (SkillComponent == nullptr)
	{
		SkillComponent = FindComponentByClass<UAFSkillComponent>();
	}

	// SkillComponent가 유효한지 반드시 확인 (방어적 프로그래밍)
	if (SkillComponent && SkillComponent->CanUseSkill(TEXT("Mage_E")))
	{
		AAFPlayerState* PS = GetPlayerState<AAFPlayerState>();
		if (PS && PS->ConsumeMana(ESkillData.ManaCost))
		{
			bIsUsingSkill = true;
			SkillComponent->StartCooldown(TEXT("Mage_E"), ESkillData.Cooldown);

			ApplyShieldToAllies(ESkillData.SkillRange, E_ShieldAmount);
			Multicast_PlaySkillEMontage();
		}
	}
}

// Q 스킬: 아마겟돈 (강력한 광역 공격)
void AAFMage::ServerRPC_SkillQ_Implementation()
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
		bool bCanUse = SkillComponent->CanUseSkill(TEXT("Mage_Q"));
		UE_LOG(LogTemp, Log, TEXT("@@@ [Server] CanUseSkill(Mage_Q): %s"), bCanUse ? TEXT("True") : TEXT("False"));

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
					SkillComponent->StartCooldown(TEXT("Mage_Q"), QSkillData.Cooldown);
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
		UE_LOG(LogTemp, Fatal, TEXT("@@@ [Server] CRITICAL: SkillComponent not found on Mage!"));
	}
}

// [핵심] 부모의 함수를 완전히 새로 씀 (슬로우 로직 삭제됨)
void AAFMage::HandleSkillHitCheck(float Radius, float Damage, float RotationOffset)
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

// 보호막 전용 로직
void AAFMage::ApplyShieldToAllies(float Radius, float Amount)
{
    if (!HasAuthority()) return;

    TArray<FOverlapResult> OverlapResults;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(Radius);

    if (GetWorld()->OverlapMultiByChannel(OverlapResults, GetActorLocation(), FQuat::Identity, ECC_Pawn, Sphere))
    {
        for (auto& Result : OverlapResults)
        {
            AActor* HitActor = Result.GetActor();
            if (HitActor && (HitActor == this || IsAlly(HitActor)))
            {
                if (UAFAttributeComponent* Attr = HitActor->FindComponentByClass<UAFAttributeComponent>())
                {
                    // 1. 서버에서 데이터 변경 (복제됨)
                    Attr->AddShield(Amount, E_Duration);

                    // 2. 모든 클라이언트에게 이펙트 재생 명령
                    Multicast_PlayShieldEffect(HitActor);
                }
            }
        }
    }
}

void AAFMage::Multicast_SpawnHeavyEffectBP_Implementation(const FVector& SpawnLocation, const FRotator& SpawnRotation)
{
	UWorld* World = GetWorld();
	if (!World) return;

	UE_LOG(LogTemp, Warning, TEXT("[Mage Heavy FX] Multicast called. BP=%s, NS=%s"),
		HeavyAttackEffectBP ? TEXT("Valid") : TEXT("Null"),
		HeavyAttackEffectNS ? TEXT("Valid") : TEXT("Null"));

	if (HeavyAttackEffectBP)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		World->SpawnActor<AActor>(HeavyAttackEffectBP, SpawnLocation, SpawnRotation, Params);
		return;
	}

	if (HeavyAttackEffectNS)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, HeavyAttackEffectNS, SpawnLocation, SpawnRotation);
		return;
	}
}

void AAFMage::Multicast_SpawnQEffectBP_Implementation(const FVector& SpawnLocation, const FRotator& SpawnRotation)
{
    if (!SkillQEffectBP) return;

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    GetWorld()->SpawnActor<AActor>(SkillQEffectBP, SpawnLocation, SpawnRotation, Params);
}

void AAFMage::Multicast_SpawnQEffectFollow_Implementation(AActor* TargetActor)
{
	if (!TargetActor || !SkillQEffectBP) return;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const FVector SpawnLoc = TargetActor->GetActorLocation() + Q_FollowOffset;

	AActor* FX = GetWorld()->SpawnActor<AActor>(SkillQEffectBP, SpawnLoc, FRotator::ZeroRotator, Params);
	if (!FX) return;

	FX->AttachToComponent(
		TargetActor->GetRootComponent(),
		FAttachmentTransformRules::KeepWorldTransform
	);
}

void AAFMage::SpawnHeavyFx_Server()
{
	if (SkillComponent == nullptr)
	{
		SkillComponent = FindComponentByClass<UAFSkillComponent>();
	}

	if (SkillComponent)
	{
		bool bCanUse = SkillComponent->CanUseSkill(TEXT("Mage_Right"));

		if (bCanUse)
		{
			bIsUsingSkill = true;
			SkillComponent->StartCooldown(TEXT("Mage_Right"), HeavyAttackData.Cooldown);
		}

		if (!HasAuthority()) return;
		if (bHeavyFxPlayed) return; // 중복 방지(타이머/노티파이 둘 다 대비)
		bHeavyFxPlayed = true;

		const float Radius = (HeavyAttackData.SkillRange > 0.f) ? HeavyAttackData.SkillRange : 250.f;
		const float Damage = (HeavyAttackData.Damage > 0.f) ? HeavyAttackData.Damage : 40.f;

		// 1) 데미지 판정(서버)
		HandleSkillHitCheck(Radius, Damage, 0.f);

		// 2) 이펙트 위치 계산(바닥 트레이스) + 멀티캐스트
		const FVector FxBase = GetActorLocation()
			+ GetActorForwardVector() * Heavy_FxForward
			+ FVector(0.f, 0.f, Heavy_FxUp);

		FHitResult Hit;
		const FVector TraceStart = FxBase + FVector(0, 0, Heavy_TraceStartUp);
		const FVector TraceEnd = FxBase - FVector(0, 0, 5000.f);

		FVector FxLoc = FxBase;
		if (GetWorld() && GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility))
		{
			FxLoc = Hit.ImpactPoint;
		}

		Multicast_SpawnHeavyEffectBP(FxLoc, FRotator::ZeroRotator);
	}
}

// 통합 공격 로직
void AAFMage::HandleOnCheckHit()
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

					if (AAFPlayerCharacter* Victim = Cast<AAFPlayerCharacter>(HitActor))
					{
						if (AAFPlayerState* VictimPS = Victim->GetPlayerState<AAFPlayerState>())
						{
							VictimPS->ConsumeMana(ManaBurnAmount);
							UE_LOG(LogTemp, Warning, TEXT("Mage Mana Burn! %f reduced"), ManaBurnAmount);
						}
						Victim->TriggerHitReact_FromAttacker(this);
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
		float CurrentPos = Anim->Montage_GetPosition(SkillQMontage);

		if (!bQFxPlayed && CurrentPos > 1.4f)
		{
			bQFxPlayed = true;

			FVector FxBase = GetActorLocation() + GetActorForwardVector() * 150.f;

			FHitResult Hit;
			FVector TraceStart = FxBase + FVector(0,0,1000.f);
			FVector TraceEnd   = FxBase - FVector(0,0,5000.f);

			FVector FxLoc = FxBase;
			if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility))
			{
				FxLoc = Hit.ImpactPoint;
			}
			Multicast_SpawnQEffectBP(FxLoc, FRotator::ZeroRotator);
		}

		{
			HandleSkillHitCheck(QSkillData.SkillRange * 0.5f, QSkillData.Damage * 0.2f, 0.f);
		}

		const float QLength = SkillQMontage ? SkillQMontage->GetPlayLength() : 0.f;
		if (QLength > 0.f && CurrentPos >= QLength - 0.1f)
		{
			bQFxPlayed = false;
			bIsUsingSkill = false;
		}
	}
	
	else if (HeavyAttackMontage && Anim->Montage_IsPlaying(HeavyAttackMontage))
	{
		if (SkillComponent == nullptr)
		{
			SkillComponent = FindComponentByClass<UAFSkillComponent>();
		}

		if (SkillComponent)
		{
			bool bCanUse = SkillComponent->CanUseSkill(TEXT("Mage_Right"));

			if (bCanUse)
			{
				bIsUsingSkill = true;
				SkillComponent->StartCooldown(TEXT("Mage_Right"), HeavyAttackData.Cooldown);
				HandleSkillHitCheck(HeavyAttackData.SkillRange, HeavyAttackData.Damage, 0.f);
			}
		}


		const float CurrentPos = Anim->Montage_GetPosition(HeavyAttackMontage);

		const float Radius = (HeavyAttackData.SkillRange > 0.f) ? HeavyAttackData.SkillRange : 250.f;
		const float Damage = (HeavyAttackData.Damage > 0.f) ? HeavyAttackData.Damage : 40.f;

	
			
	

		if (!bHeavyFxPlayed && CurrentPos >= Heavy_FxDelay)
		{
			bHeavyFxPlayed = true;

			FVector FxBase =
				GetActorLocation()
				+ GetActorForwardVector() * Heavy_FxForward
				+ FVector(0.f, 0.f, Heavy_FxUp);

			FHitResult Hit;
			FVector TraceStart = FxBase + FVector(0,0,Heavy_TraceStartUp);
			FVector TraceEnd   = FxBase - FVector(0,0,5000.f);

			FVector FxLoc = FxBase;
			if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility))
			{
				FxLoc = Hit.ImpactPoint;
			}

			Multicast_SpawnHeavyEffectBP(FxLoc, FRotator::ZeroRotator);
		}
	}
}

void AAFMage::Multicast_PlaySkillEMontage_Implementation()
{
    // 서버와 모든 클라이언트에서 동시에 실행됨
    if (SkillEMontage)
    {
        PlayAnimMontage(SkillEMontage);
    }
}

void AAFMage::Multicast_PlayShieldEffect_Implementation(AActor* TargetActor)
{
    // 타겟이 유효하고 팀원이 만든 나이아가라 에셋이 할당되어 있다면 재생
    if (TargetActor && ShieldEffect)
    {
        // 핵심: SpawnSystemAtLocation이 아니라 SpawnSystemAttached를 사용합니다.
        UNiagaraFunctionLibrary::SpawnSystemAttached(
            ShieldEffect,               // 재생할 나이아가라 시스템
            TargetActor->GetRootComponent(), // 부착할 부모 컴포넌트 (Root)
            NAME_None,                  // 부착할 소켓 이름 (필요 없으면 None)
            FVector::ZeroVector,        // 상대적 위치 (0,0,0이면 타겟 정중앙)
            FRotator::ZeroRotator,      // 상대적 회전
            EAttachLocation::SnapToTarget, // 핵심: 타겟 위치에 딱 붙임
            true                        // 시스템 자동 파괴 (이펙트 종료 시)
        );

        UE_LOG(LogTemp, Log, TEXT("Shield Effect Played on: %s"), *TargetActor->GetName());
    }
}

void AAFMage::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	LoadMageData();

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

void AAFMage::LoadMageData()
{
	// 서버/클라이언트 모두에서 실행되어야 함
	static const FString ContextString(TEXT("MageDataContext"));
	UE_LOG(LogTemp, Log, TEXT("@@@ LoadMageData Started (NetMode: %d)"), GetNetMode());

	if (SkillDataTable)
	{
		if (FAFSkillInfo* QRow = SkillDataTable->FindRow<FAFSkillInfo>(TEXT("Mage_Q"), ContextString))
		{
			QSkillData = *QRow;
		}
		else { UE_LOG(LogTemp, Error, TEXT("@@@ [Data] Failed to find Mage_Q in DataTable!")); }
	}
	else { UE_LOG(LogTemp, Error, TEXT("@@@ [Data] SkillDataTable is NOT Assigned!")); }

	if (SkillDataTable)
	{
		if (FAFSkillInfo* ERow = SkillDataTable->FindRow<FAFSkillInfo>(TEXT("Mage_E"), ContextString))
		{
			ESkillData = *ERow;
			UE_LOG(LogTemp, Log, TEXT("@@@ [Data] Mage_E Loaded - Cooldown: %f"), ESkillData.Cooldown);
		}
	}

	if (FAFSkillInfo* HeavyRow = SkillDataTable->FindRow<FAFSkillInfo>(TEXT("Mage_Right"), ContextString))
	{
		HeavyAttackData = *HeavyRow; 
	}

	// 2. 서버일 때만 PlayerState 스탯 설정
	if (HasAuthority())
	{
		AAFPlayerState* AFPS = GetPlayerState<AAFPlayerState>();
		if (AFPS && StatDataTable)
		{
			FAFPlayerCharacterStatRow* StatRow = StatDataTable->FindRow<FAFPlayerCharacterStatRow>(TEXT("Mage"), ContextString);
			if (StatRow)
			{
				AFPS->SetHealth(StatRow->Maxhp, StatRow->Maxhp);
				AFPS->SetMana(StatRow->Maxmana, StatRow->Maxmana);
				UE_LOG(LogTemp, Warning, TEXT("MaxHP Mana Binding!!"));
			}
		}
	}
}

