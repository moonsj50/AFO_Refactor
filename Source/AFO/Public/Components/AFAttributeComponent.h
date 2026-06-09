// AFAttributeComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AFAttributeComponent.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;
DECLARE_MULTICAST_DELEGATE_OneParam(FOnShieldChangedDelegate, float /*NewShield*/);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AFO_API UAFAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAFAttributeComponent();

protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Attribute")
	float MaxHealth = 300.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Attribute")
	float Health = 300.f;

	//사망 상태 플래그
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Attribute")
	bool bIsDead = false;



public:
	void ApplyDamage(float Damage, AController* InstigatedBy);

	// 체력 변경 함수 선언
	void ApplyHealthChange(int32 Value);
	void ResetMaxHealth(int32 Value);
	void ResetMaxHealth();

protected:
	// 사망 처리 분리
	void HandleDeath(AController* InstigatedBy);

public:
	// Health 변경 시 PlayerState에 동기화 하는 함수
	void SyncHealthToPlayerState();

public:
	void ModifyMaxHealth(float Ratio);

	


private:
	FTimerHandle HealthSyncTimerHandle;

	// Floating Damage
protected:
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_NotifyDamage(float Damage, FVector Location, AController* InstigatedBy, bool bIsCritical);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;



public:

	// 보호막 추가 함수 (서버에서 호출)
	void AddShield(float Amount, float Duration);

	UPROPERTY(ReplicatedUsing = OnRep_CurrentShield, BlueprintReadOnly, Category = "Status")
	float CurrentShield;

	UFUNCTION()
	void OnRep_CurrentShield(); // 보호막 이펙트 On/Off 제어용

	// 현재 보호막 수치 가져오기
	float GetCurrentShield() const { return CurrentShield; }

	// UI나 이펙트 갱신을 위한 델리게이트
	FOnShieldChangedDelegate OnShieldChanged;

private:
	FTimerHandle ShieldTimerHandle;
	void OnShieldExpired(); // 지속시간 종료 시 호출



protected:
	// 공격력 배수 (기본값 1.0)
	UPROPERTY(VisibleAnywhere, Category = "Attribute")
	float AttackMultiplier = 1.0;

	FTimerHandle AttackBuffTimerHandle;

public:
	// 외부(아이템)에서 호출할 함수
	void ApplyAttackBuff(float Multiplier, float Duration);

	// 현재 공격력 배수 반환 (데미지 계산 시 사용)
	float GetAttackMultiplier() const { return AttackMultiplier; }

public:
	// 현재 활성화된 오오라 컴포넌트를 저장 (나중에 지우기 위해)
	UPROPERTY()
	UNiagaraComponent* ActiveAuraComponent;

	// 오오라 생성/제거 함수 (서버에서 실행되어 멀티캐스트로 전파)
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ApplyAura(UNiagaraSystem* AuraSystem, FLinearColor Color, float Duration);
};