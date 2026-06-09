#include "Character/AFArcher.h"
#include "GameFramework/Controller.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Components/AFAttributeComponent.h"
#include "Components/AFStatusEffectComponent.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "CollisionShape.h"

AAFArcher::AAFArcher()
{
	PrimaryActorTick.bCanEverTick = true;
	bCanSprint = false;

	AimYaw = 0.f;
	AimPitch = 0.f;
	AimAlpha = 1.f;
	
	// 기본 이동 속도 설정
	GetCharacterMovement()->MaxWalkSpeed = 600.f;
}

void AAFArcher::BeginPlay()
{
	Super::BeginPlay();
}

void AAFArcher::HandleOnCheckHit()
{
	Super::HandleOnCheckHit();
	
	if (!HasAuthority()) return;

	UAnimInstance* Anim = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (!Anim) return;

	// 기본 공격(콤보)
	if (AttackMontage && Anim->Montage_IsPlaying(AttackMontage))
	{
		ArcherDealDamage(Archer_BasicForwardDistance, Archer_BasicRadius, Archer_BasicDamage);
		return;
	}

	// 강공격
	if (HeavyAttackMontage && Anim->Montage_IsPlaying(HeavyAttackMontage))
	{
		ArcherSkillHitCheck(Archer_HeavyForwardDistance, Archer_HeavyRadius, Archer_HeavyDamage, 0.f, false);
		return;
	}

	// E 스킬
	if (SkillEMontage && Anim->Montage_IsPlaying(SkillEMontage))
	{
		ArcherSkillHitCheck(Archer_SkillEForwardDistance, Archer_SkillERadius, Archer_SkillEDamage, 0.f, true);
		return;
	}

	// Q 스킬
	if (SkillQMontage && Anim->Montage_IsPlaying(SkillQMontage))
	{
		ArcherSkillHitCheck(Archer_SkillQForwardDistance, Archer_SkillQRadius, Archer_SkillQDamage, 0.f, true);
		return;
	}
}

void AAFArcher::ServerRPC_SkillE_Implementation()
{
	if (!HasAuthority()) return;

	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;

	if (Now < NextSkillETime_Archer)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Archer] SkillE Cooldown. Remain=%.2f"),
			NextSkillETime_Archer - Now);
		return;
	}

	NextSkillETime_Archer = Now + Archer_SkillECooldown;

	UE_LOG(LogTemp, Warning, TEXT("[Archer] SkillE Cast OK. Next=%.2f"), NextSkillETime_Archer);

	if (bIsAttacking || bIsUsingSkill) return;
	bIsUsingSkill = true;

	Multicast_PlaySkillEMontage();
}

void AAFArcher::ServerRPC_SkillQ_Implementation()
{
	if (!HasAuthority()) return;

	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;

	if (Now < NextSkillQTime_Archer)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Archer] SkillQ Cooldown. Remain=%.2f"),
			NextSkillQTime_Archer - Now);
		return;
	}

	NextSkillQTime_Archer = Now + Archer_SkillQCooldown;

	UE_LOG(LogTemp, Warning, TEXT("[Archer] SkillQ Cast OK. Next=%.2f"), NextSkillQTime_Archer);

	if (bIsAttacking || bIsUsingSkill) return;
	bIsUsingSkill = true;

	Multicast_PlaySkillQMontage();
}

void AAFArcher::ArcherDealDamage(float ForwardDistance, float Radius, float Damage)
{
	if (!HasAuthority()) return;

	const FVector Center = GetActorLocation()
		+ (GetActorForwardVector() * ForwardDistance)
		+ FVector(0, 0, 90.f);

	TArray<FOverlapResult> OverlapResults;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(Radius);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	const bool bHit = GetWorld()->OverlapMultiByChannel(
		OverlapResults, Center, FQuat::Identity, ECC_Pawn, Sphere, Params
	);

	if (!bHit) return;

	for (const FOverlapResult& Result : OverlapResults)
	{
		AActor* HitActor = Result.GetActor();
		if (!HitActor || HitActor == this) continue;
		if (IsAlly(HitActor)) continue;

		if (UAFAttributeComponent* Attr = HitActor->FindComponentByClass<UAFAttributeComponent>())
		{
			Attr->ApplyDamage(Damage, GetController());
		}

		if (AAFPlayerCharacter* Victim = Cast<AAFPlayerCharacter>(HitActor))
		{
			Victim->TriggerHitReact_FromAttacker(this);
		}
	}
}

void AAFArcher::ArcherSkillHitCheck(float ForwardDistance, float Radius, float Damage, float RotationOffset,
	bool bApplySlow)
{
	if (!HasAuthority()) return;

	FVector Forward = GetActorForwardVector();
	FVector SkillDirection = Forward.RotateAngleAxis(RotationOffset, FVector(0, 0, 1));

	const FVector SphereLocation = GetActorLocation()
		+ (SkillDirection * ForwardDistance)
		+ FVector(0, 0, 60.f);

	TArray<FOverlapResult> OverlapResults;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(Radius);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	const bool bHit = GetWorld()->OverlapMultiByChannel(
		OverlapResults, SphereLocation, FQuat::Identity, ECC_Pawn, Sphere, Params
	);

	if (!bHit) return;

	for (const FOverlapResult& Result : OverlapResults)
	{
		AActor* HitActor = Result.GetActor();
		if (!HitActor || HitActor == this) continue;
		if (IsAlly(HitActor)) continue;

		if (UAFAttributeComponent* Attr = HitActor->FindComponentByClass<UAFAttributeComponent>())
		{
			Attr->ApplyDamage(Damage, GetController());
		}

		if (AAFPlayerCharacter* Victim = Cast<AAFPlayerCharacter>(HitActor))
		{
			Victim->TriggerHitReact_FromAttacker(this);
		}

		if (bApplySlow)
		{
			if (UAFStatusEffectComponent* StatusComp = HitActor->FindComponentByClass<UAFStatusEffectComponent>())
			{
				StatusComp->ApplySlow(0.2f, 0.5f);
			}
		}
	}
}

void AAFArcher::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!Controller) return;

	const FRotator ControlRot = Controller->GetControlRotation();
	const FRotator ActorRot = GetActorRotation();
	const FRotator DeltaRot = (ControlRot - ActorRot).GetNormalized();

	AimYaw   = DeltaRot.Yaw;
	AimPitch = DeltaRot.Pitch;
	AimAlpha = 1.f;
}
