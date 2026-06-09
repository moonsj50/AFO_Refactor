#include "Animation/AFAnimInstance.h"
#include "Character/AFPlayerCharacter.h"
#include "Character/AFDarkKnight.h"
#include "Character/AFMage.h"
#include "Character/AFAurora.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UAFAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	APawn* OwnerPawn = TryGetPawnOwner();
	if (IsValid(OwnerPawn) == true)
	{
		OwnerCharacter = Cast<AAFPlayerCharacter>(OwnerPawn);
		OwnerCharacterMovement = OwnerCharacter->GetCharacterMovement();
		bIsAttack = false;
	}
}

void UAFAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	
	if (IsValid(OwnerCharacter) == true && IsValid(OwnerCharacterMovement) == true)
	{
		Velocity = OwnerCharacterMovement->Velocity;
		GroundSpeed = UKismetMathLibrary::VSizeXY(Velocity);
		
		float GroundAcceleration = UKismetMathLibrary::VSizeXY(OwnerCharacterMovement->GetCurrentAcceleration());
		bool bIsAccelerationNearlyZero = FMath::IsNearlyZero(GroundAcceleration);
		bShouldMove = (KINDA_SMALL_NUMBER < GroundSpeed) && (bIsAccelerationNearlyZero == false);
		
		bIsFalling = OwnerCharacterMovement->IsFalling();
		
		FVector WorldInput = OwnerCharacter->GetLastMovementInputVector();
		FVector LocalInput = OwnerCharacter->GetActorRotation().UnrotateVector(WorldInput);
		
		DirectionX = LocalInput.X * 100.0f;
		DirectionY = LocalInput.Y * 100.0f;
	}
	
	if (AAFPlayerCharacter* BaseChar = Cast<AAFPlayerCharacter>(OwnerCharacter))
	{
        if (AAFMage* Mage = Cast<AAFMage>(BaseChar))
		{
			bIsSprinting = Mage->bIsSprinting;
		}
		else if (AAFAurora* Aurora = Cast<AAFAurora>(BaseChar))
		{
			bIsSprinting = Aurora->bIsSprinting;
		}
		else
		{
			// 다른 캐릭터일 경우 false
			bIsSprinting = false;
		}
	}
	
	bIsSprinting = bIsSprinting && (GroundSpeed > 3.f);
}

void UAFAnimInstance::AnimNotify_AttackHit()
{
	if (auto Owner = TryGetPawnOwner())
	{
		if (auto Character = Cast<AAFPlayerCharacter>(Owner))
		{
			Character->DealDamage();
		}
	}
}

void UAFAnimInstance::AnimNotify_CheckHit()
{
	if (OnCheckHit.IsBound() == true)
	{
		OnCheckHit.Broadcast();
	}
}


