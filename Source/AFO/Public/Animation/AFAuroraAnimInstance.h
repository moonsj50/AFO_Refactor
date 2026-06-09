#pragma once

#include "CoreMinimal.h"
#include "Animation/AFAnimInstance.h"
#include "AFAuroraAnimInstance.generated.h"

class AAFAurora;

UCLASS()
class AFO_API UAFAuroraAnimInstance : public UAFAnimInstance
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
};
