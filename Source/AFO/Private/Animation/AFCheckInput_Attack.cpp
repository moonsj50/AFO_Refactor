#include "Animation/AFCheckInput_Attack.h"
#include "Character/AFPlayerCharacter.h"

void UAFCheckInput_Attack::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	AAFPlayerCharacter* C = Cast<AAFPlayerCharacter>(MeshComp->GetOwner());
	if (!C) return;

	// 핵심: 메인 Mesh가 아니면 무시 (중복 Notify 차단)
	if (MeshComp != C->GetMesh()) return;

	UAnimInstance* Anim = C->GetMesh()->GetAnimInstance();
	C->HandleOnCheckInputAttack_FromNotify(Anim);
}