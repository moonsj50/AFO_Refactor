    // AFSkillComponent.h

    #pragma once

    #include "CoreMinimal.h"
    #include "Components/ActorComponent.h"
    #include "Types/AFGameTypes.h"
    #include "AFSkillComponent.generated.h"

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCooldownStarted, FName, SkillRowName, float, CooldownTime);


    USTRUCT(BlueprintType)
        struct FAFCooldownInfo
    {
        GENERATED_BODY()

        UPROPERTY()
        FName SkillRowName = NAME_None;

        UPROPERTY()
        float ServerEndTime = 0.f; // (서버 시간 기준) 쿨타임이 끝날 시각

        UPROPERTY()
        float MaxCooldown = 0.f;

        FAFCooldownInfo() : SkillRowName(NAME_None), ServerEndTime(0.f), MaxCooldown(0.f) {}
    };


    UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
    class AFO_API UAFSkillComponent : public UActorComponent
    {
	    GENERATED_BODY()
	
    public:
        UAFSkillComponent();

        UPROPERTY(BlueprintAssignable, Category = "Events")
        FOnCooldownStarted OnCooldownStarted;

        /** 스킬 사용 가능 여부 확인 (서버/클라이언트 공용) */
        UFUNCTION(BlueprintCallable, Category = "AF|Skill")
        bool CanUseSkill(FName SkillRowName) const;

        /** 쿨타임 시작 (반드시 서버에서 호출) */
        void StartCooldown(FName SkillRowName, float CooldownTime);

        virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

        /** UI용: 남은 쿨타임 비율 (1.0 -> 0.0) */
        UFUNCTION(BlueprintCallable, Category = "AF|Skill")
        float GetCooldownRemainingRatio(FName SkillRowName) const;

        /** UI용: 남은 시간 (초 단위) */
        UFUNCTION(BlueprintCallable, Category = "AF|Skill")
        float GetRemainingTime(FName SkillRowName) const;

    protected:
        // 스킬별 타이머 핸들 (쿨타임 중인지 확인용)
        UPROPERTY()
        TMap<FName, FTimerHandle> SkillTimerMap;

        // 스킬별 최대 쿨타임 저장 (비율 계산용)
        UPROPERTY()
        TMap<FName, float> SkillMaxCooldownMap;

        /** 쿨타임 종료 시 호출될 콜백 */
        void OnCooldownFinished(FName SkillRowName);

        UPROPERTY(Replicated)
        TArray<FAFCooldownInfo> CooldownDataArray;
    };
