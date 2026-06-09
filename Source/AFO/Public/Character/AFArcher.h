#pragma once

#include "CoreMinimal.h"
#include "Character/AFPlayerCharacter.h"
#include "AFArcher.generated.h"

UCLASS()
class AFO_API AAFArcher : public AAFPlayerCharacter
{
	GENERATED_BODY()

public:
	AAFArcher();

	virtual void Tick(float DeltaTime) override;

	// AimOffset 값
	UPROPERTY(BlueprintReadOnly, Category="Aim")
	float AimYaw;

	UPROPERTY(BlueprintReadOnly, Category="Aim")
	float AimPitch;

	UPROPERTY(BlueprintReadOnly, Category="Aim")
	float AimAlpha;


protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditDefaultsOnly, Category="Balance|Archer")
	float Archer_BasicDamage = 6.f;

	UPROPERTY(EditDefaultsOnly, Category="Balance|Archer")
	float Archer_BasicForwardDistance = 260.f;

	UPROPERTY(EditDefaultsOnly, Category="Balance|Archer")
	float Archer_BasicRadius = 130.f;

	UPROPERTY(EditDefaultsOnly, Category="Balance|Archer")
	float Archer_HeavyDamage = 10.f;

	UPROPERTY(EditDefaultsOnly, Category="Balance|Archer")
	float Archer_HeavyForwardDistance = 320.f;

	UPROPERTY(EditDefaultsOnly, Category="Balance|Archer")
	float Archer_HeavyRadius = 160.f;

	UPROPERTY(EditDefaultsOnly, Category="Balance|Archer")
	float Archer_SkillEDamage = 14.f;

	UPROPERTY(EditDefaultsOnly, Category="Balance|Archer")
	float Archer_SkillEForwardDistance = 520.f;

	UPROPERTY(EditDefaultsOnly, Category="Balance|Archer")
	float Archer_SkillERadius = 200.f;

	UPROPERTY(EditDefaultsOnly, Category="Balance|Archer")
	float Archer_SkillQDamage = 22.f;

	UPROPERTY(EditDefaultsOnly, Category="Balance|Archer")
	float Archer_SkillQForwardDistance = 680.f;

	UPROPERTY(EditDefaultsOnly, Category="Balance|Archer")
	float Archer_SkillQRadius = 260.f;

	// 핵심: 데미지/사거리 Archer 전용으로 분기
	virtual void HandleOnCheckHit() override;
	
	// ===== Cooldown (Editor) =====
	UPROPERTY(EditDefaultsOnly, Category="Cooldown|Archer")
	float Archer_SkillECooldown = 8.f;
	UPROPERTY(EditDefaultsOnly, Category="Cooldown|Archer")
	float Archer_SkillQCooldown = 15.f;
	
	float NextSkillETime_Archer = 0.f;
	float NextSkillQTime_Archer = 0.f;

	// ===== Cooldown Runtime =====
	bool bCanUseSkillE_Archer = true;
	bool bCanUseSkillQ_Archer = true;

	FTimerHandle TH_ArcherSkillE;
	FTimerHandle TH_ArcherSkillQ;

	UFUNCTION()
	void ResetArcherSkillE() { bCanUseSkillE_Archer = true; }
	UFUNCTION()
	void ResetArcherSkillQ() { bCanUseSkillQ_Archer = true; }

	// 아처가 직접 쿨타임을 막도록 RPC 오버라이드
	virtual void ServerRPC_SkillE_Implementation() override;
	virtual void ServerRPC_SkillQ_Implementation() override;

private:
	void ArcherDealDamage(float ForwardDistance, float Radius, float Damage);
	void ArcherSkillHitCheck(float ForwardDistance, float Radius, float Damage, float RotationOffset, bool bApplySlow);
};