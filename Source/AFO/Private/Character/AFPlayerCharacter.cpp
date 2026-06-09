// AFPlayerCharacter.cpp

#include "Character/AFPlayerCharacter.h"
#include "Components/AFAttributeComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "Animation/AFAnimInstance.h"
#include "Player/AFPlayerController.h"
#include "Net/UnrealNetwork.h"
#include "Player/AFPlayerState.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/OverlapResult.h"
#include "Animation/AnimInstance.h"
#include "UI/AFHealthBarWidget.h"
#include "Blueprint/UserWidget.h"
#include <Components/WidgetComponent.h>
#include "Game/AFGameMode.h"
#include "Components/AFStatusEffectComponent.h"
#include "Gimmick/AFBuffItem.h"
#include "Components/CapsuleComponent.h"

AAFPlayerCharacter::AAFPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	AttributeComp = CreateDefaultSubobject<UAFAttributeComponent>(TEXT("AttributeComponent"));
	StatusEffectComp = CreateDefaultSubobject<UAFStatusEffectComponent>(TEXT("StatusEffectComponent"));

	NormalSpeed = 400.f;
	SprintSpeedMultiplier = 1.5f;
	SprintSpeed = NormalSpeed * SprintSpeedMultiplier;
	GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;

	// 스프링암 생성
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 400.f;
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->SetRelativeRotation(FRotator(-70.f, 0.f, 0.f));

	// 카메라 생성
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
	Camera->bUsePawnControlRotation = false;
	bUseControllerRotationYaw = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;
}

void AAFPlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAFPlayerCharacter, bIsAttacking);
	DOREPLIFETIME(AAFPlayerCharacter, CharacterSkills);
	DOREPLIFETIME(AAFPlayerCharacter, bIsUsingSkill);
	DOREPLIFETIME(AAFPlayerCharacter, bIsHeavyAttacking);
}

void AAFPlayerCharacter::ServerRPC_SkillE_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("Server: SkillE Input received"));

	if (bIsAttacking || bIsUsingSkill) return;

	AAFPlayerState* PS = GetPlayerState<AAFPlayerState>();
	if (!PS) return;

	if (!PS->ConsumeMana(SkillEManaCost))
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillE 실패: 서버에서 Mana 부족 확인"));
		return;
	}

	if (SkillEMontage)
	{
		bIsUsingSkill = true;
		// 서버에서 몽타주를 재생하면 설정에 따라 클라이언트들에게 복제
		Multicast_PlaySkillEMontage();
	}
}

void AAFPlayerCharacter::ServerRPC_SkillQ_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("Server: SkillQ Input received"));

	if (bIsAttacking || bIsUsingSkill) return;

	AAFPlayerState* PS = GetPlayerState<AAFPlayerState>();
	if (!PS) return;

	if (!PS->ConsumeMana(SkillQManaCost))
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillE 실패: 서버에서 Mana 부족 확인"));
		return;
	}

	if (SkillQMontage)
	{
		bIsUsingSkill = true;
		// 서버에서 몽타주를 재생하면 설정에 따라 클라이언트들에게 복제
		Multicast_PlaySkillQMontage();
	}
}

void AAFPlayerCharacter::Multicast_PlaySkillEMontage_Implementation()
{
	// 서버와 모든 클라이언트에서 동시에 실행됨
	if (SkillEMontage)
	{
		bIsUsingSkill = true;
		
		LockMovement();
		PlayAnimMontage(SkillEMontage);
	}
}

void AAFPlayerCharacter::Multicast_PlaySkillQMontage_Implementation()
{
	// 서버와 모든 클라이언트에서 동시에 실행됨
	if (SkillQMontage)
	{
		bIsUsingSkill = true;
		
		LockMovement();  
		PlayAnimMontage(SkillQMontage);
	}
}

void AAFPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (!CharacterKey.IsNone())
	{
		InitializeCharacterData(CharacterKey.ToString());
	}

	if (!CharacterKey.IsNone())
	{
		InitializeCharacterData(CharacterKey.ToString());
	}

	// 생성 시점에 이 캐릭터의 고유 속도를 저장
	if (GetCharacterMovement())
	{
		DefaultMaxWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
	}

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->OnMontageEnded.AddDynamic(this, &AAFPlayerCharacter::OnAttackMontageEnded);
	}

	if (GetNetMode() != NM_DedicatedServer)
	{
		// 1. 위젯 컴포넌트 찾기 (이름으로 찾거나 클래스로 찾기)
		UWidgetComponent* HPBarComp = FindComponentByClass<UWidgetComponent>();
		if (HPBarComp)
		{
			// 2. 생성된 위젯 인스턴스를 우리 C++ 클래스로 캐스팅
			UAFHealthBarWidget* HPWidget = Cast<UAFHealthBarWidget>(HPBarComp->GetUserWidgetObject());
			if (HPWidget)
			{
				// 3. ★ 여기서 깨워줘야 타이머가 돌기 시작합니다!
				HPWidget->BindToCharacter(this);
				UE_LOG(LogTemp, Warning, TEXT("[Character] Success: BindToCharacter called for %s"), *GetName());
			}
			else
			{
				// 캐스팅 실패 시 로그 (부모 클래스 설정 확인용)
				UE_LOG(LogTemp, Error, TEXT("[Character] Failed to Cast Widget for %s! Check Parent Class."), *GetName());
			}
		}
	}

}

void AAFPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (AAFPlayerController* PlayerController = Cast<AAFPlayerController>(GetController()))
		{
			if (PlayerController->MoveAction)
			{
				EnhancedInput->BindAction(
					PlayerController->MoveAction,
					ETriggerEvent::Triggered,
					this,
					&AAFPlayerCharacter::Move
				);
			}

			if (PlayerController->LookAction)
			{
				EnhancedInput->BindAction(
					PlayerController->LookAction,
					ETriggerEvent::Triggered,
					this,
					&AAFPlayerCharacter::Look
				);
			}

			if (PlayerController->JumpAction)
			{
				EnhancedInput->BindAction(
					PlayerController->JumpAction,
					ETriggerEvent::Triggered,
					this,
					&AAFPlayerCharacter::StartJump
				);

				EnhancedInput->BindAction(
					PlayerController->JumpAction,
					ETriggerEvent::Completed,
					this,
					&AAFPlayerCharacter::StopJump
				);
			}

			if (PlayerController->SprintAction)
			{
				EnhancedInput->BindAction(
					PlayerController->SprintAction,
					ETriggerEvent::Started,
					this,
					&AAFPlayerCharacter::StartSprint
				);

				EnhancedInput->BindAction(
					PlayerController->SprintAction,
					ETriggerEvent::Completed,
					this,
					&AAFPlayerCharacter::StopSprint
				);
			}

			if (PlayerController->SkillEAction)
			{
				EnhancedInput->BindAction(
					PlayerController->SkillEAction,
					ETriggerEvent::Started,
					this,
					&AAFPlayerCharacter::ServerRPC_SkillE
				);
			}

			if (PlayerController->SkillQAction)
			{
				EnhancedInput->BindAction(
					PlayerController->SkillQAction,
					ETriggerEvent::Started,
					this,
					&AAFPlayerCharacter::ServerRPC_SkillQ
				);
			}
			
			if (PlayerController->AttackAction)
			{
				EnhancedInput->BindAction(
					PlayerController->AttackAction,
					ETriggerEvent::Started,
					this,
					&ThisClass::InputAttackMelee
				);
			}
			
			if (PlayerController->HeavyAttackAction)
			{
				EnhancedInput->BindAction(
					PlayerController->HeavyAttackAction,
					ETriggerEvent::Started,
					this,
					&ThisClass::InputHeavyAttack
				);
			}
		}
	}
}

void AAFPlayerCharacter::Move(const FInputActionValue& value)
{
	if (bMovementLocked) return;
	if (bIsUsingSkill) return;
	if (bIsHeavyAttacking) return;
	if (!Controller) return;

	const FVector2D MoveInput = value.Get<FVector2D>();
	if (MoveInput.IsNearlyZero()) return;

	// W / S 처리 (MoveInput.Y)
	if (MoveInput.X > 0.f)
	{
		// W
		AddMovementInput(ForwardDir, MoveInput.X * ForwardSpeed);
	}
	else if (MoveInput.X < 0.f)
	{
		// S
		AddMovementInput(ForwardDir, MoveInput.X * BackwardSpeed);
	}

	// A / D 처리 (MoveInput.X)
	if (MoveInput.Y > 0.f)
	{
		// D
		AddMovementInput(RightDir, MoveInput.Y * RightSpeed);
	}
	else if (MoveInput.Y < 0.f)
	{
		// A
		AddMovementInput(RightDir, MoveInput.Y * LeftSpeed);
	}

	// 컨트롤러(카메라)의 회전
	const FRotator ControlRot = Controller->GetControlRotation();
	const FRotator YawRot(0.f, ControlRot.Yaw, 0.f);

	// 카메라 기준 Forward / Right 벡터 생성
	ForwardDir = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
	RightDir = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);
}

void AAFPlayerCharacter::StartJump(const FInputActionValue& value)
{
	UE_LOG(LogTemp, Warning, TEXT(" Jump "));

	if (value.Get<bool>())
	{
		Jump();
	}
}

void AAFPlayerCharacter::StopJump(const FInputActionValue& value)
{
	if (!value.Get<bool>())
	{
		StopJumping();
	}
}

void AAFPlayerCharacter::Look(const FInputActionValue& value)
{
	FVector2D LookInput = value.Get<FVector2D>();

	AddControllerYawInput(LookInput.X);
	AddControllerPitchInput(LookInput.Y);
}

void AAFPlayerCharacter::StartSprint(const FInputActionValue& value)
{
	if (!bCanSprint) return;

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
	}
	
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
}

void AAFPlayerCharacter::StopSprint(const FInputActionValue& value)
{
	if (!bCanSprint) return;

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
	}
}

void AAFPlayerCharacter::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == HeavyAttackMontage)
	{
		bIsAttacking = false;
		bIsHeavyAttacking = false;

		if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
		{
			MoveComp->SetMovementMode(MOVE_Walking);
		}

		if (!bIsUsingSkill)
		{
			UnlockMovement();
		}
	}

	if (Montage == SkillEMontage || Montage == SkillQMontage)
	{
		bIsUsingSkill = false;

		if (!bIsHeavyAttacking)
		{
			UnlockMovement();
		}
	}

	if (Montage == AttackMontage)
	{
		bIsAttacking = false;
	}
	if (Montage == HitReactMontage_Front.Get() ||
		Montage == HitReactMontage_Back.Get() ||
		Montage == HitReactMontage_Left.Get() ||
		Montage == HitReactMontage_Right.Get())
	{
		bIsHit = false;
	}
}

void AAFPlayerCharacter::HandleOnCheckHit()
{
	if (!HasAuthority()) return;

	UAnimInstance* Anim = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (!Anim) return;

	UAnimMontage* CurrentMontage = Anim->GetCurrentActiveMontage();
	if (!CurrentMontage) return;

	UE_LOG(LogTemp, Log, TEXT("HandleOnCheckHit Called! Montage: %s"), *CurrentMontage->GetName());

	FString MontageName = CurrentMontage->GetName();

	if (AttackMontage && Anim->Montage_IsPlaying(AttackMontage))
	{
		DealDamage();
	}

	else if (HeavyAttackMontage && Anim->Montage_IsPlaying(HeavyAttackMontage))
	{
		HandleSkillHitCheck(180.f, 35.f, 0.f);
	}

	else if (SkillEMontage && Anim->Montage_IsPlaying(SkillEMontage))
	{
		HandleSkillHitCheck(250.f, 40.f, 0.f);
	}
	else if (SkillQMontage && Anim->Montage_IsPlaying(SkillQMontage))
	{
		HandleSkillHitCheck(320.f, 80.f, 0.f);
	}
}

void AAFPlayerCharacter::HandleOnCheckInputAttack()
{
	UE_LOG(LogTemp, Warning, TEXT("Handle CALLED. pressed=%d combo=%d"),
		bIsAttackKeyPressed, CurrentComboCount);

	UAnimInstance* Anim = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	const bool bPlaying = Anim ? Anim->Montage_IsPlaying(AttackMontage) : false;
	UE_LOG(LogTemp, Warning, TEXT("IsPlaying AttackMontage=%d"), bPlaying);

	if (!bPlaying)
	{
		UE_LOG(LogTemp, Warning, TEXT("Skip Jump: AttackMontage not playing now"));
		return;
	}

	if (bIsAttackKeyPressed)
	{
		CurrentComboCount = FMath::Clamp(CurrentComboCount + 1, 1, MaxComboCount);

		FName NextSectionName = *FString::Printf(TEXT("%s%02d"),
			*AttackAnimMontageSectionPrefix, CurrentComboCount);

		UE_LOG(LogTemp, Warning, TEXT("JumpToSection => %s"), *NextSectionName.ToString());

		Anim->Montage_JumpToSection(NextSectionName, AttackMontage);
		bIsAttackKeyPressed = false;
	}
}

void AAFPlayerCharacter::EndAttack(UAnimMontage* InMontage, bool bInterruped)
{
	UE_LOG(LogTemp, Warning, TEXT("EndAttack CALLED. Montage=%s Interrupted=%d Combo=%d"),*GetNameSafe(InMontage), bInterruped, CurrentComboCount);
	if (CurrentComboCount == 0 && !bIsNowAttacking)
	{
		return;
	}

	CurrentComboCount = 0;
	bIsAttackKeyPressed = false;
	bIsNowAttacking = false;

	if (OnMeleeAttackMontageEndedDelegate.IsBound() == true)
	{
		OnMeleeAttackMontageEndedDelegate.Unbind();
	}
}

bool AAFPlayerCharacter::IsActuallyMoving() const
{
	if (!GetCharacterMovement()) return false;

	// 속도 기반(서버/클라 공통으로 안정적)
	const float Speed2D = GetVelocity().Size2D();
	const bool bHasSpeed = Speed2D > 3.f; // 약간의 오차 제거용

	return bHasSpeed;
}

void AAFPlayerCharacter::BeginAttack()
{
	bIsNowAttacking = true;

	const float PlayedLen = PlayAnimMontage(AttackMontage, 1.f, FName("Attack01"));
	UE_LOG(LogTemp, Warning, TEXT("BeginAttack PlayAnimMontage Len=%.3f Montage=%s"),
		PlayedLen, *GetNameSafe(AttackMontage));

	if (PlayedLen <= 0.f)
	{
		UE_LOG(LogTemp, Error, TEXT("BeginAttack FAILED: montage not played (slot/animgraph 확인)"));
		bIsNowAttacking = false;
		return;
	}

	CurrentComboCount = 1;

	UAnimInstance* Anim = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (Anim && !OnMeleeAttackMontageEndedDelegate.IsBound())
	{
		OnMeleeAttackMontageEndedDelegate.BindUObject(this, &ThisClass::EndAttack);
		Anim->Montage_SetEndDelegate(OnMeleeAttackMontageEndedDelegate, AttackMontage);
	}
}

void AAFPlayerCharacter::InputHeavyAttack(const FInputActionValue& InValue)
{
	if (GetCharacterMovement()->IsFalling()) return;
	if (bIsUsingSkill) return;
	if (!HeavyAttackMontage) return;

	// 콤보(좌클릭) 중이면 강공격 막기 (원하면 삭제 가능)
	if (CurrentComboCount > 0 || bIsNowAttacking) return;

	UAnimInstance* Anim = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (!Anim) return;

	// 1) 연타해도 재시작(칠랑말랑) 안 되게 막기
	if (bIsHeavyAttacking) return;
	if (Anim->Montage_IsPlaying(HeavyAttackMontage)) return;

	bIsAttacking = true;
	bIsHeavyAttacking = true;

	// 2) 움직이면서 사용 방지: 입력 무시 + 즉시 멈춤 + 이동모드 None
	LockMovement(); // 네가 만든 함수(StopMovementImmediately + IgnoreMoveInput)
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
		MoveComp->DisableMovement();
	}

	const float Len = PlayAnimMontage(HeavyAttackMontage);
	UE_LOG(LogTemp, Warning, TEXT("HeavyAttack PlayAnimMontage Len=%.3f Montage=%s"),
		Len, *GetNameSafe(HeavyAttackMontage));

	if (Len <= 0.f)
	{
		// 재생 실패 시 원복
		bIsAttacking = false;
		bIsHeavyAttacking = false;

		if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
		{
			MoveComp->SetMovementMode(MOVE_Walking);
		}
		UnlockMovement();
	}
	
	ServerRPC_HeavyAttack();
}

void AAFPlayerCharacter::Attack()
{
	if (bIsAttacking) return;

	// 클라이언트에서 Server RPC 호출
	ServerAttackRequest();
}

void AAFPlayerCharacter::DealDamage()
{
	if (!HasAuthority()) return;

	FVector Center = GetActorLocation() + (GetActorForwardVector() * 120.f) + FVector(0, 0, 90.f);
	float Radius = 120.f;

	TArray<FOverlapResult> OverlapResults;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(Radius);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	
	const bool bHit = GetWorld()->OverlapMultiByChannel
	(
	OverlapResults,
	Center,
	FQuat::Identity,
	ECC_Pawn,
	Sphere,
	Params
	);
	/*bool bHit = GetWorld()->OverlapMultiByChannel(
		OverlapResults, Center, FQuat::Identity, ECC_Pawn, Sphere, Params
	);*/
	
	if (!bHit) return;
	{
		for (const FOverlapResult& Result : OverlapResults)
		{
			AActor* HitActor = Result.GetActor();
			if (!HitActor) continue;
			if (HitActor == this) continue;
			if (IsAlly(HitActor)) continue;

			if (UAFAttributeComponent* Attr = HitActor->FindComponentByClass<UAFAttributeComponent>())
			{
				Attr->ApplyDamage(20.f, GetController());
			}

			if (AAFPlayerCharacter* Victim = Cast<AAFPlayerCharacter>(HitActor))
			{
				Victim->TriggerHitReact_FromAttacker(this);
			}
		}
	}
}

void AAFPlayerCharacter::ServerAttackRequest_Implementation()
{
	// 서버에서만 실행됩니다.

	AAFPlayerState* PS = GetPlayerState<AAFPlayerState>();
	if (!PS)
	{
		return;
	}

	const float AttackManaCost = 5.f;

	// Mana 부족 체크는 서버에서 안전하게 처리됩니다.
	if (!PS->ConsumeMana(AttackManaCost))
	{
		return;
	}

	// Mana 충분 → 공격 실행 (서버)
	if (AttackMontage)
	{
		bIsAttacking = true;

		MulticastPlayAttackMontage();
		DealDamage();

		UE_LOG(LogTemp, Warning, TEXT("Attack 성공: Mana %.1f 소모 (서버)"), AttackManaCost);
	}
}

void AAFPlayerCharacter::MulticastPlayAttackMontage_Implementation()
{
	// 모든 클라이언트 (서버 포함)에서 모션을 재생
	PlayAnimMontage(AttackMontage);
}

void AAFPlayerCharacter::ServerRPC_HeavyAttack_Implementation()
{
	if (GetCharacterMovement()->IsFalling()) return;
	if (bIsUsingSkill) return;
	if (!HeavyAttackMontage) return;

	// 콤보(좌클릭) 중이면 강공격 막기
	if (CurrentComboCount > 0 || bIsNowAttacking) return;

	UAnimInstance* Anim = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (!Anim) return;

	// 연타 재시작 방지
	if (bIsHeavyAttacking) return;
	if (Anim->Montage_IsPlaying(HeavyAttackMontage)) return;

	bIsAttacking = true;
	bIsHeavyAttacking = true;

	Multicast_PlayHeavyAttackMontage();
}

void AAFPlayerCharacter::Multicast_PlayHeavyAttackMontage_Implementation()
{
	if (!HeavyAttackMontage) return;

	bIsAttacking = true;
	bIsHeavyAttacking = true;

	LockMovement();

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
		MoveComp->DisableMovement();
	}

	const float Len = PlayAnimMontage(HeavyAttackMontage);
	UE_LOG(LogTemp, Warning, TEXT("HeavyAttack Multicast Play Len=%.3f Montage=%s"),
		Len, *GetNameSafe(HeavyAttackMontage));
}

void AAFPlayerCharacter::LockMovement()
{
	if (bMovementLocked) return;
	bMovementLocked = true;

	// 이미 움직이고 있던 속도/가속 끊기
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
	}

	// 입력으로 인한 이동만 막기(몽타주 루트모션은 필요하면 유지 가능)
	if (AController* C = GetController())
	{
		C->SetIgnoreMoveInput(true);
	}
}

void AAFPlayerCharacter::UnlockMovement()
{
	bMovementLocked = false;

	if (AController* C = GetController())
	{
		C->SetIgnoreMoveInput(false);
	}
}

void AAFPlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

}

void AAFPlayerCharacter::InputAttackMelee(const FInputActionValue& InValue)
{
	Server_DoComboAttack();
}

void AAFPlayerCharacter::Server_DoComboAttack_Implementation()
{
	if (!bIsNowAttacking)
	{
		bIsNowAttacking = true;
		CurrentComboCount = 1;
		bIsAttackKeyPressed = false;

		Multicast_PlayComboSection(1);
	}
	else
	{
		bIsAttackKeyPressed = true;
	}
}

void AAFPlayerCharacter::Multicast_PlayComboSection_Implementation(int32 ComboCount)
{
	UAnimInstance* Anim = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (!Anim || !AttackMontage) return;

	if (ComboCount == 1)
	{
		PlayAnimMontage(AttackMontage, 1.f, FName("Attack01"));

		// 종료 시점 리셋을 위한 델리게이트
		OnMeleeAttackMontageEndedDelegate.BindUObject(this, &AAFPlayerCharacter::EndAttack);
		Anim->Montage_SetEndDelegate(OnMeleeAttackMontageEndedDelegate, AttackMontage);
	}
	else
	{
		// 콤보 섹션 점프
		FName NextSectionName = *FString::Printf(TEXT("%s%02d"), *AttackAnimMontageSectionPrefix, ComboCount);
		Anim->Montage_JumpToSection(NextSectionName, AttackMontage);
	}
}

void AAFPlayerCharacter::HandleOnCheckInputAttack_FromNotify(UAnimInstance* Anim)
{
	if (!HasAuthority()) return;

	if (bIsAttackKeyPressed)
	{
		bIsAttackKeyPressed = false;
		CurrentComboCount = FMath::Clamp(CurrentComboCount + 1, 1, MaxComboCount);

		Multicast_PlayComboSection(CurrentComboCount);
	}
}

void AAFPlayerCharacter::TriggerHitReact_FromAttacker(AActor* Attacker)
{
	if (!HasAuthority()) return;
	if (bIsHit) return;

	const EAFHitDir Dir = CalcHitDir(Attacker);

	bIsHit = true;
	Multicast_PlayHitReact(Dir);
}

void AAFPlayerCharacter::HandleSkillHitCheck(float Radius, float Damage, float RotationOffset)
{
	if (!HasAuthority()) return;

	FVector Forward = GetActorForwardVector();
	FVector SkillDirection = Forward.RotateAngleAxis(RotationOffset, FVector(0, 0, 1));
	FVector SphereLocation = GetActorLocation() + (SkillDirection * 150.f) + FVector(0, 0, 60.f);
	
	TArray<FOverlapResult> OverlapResults;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(Radius);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	const bool bHit = GetWorld()->OverlapMultiByChannel(
		OverlapResults,
		SphereLocation,
		FQuat::Identity,
		ECC_Pawn,
		Sphere,
		Params
	);
	
	if (!bHit) return;

	for (const FOverlapResult& Result : OverlapResults)
	{
		AActor* HitActor = Result.GetActor();
		if (!HitActor) continue;
		if (HitActor == this) continue;
		if (IsAlly(HitActor)) continue;

		if (UAFAttributeComponent* Attr = HitActor->FindComponentByClass<UAFAttributeComponent>())
		{
			Attr->ApplyDamage(Damage, GetController());
		}

		if (AAFPlayerCharacter* Victim = Cast<AAFPlayerCharacter>(HitActor))
		{
			Victim->TriggerHitReact_FromAttacker(this);
		}

		// 슬로우 적용
		if (UAFStatusEffectComponent* StatusComp = HitActor->FindComponentByClass<UAFStatusEffectComponent>())
		{
			StatusComp->ApplySlow(0.2f, 0.5f);
		}
	}
}

bool AAFPlayerCharacter::IsAlly(AActor* InTargetActor)
{
	if (!InTargetActor) return false;

	APawn* TargetPawn = Cast<APawn>(InTargetActor);
	if (!TargetPawn) return false;

	AAFPlayerState* MyPS = Cast<AAFPlayerState>(GetPlayerState());
	AAFPlayerState* TargetPS = Cast<AAFPlayerState>(TargetPawn->GetPlayerState());

	if (MyPS && TargetPS)
	{
		return MyPS->GetTeamID() == TargetPS->GetTeamID();
	}

	return false;
}

void AAFPlayerCharacter::Multicast_PlayHitReact_Implementation(EAFHitDir Dir)
{
	TObjectPtr<UAnimMontage> Montage = nullptr;

	switch (Dir)
	{
	case EAFHitDir::Front: Montage = HitReactMontage_Front; break;
	case EAFHitDir::Back:  Montage = HitReactMontage_Back;  break;
	case EAFHitDir::Left:  Montage = HitReactMontage_Left;  break;
	case EAFHitDir::Right: Montage = HitReactMontage_Right; break;
	}

	if (!Montage) return;

	if (UAnimInstance* Anim = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
	{
		Anim->StopAllMontages(0.05f); // 맞으면 스킬/공격 끊기
	}

	bIsUsingSkill = false;
	bIsAttacking = false;
	bIsHeavyAttacking = false;

	UnlockMovement();

	PlayAnimMontage(Montage.Get(), 1.f);
}

EAFHitDir AAFPlayerCharacter::CalcHitDir(AActor* Attacker) const
{
	if (!Attacker) return EAFHitDir::Front;

	FVector Dir = Attacker->GetActorLocation() - GetActorLocation(); // 공격자 - 피해자
	Dir.Z = 0.f;

	if (Dir.IsNearlyZero()) return EAFHitDir::Front;
	Dir.Normalize();

	const float F = FVector::DotProduct(GetActorForwardVector(), Dir);
	const float R = FVector::DotProduct(GetActorRightVector(), Dir);

	if (FMath::Abs(F) >= FMath::Abs(R))
	{
		return (F >= 0.f) ? EAFHitDir::Front : EAFHitDir::Back;
	}
	return (R >= 0.f) ? EAFHitDir::Right : EAFHitDir::Left;
}

void AAFPlayerCharacter::ApplySpeedBuff(float Multiplier, float Duration)
{
	if (!HasAuthority() || !GetCharacterMovement()) return;

	// 기본 속도 기준 배수 적용
	GetCharacterMovement()->MaxWalkSpeed = DefaultMaxWalkSpeed * Multiplier;

	// 3. 복구 타이머
	GetWorldTimerManager().SetTimer(SpeedBuffTimerHandle, [this]()
		{
			if (GetCharacterMovement())
			{
				GetCharacterMovement()->MaxWalkSpeed = DefaultMaxWalkSpeed;
				UE_LOG(LogTemp, Log, TEXT("Speed Buff Expired!"));
			}
		}, Duration, false);
}


void AAFPlayerCharacter::InitializeCharacterData(FString CharacterName)
{
	// 캐릭터 이름이 비어있으면 실행하지 않음 (방어적 프로그래밍)
	if (CharacterName.IsEmpty()) return;

	static const FString ContextString(TEXT("Character Data Context"));

	// 1. 캐릭터 기본 스탯 로드
	if (StatDataTable)
	{
		FAFPlayerCharacterStatRow* FoundStat = StatDataTable->FindRow<FAFPlayerCharacterStatRow>(FName(*CharacterName), ContextString);
		if (FoundStat)
		{
			BaseStats = *FoundStat;

			// 이동 속도는 서버와 클라이언트 모두 동기화되어야 하므로 여기서 설정
			if (GetCharacterMovement())
			{
				GetCharacterMovement()->MaxWalkSpeed = BaseStats.MoveSpeed;
			}

			UE_LOG(LogTemp, Log, TEXT("[%s] 스탯 로드 성공! HP: %f"), *CharacterName, BaseStats.Maxhp);
		}
	}

	// 2. 캐릭터 스킬 5종 로드
	if (SkillDataTable)
	{
		TArray<FString> Suffixes = { TEXT("Left"), TEXT("Right"), TEXT("Passive"), TEXT("Q"), TEXT("E") };
		CharacterSkills.Empty();

		for (const FString& Suffix : Suffixes)
		{
			FName SkillRowName = FName(*(CharacterName + TEXT("_") + Suffix));
			FAFSkillInfo* FoundSkill = SkillDataTable->FindRow<FAFSkillInfo>(SkillRowName, ContextString);

			if (FoundSkill)
			{
				CharacterSkills.Add(*FoundSkill);
			}
			else
			{
				// 데이터 테이블에 키가 없을 경우 로그를 남겨 기획 실수를 방지
				UE_LOG(LogTemp, Warning, TEXT("[%s] 스킬 로드 실패! 데이터 테이블을 확인하세요."), *SkillRowName.ToString());
			}
		}
	}
}


void AAFPlayerCharacter::StartDeath(AController* LastInstigator)
{
	if (!HasAuthority()) return;

	// 1. 서버에서 게임모드에 알림 (점수/리스폰 처리)
	if (AAFGameMode* GM = GetWorld()->GetAuthGameMode<AAFGameMode>())
	{
		// LastInstigator는 나를 죽인 사람 (낙사면 nullptr)
		GM->HandlePlayerDeath(GetController(), LastInstigator);
	}

	// 2. 모든 클라이언트에게 사망 연출 명령
	Multicast_OnDeath();

}

void AAFPlayerCharacter::Multicast_OnDeath_Implementation()
{
	// 애니메이션 재생 (사망 몽타주)
	if (DeathMontage)
	{
		PlayAnimMontage(DeathMontage);
	}

	// 전투 중이던 플래그들 정리
	bIsAttacking = false;
	bIsUsingSkill = false;

	// 이동 및 콜리전 정지
	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
}

void AAFPlayerCharacter::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (HasAuthority() && OtherActor && OtherActor->ActorHasTag(TEXT("WaterFloor")))
	{
		// 낙사 (가해자 nullptr)
		StartDeath(nullptr);
	}
}





