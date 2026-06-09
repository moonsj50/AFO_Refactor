#pragma once

#include "CoreMinimal.h"
#include "Character/AFPlayerCharacter.h"
#include "Types/AFGameTypes.h"
#include "AFAurora.generated.h"

UCLASS()
class AFO_API AAFAurora : public AAFPlayerCharacter
{
	GENERATED_BODY()

public:
	AAFAurora();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bIsSprinting = false;

	virtual void StartSprint(const FInputActionValue& Value) override;
	virtual void StopSprint(const FInputActionValue& Value) override;

	virtual void Tick(float DeltaTime) override;

	// ===== Aim Offset =====
	UPROPERTY(BlueprintReadOnly, Category="Aim")
	float AimYaw;

	UPROPERTY(BlueprintReadOnly, Category="Aim")
	float AimPitch;

	UPROPERTY(BlueprintReadOnly, Category="Aim")
	float AimAlpha;

protected:
	virtual void BeginPlay() override;



protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float AttackDamage = 20.f;
	bool bHeavyHitChecked = false;

	/** --- 데이터 로드 로직 --- */
	void LoadAuroraData();

	// 스킬 컴포넌트를 저장할 멤버 변수 선언
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UAFSkillComponent> SkillComponent;

	virtual void HandleOnCheckHit() override;
	virtual void HandleSkillHitCheck(float Radius, float Damage, float RotationOffset) override;

	virtual void ServerRPC_SkillE_Implementation() override;
	virtual void ServerRPC_SkillQ_Implementation() override;

	// 내부적으로 사용할 스킬 데이터 캐싱 (매번 FindRow 방지)
	FAFSkillInfo QSkillData;
	FAFSkillInfo ESkillData;
	FAFSkillInfo HeavyAttackData;

	virtual void OnRep_PlayerState() override;
};
