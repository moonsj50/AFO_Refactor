// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/AFStatusEffectComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"

UAFStatusEffectComponent::UAFStatusEffectComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UAFStatusEffectComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter) return;

	MoveComp = OwnerCharacter->GetCharacterMovement();
	if (MoveComp)
	{
		OriginalWalkSpeed = MoveComp->MaxWalkSpeed;
	}
}

void UAFStatusEffectComponent::ApplySlow(float InSlowRatio, float InDuration)
{
	//서버 전용
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;
	if (!MoveComp) return;

	//더 약한 슬로우는 무시
	if (bIsSlowed && InSlowRatio >= SlowRatio)
	{
		return;
	}

	//20% 슬로우
	SlowRatio = FMath::Clamp(InSlowRatio, 0.1f, 1.f);
	SlowDuration = InDuration;

	ApplySlow_Internal();

	// 타이머 리셋
	GetWorld()->GetTimerManager().ClearTimer(SlowTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(
		SlowTimerHandle,
		this,
		&UAFStatusEffectComponent::ClearSlow,
		SlowDuration,
		false
	);
}

void UAFStatusEffectComponent::ApplySlow_Internal()
{
	if (!MoveComp) return;

	bIsSlowed = true;
	MoveComp->MaxWalkSpeed = OriginalWalkSpeed * SlowRatio;

	UE_LOG(LogTemp, Warning, TEXT("[Slow] Applied Ratio=%.2f"), SlowRatio);
}

void UAFStatusEffectComponent::ClearSlow()
{
	if (!MoveComp) return;

	MoveComp->MaxWalkSpeed = OriginalWalkSpeed;
	bIsSlowed = false;
	SlowRatio = 1.f;

	UE_LOG(LogTemp, Warning, TEXT("[Slow] Cleared"));
}
