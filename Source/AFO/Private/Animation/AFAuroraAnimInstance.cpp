#include "Animation/AFAuroraAnimInstance.h"
#include "Character/AFAurora.h"

void UAFAuroraAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	AAFAurora* Aurora = Cast<AAFAurora>(OwnerCharacter);
	if (!Aurora) return;

	AimYaw   = Aurora->AimYaw;
	AimPitch = Aurora->AimPitch;
	AimAlpha = Aurora->AimAlpha;
}
