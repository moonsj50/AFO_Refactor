#pragma once

#include "CoreMinimal.h"
#include "AFPlayerCharacter.h"
#include "AFDarkKnight.generated.h"

class UAnimMontage;

UCLASS()
class AFO_API AAFDarkKnight : public AAFPlayerCharacter
{
	GENERATED_BODY()

public:
	AAFDarkKnight();
	virtual void BeginPlay() override;

	// 패시브: 출혈 (1초마다 5씩)
	void ApplyPassiveBleed(AActor* Target);

	// E 스킬: 광역 회전 공격
	UFUNCTION(Server, Reliable)
	void UseSkillE();

protected:
	// ===== Q 연출 (에디터에서 세팅) =====
	UPROPERTY(EditAnywhere, Category="DarkKnight|Skill Q")
	TObjectPtr<UAnimMontage> SkillQChargeMontage = nullptr;

	// 여기!! 나이아가라가 아니라 "BP 액터 클래스"를 넣을거임
	UPROPERTY(EditAnywhere, Category="DarkKnight|Skill Q|VFX")
	TSubclassOf<AActor> QChargeFXBP;

	UPROPERTY(EditAnywhere, Category="DarkKnight|Skill Q|VFX")
	FName QChargeAttachSocket = NAME_None;

	UPROPERTY(EditAnywhere, Category="DarkKnight|Skill Q|VFX")
	FVector QChargeFXLocationOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category="DarkKnight|Skill Q|VFX")
	FRotator QChargeFXRotationOffset = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, Category="DarkKnight|Skill Q|Timing")
	float QChargeFxDelay = 2.0f;

	UPROPERTY(EditAnywhere, Category="DarkKnight|Skill Q|Buff")
	float QBuffDuration = 20.f;

	UPROPERTY(EditAnywhere, Category="DarkKnight|Skill Q|Buff")
	float QBuffMultiplier = 1.05f;

	

private:
	FTimerHandle QBuffTimer;
	FTimerHandle QChargeDelayTimer;

	bool bIsQBuffActive = false;
	bool bIsQCharging = false;

	UPROPERTY(Transient)
	TObjectPtr<AActor> QChargeFXActor = nullptr;

	void ApplyQBuff_Server();
	void EndQBuff();


	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayQChargeMontage();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StartQChargeFX();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StopQChargeFX();


protected:

	float AttackDamage = 30.f;

	/** --- 데이터 로드 로직 --- */
	void LoadDarkKnightData();

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
