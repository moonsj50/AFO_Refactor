#include "Animation/AFAnimNotify.h"
#include "Character/AFPlayerCharacter.h"

void UAFAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (IsValid(MeshComp) == true)
	{
		AAFPlayerCharacter* AttackingCharacter = Cast<AAFPlayerCharacter>(MeshComp->GetOwner());
		if (IsValid(AttackingCharacter) == true)
		{
			AttackingCharacter->HandleOnCheckHit();
		}
	}
}
