// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AFStatusEffectComponent.generated.h"

class ACharacter;
class UCharacterMovementComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class AFO_API UAFStatusEffectComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAFStatusEffectComponent();

	// 서버 전용 슬로우 적용
	void ApplySlow(float InSlowRatio, float InDuration);

protected:
	virtual void BeginPlay() override;

private:
	// 캐시
	UPROPERTY()
	ACharacter* OwnerCharacter = nullptr;

	UPROPERTY()
	UCharacterMovementComponent* MoveComp = nullptr;

	// 상태
	bool bIsSlowed = false;

	float OriginalWalkSpeed = 0.f;
	float SlowRatio = 1.f;
	float SlowDuration = 0.f;

	FTimerHandle SlowTimerHandle;

	void ApplySlow_Internal();

	void ClearSlow();

	
};
