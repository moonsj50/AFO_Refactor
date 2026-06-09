#include "Animation/AFArcherAnimInstance.h"
#include "Character/AFArcher.h"
#include "GameFramework/CharacterMovementComponent.h"

void UAFArcherAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	AAFArcher* Archer = Cast<AAFArcher>(OwnerCharacter);
	if (!Archer || !OwnerCharacterMovement) return;

	// ===== Aim Offset =====
	AimYaw   = Archer->AimYaw;
	AimPitch = Archer->AimPitch;
	AimAlpha = Archer->AimAlpha;

	// ===== Lean (좌우 기울기) =====
	FVector CurrentVelocity = OwnerCharacterMovement->Velocity;
	FVector Right = Archer->GetActorRightVector();

	float RightSpeed = FVector::DotProduct(Right, CurrentVelocity);
	LeanAngle = FMath::Clamp(RightSpeed / 300.f, -1.f, 1.f);

	// ===== Slope (경사각) =====
	FVector FloorNormal = OwnerCharacterMovement->CurrentFloor.HitResult.Normal;
	SlopeAngle = FMath::RadiansToDegrees(
		FMath::Acos(FVector::DotProduct(FloorNormal, FVector::UpVector))
	);
}