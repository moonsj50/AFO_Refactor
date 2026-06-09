#include "Animation/AFMageAnimInstance.h"
#include "Character/AFMage.h"
#include "GameFramework/CharacterMovementComponent.h"

void UAFMageAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	AAFMage* Mage = Cast<AAFMage>(OwnerCharacter);
	if (!Mage || !OwnerCharacterMovement) return;

	// ===== Aim Offset =====
	AimYaw   = Mage->AimYaw;
	AimPitch = Mage->AimPitch;
	AimAlpha = Mage->AimAlpha;

	// ===== Lean =====
	FVector CurrentVelocity = OwnerCharacterMovement->Velocity;
	FVector Right = Mage->GetActorRightVector();

	float RightSpeed = FVector::DotProduct(Right, CurrentVelocity);
	LeanAngle = FMath::Clamp(RightSpeed / 300.f, -1.f, 1.f);

	// ===== Slope =====
	FVector FloorNormal = OwnerCharacterMovement->CurrentFloor.HitResult.Normal;
	SlopeAngle = FMath::RadiansToDegrees(
		FMath::Acos(FVector::DotProduct(FloorNormal, FVector::UpVector))
	);
}
