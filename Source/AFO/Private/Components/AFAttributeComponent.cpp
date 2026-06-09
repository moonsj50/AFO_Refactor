// AFAttributeComponent.cpp

#include "Components/AFAttributeComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Player/AFPlayerState.h"
#include "Game/AFGameMode.h"
#include "UI/AFFloatingDamageManager.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Character/AFPlayerCharacter.h"

UAFAttributeComponent::UAFAttributeComponent()
{
	SetIsReplicatedByDefault(true);
	CurrentShield = 0.f;
	PrimaryComponentTick.bCanEverTick = false;
}

void UAFAttributeComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UAFAttributeComponent, CurrentShield);
}


void UAFAttributeComponent::BeginPlay()
{
	Super::BeginPlay();

	Health = MaxHealth;
	bIsDead = false;

	UE_LOG(LogTemp, Warning, TEXT("Attribute Begin"));

	if (GetOwner()->HasAuthority())
	{
		SyncHealthToPlayerState();
	}
}

void UAFAttributeComponent::ApplyDamage(float Damage, AController* InstigatedBy)
{
	// 서버 전용, 이미 사망 시 등 기존 체크 유지
	if (!GetOwner() || !GetOwner()->HasAuthority() || bIsDead || Damage <= 0.f)
	{
		return;
	}

	float FinalDamage = Damage;

	// 1. 공격자(InstigatedBy)의 버프 확인
	if (InstigatedBy)
	{
		// 공격자의 PlayerState나 Character에서 AttributeComponent를 찾아 배수를 가져옴
		if (APawn* InstigatorPawn = InstigatedBy->GetPawn())
		{
			if (UAFAttributeComponent* AttackerAttr = InstigatorPawn->FindComponentByClass<UAFAttributeComponent>())
			{
				// 공격자의 배수를 최종 데미지에 곱함
				FinalDamage *= AttackerAttr->GetAttackMultiplier();
			}
		}
	}

	// 2. 보호막(Shield) 처리
	if (CurrentShield > 0.f)
	{
		float DamageToShield = FMath::Min(CurrentShield, FinalDamage);
		CurrentShield -= DamageToShield;
		FinalDamage -= DamageToShield;

		// UI 갱신용
		OnRep_CurrentShield();
	}

	// 3. 최종 체력 차감 (한 번만 수행)
	if (FinalDamage > 0.f)
	{
		Health = FMath::Clamp(Health - FinalDamage, 0.f, MaxHealth);
	}

	UE_LOG(LogTemp, Warning, TEXT("[%s] FinalDamage: %f, Remaining HP: %f"), *GetOwner()->GetName(), FinalDamage, Health);


	// InstigatedBy를 통해 '누가 때렸는지'를 판별하여 공격자/피격자 색상을 결정
	Multicast_NotifyDamage(FinalDamage, GetOwner()->GetActorLocation(), InstigatedBy, false);



	SyncHealthToPlayerState();
	// 타이머를 이용한 반복 로직은 SyncHealthToPlayerState 내부에서 처리되므로,
	// 여기서는 그냥 호출만 하면 됩니다.
	// 

	if (Health <= 0.f)
	{
		if (AAFPlayerCharacter* OwnerChar = Cast<AAFPlayerCharacter>(GetOwner()))
		{
			OwnerChar->StartDeath(InstigatedBy);
		}
		HandleDeath(InstigatedBy);
	}
}

void UAFAttributeComponent::ApplyHealthChange(int32 Value)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	const int32 OldHealth = Health;
	Health = FMath::Clamp(Health + Value, 0, MaxHealth);
}


void UAFAttributeComponent::ResetMaxHealth()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	// 원본값으로 복원
	if (MaxHealth > 0)
	{
		MaxHealth = MaxHealth;
		Health = FMath::Clamp(Health, 0, MaxHealth); 
		MaxHealth = -1; // 초기화 
	}
}

void UAFAttributeComponent::HandleDeath(AController* InstigatedBy)
{
	if (bIsDead) return;
	bIsDead = true;

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(HealthSyncTimerHandle);
	}

	SyncHealthToPlayerState();

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor) return;

	UE_LOG(LogTemp, Warning, TEXT("[%s] is Dead"), *OwnerActor->GetName());

	if (APawn* PawnOwner = Cast<APawn>(OwnerActor))
	{
		AController* VictimController = PawnOwner->GetController();
		if (!VictimController) return;

		if (AAFGameMode* GM = GetWorld()->GetAuthGameMode<AAFGameMode>())
		{
			GM->HandlePlayerDeath(VictimController, InstigatedBy);
		}
	}


}

void UAFAttributeComponent::SyncHealthToPlayerState()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (APawn* PawnOwner = Cast<APawn>(GetOwner()))
	{
		// 1. PlayerState를 찾으려 시도합니다.
		if (AAFPlayerState* PS = PawnOwner->GetPlayerState<AAFPlayerState>())
		{
			// ★★★ 동기화 성공! 타이머를 멈춥니다.
			GetWorld()->GetTimerManager().ClearTimer(HealthSyncTimerHandle);

			PS->SetHealth(Health, MaxHealth);
			UE_LOG(LogTemp, Warning, TEXT("Attribute Sync: SUCCESS! Timer Cleared."));
			return;
		}
	}

	// 2. 동기화 실패 시 (Failed Attribute Sync2 발생 시)
	UE_LOG(LogTemp, Warning, TEXT("Sync FAILED. Retrying in 0.2 seconds."));

	// PlayerState를 찾을 때까지 0.2초마다 이 함수를 반복 실행합니다.
	if (!GetWorld()->GetTimerManager().IsTimerActive(HealthSyncTimerHandle))
	{
		GetWorld()->GetTimerManager().SetTimer(
			HealthSyncTimerHandle,
			this,
			&UAFAttributeComponent::SyncHealthToPlayerState,
			0.2f, // 0.2초마다 반복
			true  // 반복 실행
		);
	}
}

void UAFAttributeComponent::ModifyMaxHealth(float Ratio)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	MaxHealth = FMath::Max(1.f, MaxHealth * Ratio);

	// 현재 체력도 비율 맞춰서 늘려주고 싶으면 아래 추가
	Health = FMath::Min(Health * Ratio, MaxHealth);

	
}


void UAFAttributeComponent::Multicast_NotifyDamage_Implementation(float Damage, FVector Location, AController* InstigatedBy, bool bIsCritical)
{
	AAFFloatingDamageManager* DamageManager = Cast<AAFFloatingDamageManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AAFFloatingDamageManager::StaticClass()));

	if (DamageManager)
	{


		APlayerController* LocalPC = GetWorld()->GetFirstPlayerController();
		if (!LocalPC) return;

		AAFPlayerState* MyPS = LocalPC->GetPlayerState<AAFPlayerState>();
		if (!MyPS) return;

		// 2. 공격자의 PlayerState 가져오기
		AAFPlayerState* InstigatorPS = InstigatedBy ? InstigatedBy->GetPlayerState<AAFPlayerState>() : nullptr;

		bool bIsEnemyDamage = false;

		if (InstigatorPS)
		{
			// [팀 비교 로직]
			if (MyPS->GetTeamID() == InstigatorPS->GetTeamID())
			{
				bIsEnemyDamage = true; // 우리 팀의 공격 성공!
			}
		}
		else
		{
			// 공격자 정보가 없는 경우(예: 환경 데미지 등)는 기본적으로 빨간색
			bIsEnemyDamage = false;
		}

		DamageManager->ShowDamage(Damage, Location, bIsEnemyDamage, bIsCritical);
	}
}


void UAFAttributeComponent::AddShield(float Amount, float Duration)
{
	if (!GetOwner()->HasAuthority()) return;

	CurrentShield += Amount;
	OnRep_CurrentShield(); // 서버에서도 즉시 알림

	// 지속시간이 지나면 보호막 제거 타이머 설정
	GetWorld()->GetTimerManager().SetTimer(ShieldTimerHandle, this, &UAFAttributeComponent::OnShieldExpired, Duration, false);
}

void UAFAttributeComponent::OnShieldExpired()
{
	CurrentShield = 0.f;
	OnRep_CurrentShield();
}

void UAFAttributeComponent::OnRep_CurrentShield()
{
	OnShieldChanged.Broadcast(CurrentShield);
}


void UAFAttributeComponent::ApplyAttackBuff(float Multiplier, float Duration)
{
	if (GetOwnerRole() < ROLE_Authority) return;

	AttackMultiplier = Multiplier;

	// 기존 타이머가 있다면 초기화 (버프 시간 갱신)
	GetWorld()->GetTimerManager().SetTimer(AttackBuffTimerHandle, [this]()
		{
			AttackMultiplier = 1.0f; // 원래대로 복구
			UE_LOG(LogTemp, Log, TEXT("Attack Buff Expired!"));
		}, Duration, false);
}

void UAFAttributeComponent::Multicast_ApplyAura_Implementation(UNiagaraSystem* AuraSystem, FLinearColor Color, float Duration)
{
	if (!AuraSystem || !GetOwner()) return;

	// [중요] 기존 오오라 제거 로직을 별도로 분리
	if (ActiveAuraComponent && ActiveAuraComponent->IsValidLowLevel())
	{
		ActiveAuraComponent->DestroyComponent();
	}
	ActiveAuraComponent = nullptr;

// 오라 붙이기
	ActiveAuraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
		AuraSystem,
		GetOwner()->GetRootComponent(),
		NAME_None,
		FVector(0.f, 0.f, -90.f),
		FRotator::ZeroRotator,
		EAttachLocation::KeepRelativeOffset,
		true
	);

	if (ActiveAuraComponent)
	{
		// 색상 적용 전후로 로그를 찍어 Alpha값 확인
		ActiveAuraComponent->SetNiagaraVariableLinearColor(TEXT("User.BaseColor"), Color);
		ActiveAuraComponent->Activate(true);

		UE_LOG(LogTemp, Warning, TEXT("[%s] Aura Attached! Color Alpha: %f"), *GetOwner()->GetName(), Color.A);

		// 타이머 로직
		FTimerHandle AuraTimer;
		TWeakObjectPtr<UNiagaraComponent> WeakComp = ActiveAuraComponent;
		GetWorld()->GetTimerManager().SetTimer(AuraTimer, [WeakComp]()
			{
				if (WeakComp.IsValid())
				{
					WeakComp->DestroyComponent();
				}
			}, Duration, false);
	}
}