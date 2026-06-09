// AFMage.h

#pragma once

#include "CoreMinimal.h"
#include "Character/AFPlayerCharacter.h"
#include "Types/AFGameTypes.h"
#include "AFMage.generated.h"

class UNiagaraSystem;

UCLASS()
class AFO_API AAFMage : public AAFPlayerCharacter
{
	GENERATED_BODY()

public:
	AAFMage();
	


protected:

	/** --- 데이터 로드 로직 --- */
	void LoadMageData();

	// 스킬 컴포넌트를 저장할 멤버 변수 선언
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UAFSkillComponent> SkillComponent;

	// Mage 전용 스킬 수치 (에디터에서 수정 가능)
	UPROPERTY(EditAnywhere, Category = "Mage|Attack")  // 기본공격 데미지
	float AttackDamage = 20.f;

	UPROPERTY(EditAnywhere, Category = "Mage|Skill E") // 보호막 수치
	float E_ShieldAmount = 60.f;

	UPROPERTY(EditAnywhere, Category = "Mage|Skill E")  // E스킬 범위
	float E_Range = 500.f;

	UPROPERTY(EditAnywhere, Category = "Mage|Balance") // 보호막 지속 시간
	float E_Duration = 5.f;

	UPROPERTY(EditAnywhere, Category = "Mage|Skill Q")  // Q스킬 데미지
	float Q_Damage = 120.f;

	UPROPERTY(EditAnywhere, Category = "Mage|Skill Q")  // Q스킬 범위
	float Q_Radius = 400.f;

	// 일반 공격 마나 감소량
	UPROPERTY(EditAnywhere, Category = "Mage|Passive")
	float ManaBurnAmount = 10.f;

	// 스킬별 쿨타임 수치
	UPROPERTY(EditAnywhere, Category = "Mage|Cooldown")
	float Q_CooldownTime = 10.f;

	UPROPERTY(EditAnywhere, Category = "Mage|Cooldown")
	float E_CooldownTime = 15.f;
	
	// 강공격 사거리
	UPROPERTY(EditAnywhere, Category="Mage|Heavy Attack|FX")
	float Heavy_FxForward = 350.f;   // 정면 거리

	UPROPERTY(EditAnywhere, Category="Mage|Heavy Attack|FX")
	float Heavy_FxUp = 200.f;        // 기준점 높이

	UPROPERTY(EditAnywhere, Category="Mage|Heavy Attack|FX")
	float Heavy_TraceStartUp = 1500.f; // 위에서 떨어지는 느낌(트레이스 시작 높이)
	
	UPROPERTY(EditAnywhere, Category="Mage|Heavy Attack|FX")
	float Heavy_FxDelay = 0.35f; // 강공격 이펙트가 터지는 타이밍(몽타주 임팩트 프레임에 맞추기)
	
	bool bWasHeavyMontagePlaying = false;

	FTimerHandle TimerHandle_HeavyFx;
	FTimerHandle TimerHandle_HeavyReset;

	UFUNCTION()
	void SpawnHeavyFx_Server(); // 서버에서 위치 계산 후 멀티캐스트 호출
	
	// Q스킬 타겟 추적
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SpawnQEffectFollow(AActor* TargetActor);

	UPROPERTY(EditAnywhere, Category="Mage|Skill Q|Follow")
	float Q_TargetSearchRadius = 1200.f;

	UPROPERTY(EditAnywhere, Category="Mage|Skill Q|Follow")
	float Q_TargetMaxAngleDeg = 45.f;

	UPROPERTY(EditAnywhere, Category="Mage|Skill Q|Follow")
	FVector Q_FollowOffset = FVector(0.f, 0.f, 0.f);
	// 쿨타임 관리용 핸들
	FTimerHandle TimerHandle_SkillQ;
	FTimerHandle TimerHandle_SkillE;

	bool bCanUseSkillQ = true;
	bool bCanUseSkillE = true;

	// 쿨타임 초기화 함수
	void ResetSkillQ() { bCanUseSkillQ = true; }
	void ResetSkillE() { bCanUseSkillE = true; }

	// 부모의 판정 함수 오버라이드
	virtual void HandleOnCheckHit() override;

	virtual void Multicast_PlaySkillEMontage_Implementation() override;


	
	virtual void Jump() override; // 점프 차단
	virtual void StopJumping() override;
	virtual void StartSprint(const FInputActionValue& Value) override;
	virtual void StopSprint(const FInputActionValue& Value) override;
	virtual void Tick(float DeltaTime) override;


	// 내부적으로 사용할 스킬 데이터 캐싱 (매번 FindRow 방지)
	FAFSkillInfo QSkillData;
	FAFSkillInfo ESkillData;
	FAFSkillInfo HeavyAttackData;

public: 
	// AimOffset 값
	UPROPERTY(BlueprintReadOnly, Category="Aim")
	float AimYaw;
	UPROPERTY(BlueprintReadOnly, Category="Aim")
	float AimPitch;
	UPROPERTY(BlueprintReadOnly, Category="Aim")
	float AimAlpha;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bIsSprinting = false;

protected:
	virtual void BeginPlay() override;

	virtual void OnRep_PlayerState() override;

	// Q 이펙트 1회 재생 가드
	bool bQFxPlayed = false;
	
	bool bHeavyFxPlayed = false;

	// 부모의 RPC를 오버라이드 (Implementation만 작성하면 됨)
	virtual void ServerRPC_SkillE_Implementation() override;
	virtual void ServerRPC_SkillQ_Implementation() override;

	// Mage 전용 패시브를 위해 판정 함수 오버라이드
	virtual void HandleSkillHitCheck(float Radius, float Damage, float RotationOffset) override;

	// 보호막 부여 전용 함수 (내부 로직 분리)
	void ApplyShieldToAllies(float Radius, float Amount);

	// 보호막 이펙트 멀티캐스트
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayShieldEffect(AActor* TargetActor);
	
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_SpawnQEffectBP(const FVector& SpawnLocation, const FRotator& SpawnRotation);

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_SpawnHeavyEffectBP(const FVector& SpawnLocation, const FRotator& SpawnRotation);

	// 에디터에서 설정할 이펙트 에셋
	UPROPERTY(EditAnywhere, Category = "Mage|Effects")
	TObjectPtr<UNiagaraSystem> ShieldEffect;

	UPROPERTY(EditAnywhere, Category = "Mage|Effects")
	TSubclassOf<AActor> SkillQEffectBP;

	UPROPERTY(EditAnywhere, Category = "Mage|Effects")
	TSubclassOf<AActor> HeavyAttackEffectBP;

	UPROPERTY(EditAnywhere, Category = "Mage|Effects")
	TObjectPtr<UNiagaraSystem> HeavyAttackEffectNS;
};
