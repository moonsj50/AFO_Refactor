#include "Animation/AFAnimNotify_PlaySFX.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

void UAFAnimNotify_PlaySFX::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);

	if (!MeshComp) return;

	UWorld* World = MeshComp->GetWorld();
	if (!World) return;

	if (World->GetNetMode() == NM_DedicatedServer) return;

	if (Sounds.Num() <= 0) return;

	USoundBase* Sound = Sounds[FMath::RandRange(0, Sounds.Num() - 1)];
	if (!Sound) return;

	if (bAttachToMesh)
	{
		UGameplayStatics::SpawnSoundAttached(
			Sound,
			MeshComp,
			AttachSocketName,
			FVector::ZeroVector,
			EAttachLocation::SnapToTarget,
			true,
			VolumeMultiplier,
			PitchMultiplier
		);
	}
	else
	{
		UGameplayStatics::PlaySoundAtLocation(
			MeshComp,
			Sound,
			MeshComp->GetComponentLocation(),
			VolumeMultiplier,
			PitchMultiplier
		);
	}
}
