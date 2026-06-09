#pragma once

#include "CoreMinimal.h"
#include "Character/AFPlayerCharacter.h"
#include "AFWereWolf.generated.h"

UCLASS()
class AFO_API AAFWereWolf : public AAFPlayerCharacter
{
	GENERATED_BODY()
	
public:
	AAFWereWolf();
	virtual void BeginPlay() override;

	// 패시브: 평타 피흡 (데미지 적용 시 호출)
	void ApplyLifeSteal();

	// Q: 이동속도 증가 
	UFUNCTION(Server, Reliable)
	void Server_ApplySpeedBoost(float Ratio, float Duration);

	// E: 출혈 데미지
	UFUNCTION(Server, Reliable)
	void Server_ApplyBleeding(AActor* Target, float DamagePerSec, float Duration);

private:
	FTimerHandle SpeedBoostTimer;
	FTimerHandle BleedingTimer;

	float OriginalWalkSpeed = 0.f;

	//내부 처리 함수
	void ResetSpeedBoost();

	void ApplyBleedingTick(AActor* Target, float DamagePerSec);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	float WereWolfMoveSpeed = 200.f; 



protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float AttackDamage = 40.f;
	/** --- 데이터 로드 로직 --- */
	void LoadWereWolfData();

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