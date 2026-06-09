// AFPlayerCharacter.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputAction.h"
#include "Types/AFGameTypes.h"
#include "Net/UnrealNetwork.h"
#include "AFPlayerCharacter.generated.h"


class UAFStatusEffectComponent;
class USpringArmComponent;
class UCameraComponent;
class UAnimMontage;
struct FInputActionValue;
class UAFAttributeComponent;


UENUM(BlueprintType)
enum class EAFHitDir : uint8
{
	Front,
	Back,
	Left,
	Right
};

UCLASS()
class AFO_API AAFPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AAFPlayerCharacter();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable)
	void ServerRPC_SkillE();
	UFUNCTION(Server, Reliable)
	void ServerRPC_SkillQ();

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
	void Move(const FInputActionValue& value);          // 이동구현 앞뒤좌우
	UFUNCTION()
	void StartJump(const FInputActionValue& value);     // 점프 시작
	UFUNCTION()
	void StopJump(const FInputActionValue& value);      // 점프 종료
	UFUNCTION()
	void Look(const FInputActionValue& value);          // 마우스 시야회전
	UFUNCTION()
	virtual void StartSprint(const FInputActionValue& Value);
	UFUNCTION()
	virtual void StopSprint(const FInputActionValue& Value);


	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite)
	bool bIsAttacking = false;
	UFUNCTION()
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	// W : Forward Speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Directional Speed")
	float ForwardSpeed = 1.0f;

	// S : Backward Speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Directional Speed")
	float BackwardSpeed = 0.65f;

	// D : Right Speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Directional Speed")
	float RightSpeed = 0.72f;

	// A : Left Speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Directional Speed")
	float LeftSpeed = 0.72f;

	// W / S 방향 (카메라 기준 Forward)
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Movement|Directional Vectors")
	FVector ForwardDir = FVector::ZeroVector;

	// A / D 방향 (카메라 기준 Right)
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Movement|Directional Vectors")
	FVector RightDir = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float NormalSpeed = 300.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SprintSpeedMultiplier = 1.5f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float SprintSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	bool bCanSprint = true;

	// Skill Montages
	UPROPERTY(EditAnywhere, Category = "Skill")
	UAnimMontage* SkillEMontage;

	UPROPERTY(EditAnywhere, Category = "Skill")
	UAnimMontage* SkillQMontage;
	
	// Mana Cost
	UPROPERTY(EditAnywhere, Category = "Skill")
	float SkillEManaCost = 30.f;

	UPROPERTY(EditAnywhere, Category = "Skill")
	float SkillQManaCost = 80.f;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite)
	bool bIsUsingSkill = false;
	
	// Attack
	
	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* HeavyAttackMontage;
	
	void InputHeavyAttack(const FInputActionValue& InValue);

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite)
	bool bIsHeavyAttacking = false;
	
	UPROPERTY(EditDefaultsOnly, Category="Hit")
	TObjectPtr<UAnimMontage> HitReactMontage_Front;

	UPROPERTY(EditDefaultsOnly, Category="Hit")
	TObjectPtr<UAnimMontage> HitReactMontage_Back;

	UPROPERTY(EditDefaultsOnly, Category="Hit")
	TObjectPtr<UAnimMontage> HitReactMontage_Left;

	UPROPERTY(EditDefaultsOnly, Category="Hit")
	TObjectPtr<UAnimMontage> HitReactMontage_Right;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Hit")
	bool bIsHit = false;

public:
	void Attack();

	UFUNCTION()
	void DealDamage();

	UFUNCTION()
	virtual void HandleOnCheckHit();

	UFUNCTION()
	void HandleOnCheckInputAttack();

	virtual void BeginAttack();

	UFUNCTION()
	virtual void EndAttack(UAnimMontage* InMontage, bool bInterruped);
	
	void HandleOnCheckInputAttack_FromNotify(UAnimInstance* Anim);
	
	void TriggerHitReact_FromAttacker(AActor* Attacker);

protected:
	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* SpringArm;
	UPROPERTY(VisibleAnywhere)
	UCameraComponent* Camera;

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* AttackMontage;

	void InputAttackMelee(const FInputActionValue& InValue);

protected:
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	UAFAttributeComponent* AttributeComp; // 캐릭터 속성 관리 component

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Component")
	UAFStatusEffectComponent* StatusEffectComp; //캐릭터 스킬 효과 관리 component
	
	// 스프린트 입력 상태(누르고 있는지)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Movement")
	bool bSprintHeld = false;

	// 실제 이동 중인지(입력/속도 기반)
	bool IsActuallyMoving() const;

	FString AttackAnimMontageSectionPrefix = FString(TEXT("Attack"));

	int32 MaxComboCount = 3;

	int32 CurrentComboCount = 0;

	bool bIsNowAttacking = false;

	bool bIsAttackKeyPressed = false;

	FOnMontageEnded OnMeleeAttackMontageEndedDelegate;

	UFUNCTION(NetMulticast, Reliable)
	virtual void Multicast_PlaySkillEMontage();

	UFUNCTION(NetMulticast, Reliable)
	virtual void Multicast_PlaySkillQMontage();
	
	UFUNCTION(Server, Reliable)
	void ServerRPC_HeavyAttack();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayHeavyAttackMontage();
	
	// 이동 잠금
	bool bMovementLocked = false;

	void LockMovement();
	void UnlockMovement();
	
	// 서버로 공격 요청을 보내는 함수 (클라이언트에서 호출)
	UFUNCTION(Server, Reliable)
	void ServerAttackRequest();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayAttackMontage();

	virtual void OnRep_PlayerState() override;
	
	// 콤보 서버 함수
	UFUNCTION(Server, Reliable)
	void Server_DoComboAttack();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayComboSection(int32 ComboCount);

	// 스킬 범위 함수
	UFUNCTION()
	virtual void HandleSkillHitCheck(float Radius, float Damage, float RotationOffset = 0.f);


	// 아군인지 확인하는 함수
	bool IsAlly(AActor* InTargetActor);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayHitReact(EAFHitDir Dir);
	EAFHitDir CalcHitDir(AActor* Attacker) const;


	// 이동속도 버프

	protected:
		// 캐릭터마다 다른 기본 속도를 저장
		float DefaultMaxWalkSpeed;

		FTimerHandle SpeedBuffTimerHandle;

public:
	void ApplySpeedBuff(float Multiplier, float Duration);







	// 데이터 테이블

	protected:
		// --- 데이터 테이블 참조 ---

			
		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AF | Data")
		FName CharacterKey;

		UPROPERTY(EditAnywhere, Category = "AF | Data")
		TObjectPtr<class UDataTable> SkillDataTable;

		UPROPERTY(EditAnywhere, Category = "AF | Data")
		TObjectPtr<class UDataTable> StatDataTable;

		// --- 로드된 데이터 저장 변수 ---
		UPROPERTY(BlueprintReadOnly, Category = "AF | Stat")
		FAFPlayerCharacterStatRow BaseStats;

		UPROPERTY(Replicated, BlueprintReadOnly, Category = "AF | Skill")
		TArray<FAFSkillInfo> CharacterSkills;

		// --- 초기화 함수 ---
		/** 에디터에서 설정한 캐릭터 이름(RowName)을 기반으로 모든 정보를 로드합니다. */
		void InitializeCharacterData(FString CharacterName);


		public:
			// 서버에서 호출될 사망 처리 함수
			UFUNCTION(BlueprintCallable, Category = "AF|Combat")
			void StartDeath(AController* LastInstigator = nullptr);

protected:
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnDeath();

	void NotifyActorBeginOverlap(AActor* OtherActor); // 물에 빠졌을 때

	UPROPERTY(EditDefaultsOnly, Category = "AF|Animation")
	TObjectPtr<UAnimMontage> DeathMontage;
};