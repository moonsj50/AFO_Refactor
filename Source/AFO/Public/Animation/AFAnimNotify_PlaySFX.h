#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AFAnimNotify_PlaySFX.generated.h"

UCLASS()
class AFO_API UAFAnimNotify_PlaySFX : public UAnimNotify
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SFX")
	TArray<USoundBase*> Sounds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SFX")
	float VolumeMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SFX")
	float PitchMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SFX")
	bool bAttachToMesh = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SFX")
	FName AttachSocketName = NAME_None;

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};
