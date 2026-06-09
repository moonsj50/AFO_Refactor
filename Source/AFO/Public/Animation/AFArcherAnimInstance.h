#pragma once

#include "CoreMinimal.h"
#include "Animation/AFAnimInstance.h"
#include "AFArcherAnimInstance.generated.h"

class AAFArcher;

UCLASS()
class AFO_API UAFArcherAnimInstance : public UAFAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	// ===== Aim Offset =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Aim")
	float AimYaw = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Aim")
	float AimPitch = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Aim")
	float AimAlpha = 0.f;
	
	// ===== Movement Lean / Slope =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Movement")
	float LeanAngle = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Movement")
	float SlopeAngle = 0.f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="IK")
	bool bShouldDoIKTrace = false;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="IK")
	FVector LookAtLocation = FVector::ZeroVector;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="IK")
	float IKAlpha = 1.f;
};