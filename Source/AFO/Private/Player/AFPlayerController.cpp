#include "Player/AFPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "UI/AFESCWidget.h"
#include "UI/AFInGameWidget.h"
#include "UI/AFRespawnWidget.h"
#include "UI/AFKillLogContainer.h"
#include "UI/AFScoreboardWidget.h"
#include "Game/AFGameState.h"
#include "Player/AFPlayerState.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"
#include "EnhancedInputComponent.h"
#include "UI/AFSkillMainWidget.h"

AAFPlayerController::AAFPlayerController()
	: InputMappingContext(nullptr)
	, MoveAction(nullptr)
	, JumpAction(nullptr)
	, LookAction(nullptr)
	, SprintAction(nullptr)
	, AttackAction(nullptr)
	, ESC(nullptr)
	, SkillEAction(nullptr)
	, SkillQAction(nullptr)
	, HeavyAttackAction(nullptr)
	, ScoreboardWidgetClass(nullptr)
	, ScoreboardWidget(nullptr)
	, SkillMainWidgetClass(nullptr)
	, SkillMainWidget(nullptr)
	, InGameWidgetClass(nullptr)
	, ESCWidgetClass(nullptr)
	, InGameWidget(nullptr)
	, ESCWidget(nullptr)
	, RespawnWidgetClass(nullptr)
	, CurrentRespawnWidget(nullptr)
	, KillLogContainerClass(nullptr)
	, KillLogContainer(nullptr)
{
}

void AAFPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (InputMappingContext)
			{
				Subsystem->AddMappingContext(InputMappingContext, 0);
			}
		}
	}

	if (!IsLocalController())
	{
		return;
	}

	// Skill HUD
	if (IsLocalController() && SkillMainWidgetClass)
	{
		// 형변환을 통해 생성하여 클래스 전용 함수들을 사용할 수 있게 합니다.
		SkillMainWidget = CreateWidget<UAFSkillMainWidget>(this, SkillMainWidgetClass);
		if (SkillMainWidget)
		{
			SkillMainWidget->AddToViewport();
			UE_LOG(LogTemp, Warning, TEXT("@@@ [PC] SkillMain Initialized!"));
		}
	}

	// InGame HUD
	if (IsValid(InGameWidgetClass))
	{
		InGameWidget = CreateWidget<UAFInGameWidget>(this, InGameWidgetClass);
		if (IsValid(InGameWidget))
		{
			InGameWidget->AddToViewport();
		}
	}

	// ESC Menu
	if (IsValid(ESCWidgetClass))
	{
		ESCWidget = CreateWidget<UAFESCWidget>(this, ESCWidgetClass);
		if (IsValid(ESCWidget))
		{
			ESCWidget->AddToViewport(999);
			ESCWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	// KillLog Container
	if (KillLogContainerClass)
	{
		KillLogContainer = CreateWidget<UAFKillLogContainer>(this, KillLogContainerClass);
		if (KillLogContainer)
		{
			KillLogContainer->AddToViewport();
		}
	}

	// ★ 여기서 GameState 델리게이트 바인딩
	TryBindGameState();
}

void AAFPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (ESC)
		{
			EnhancedInputComponent->BindAction(ESC, ETriggerEvent::Started, this, &AAFPlayerController::ToggleESCMenu);
		}
	}
}

void AAFPlayerController::ToggleESCMenu()
{
	if (!IsValid(ESCWidget))
	{
		return;
	}

	bIsESCMenuOpen = !bIsESCMenuOpen;

	if (bIsESCMenuOpen)
	{
		ESCWidget->SetVisibility(ESlateVisibility::Visible);
		SetInputMode(FInputModeGameAndUI());
		bShowMouseCursor = true;
	}
	else
	{
		ESCWidget->SetVisibility(ESlateVisibility::Collapsed);
		SetInputMode(FInputModeGameOnly());
		bShowMouseCursor = false;
	}
}

void AAFPlayerController::Client_ShowRespawnWidget_Implementation(float Duration)
{
	if (RespawnWidgetClass)
	{
		CurrentRespawnWidget = CreateWidget<UAFRespawnWidget>(this, RespawnWidgetClass);
		if (CurrentRespawnWidget)
		{
			CurrentRespawnWidget->InitRespawnTimer(Duration);
			CurrentRespawnWidget->AddToViewport();
		}
	}
}

void AAFPlayerController::Client_ClearRespawnWidget_Implementation()
{
	if (CurrentRespawnWidget)
	{
		CurrentRespawnWidget->RemoveFromParent();
		CurrentRespawnWidget = nullptr;
	}
}

void AAFPlayerController::Client_ShowKillLog_Implementation(const FString& KillerName, FLinearColor KillerColor, const FString& VictimName, FLinearColor VictimColor)
{
	if (KillLogContainer)
	{
		KillLogContainer->AddKillLog(KillerName, KillerColor, VictimName, VictimColor);
	}
}

void AAFPlayerController::TryBindGameState()
{
	if (!IsLocalController()) return;

	AAFGameState* GS = GetWorld() ? GetWorld()->GetGameState<AAFGameState>() : nullptr;
	if (!GS)
	{
		FTimerHandle Temp;
		GetWorldTimerManager().SetTimer(Temp, this, &AAFPlayerController::TryBindGameState, 0.2f, false);
		return;
	}

	GS->OnMatchResultChanged.RemoveDynamic(this, &AAFPlayerController::OnMatchResultChanged);
	GS->OnMatchResultChanged.AddDynamic(this, &AAFPlayerController::OnMatchResultChanged);

	GS->OnPlayerArrayChanged.RemoveDynamic(this, &AAFPlayerController::OnPlayerArrayChanged);
	GS->OnPlayerArrayChanged.AddDynamic(this, &AAFPlayerController::OnPlayerArrayChanged);
	GS->OnGamePhaseChanged.RemoveDynamic(this, &AAFPlayerController::OnGamePhaseChanged);
	GS->OnGamePhaseChanged.AddDynamic(this, &AAFPlayerController::OnGamePhaseChanged);

	OnGamePhaseChanged(GS->GetCurrentGamePhase());
}

void AAFPlayerController::OnPlayerArrayChanged()
{
	if (!IsLocalController()) return;

	if (ScoreboardWidget)
	{
		if (AAFGameState* GS = GetWorld() ? GetWorld()->GetGameState<AAFGameState>() : nullptr)
		{
			ScoreboardWidget->SetMatchResult(GS->MatchResult);

			ScoreboardWidget->RefreshPlayerList();

			ScoreboardWidget->RefreshTotals();
		}
	}
}

void AAFPlayerController::OnMatchResultChanged(const FAFMatchResult& NewResult)
{
	if (!IsLocalController()) return;

	if (!ScoreboardWidget && ScoreboardWidgetClass)
	{
		ScoreboardWidget = CreateWidget<UAFScoreboardWidget>(this, ScoreboardWidgetClass);
		if (ScoreboardWidget)
		{
			ScoreboardWidget->AddToViewport(2000);
		}
	}

	if (ScoreboardWidget)
	{
		ScoreboardWidget->SetMatchResult(NewResult);

		ScoreboardWidget->RefreshPlayerList();

		ScoreboardWidget->RefreshTotals();
	}

	SetInputMode(FInputModeUIOnly());
	bShowMouseCursor = true;

	ShowScoreboardIfNeeded();
}


void AAFPlayerController::RefreshSkillUI(UAFSkillComponent* InSkillComp)
{
	// SkillMainWidget2가 아닌, 위에서 생성한 SkillMainWidget을 사용해야 합니다.
	if (SkillMainWidget && InSkillComp)
	{
		SkillMainWidget->UpdateAllSlotsComponent(InSkillComp);
		UE_LOG(LogTemp, Warning, TEXT("@@@ [PC] Skill UI Refreshed with New Component!"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("@@@ [PC] Refresh Failed: Widget(%s), Comp(%s)"),
			SkillMainWidget ? TEXT("Valid") : TEXT("NULL"),
			InSkillComp ? TEXT("Valid") : TEXT("NULL"));
	}
}


void AAFPlayerController::OnGamePhaseChanged(EAFGamePhase NewPhase)
{
	if (!IsLocalController()) return;
	if (NewPhase != EAFGamePhase::EAF_GameOver) return;

	if (!ScoreboardWidget && ScoreboardWidgetClass)
	{
		ScoreboardWidget = CreateWidget<UAFScoreboardWidget>(this, ScoreboardWidgetClass);
		if (ScoreboardWidget)
		{
			ScoreboardWidget->AddToViewport(2000);
		}
	}

	if (ScoreboardWidget)
	{
		if (AAFGameState* GS = GetWorld() ? GetWorld()->GetGameState<AAFGameState>() : nullptr)
		{
			ScoreboardWidget->SetMatchResult(GS->MatchResult);
		}

		ScoreboardWidget->RefreshPlayerList();
		ScoreboardWidget->RefreshTotals();

		FTimerHandle Tmp;
		GetWorldTimerManager().SetTimer(Tmp, [this]()
			{
				if (ScoreboardWidget)
				{
					ScoreboardWidget->RefreshPlayerList();
					ScoreboardWidget->RefreshTotals();
				}
			}, 0.2f, false);
	}

	SetInputMode(FInputModeUIOnly());
	bShowMouseCursor = true;
}


void AAFPlayerController::ShowScoreboardIfNeeded()
{
	if (!IsLocalController()) return;
	if (bScoreboardShown) return;

	if (!ScoreboardWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[PC] ScoreboardWidgetClass is NULL. Set it to WBP_Scoreboard in BP defaults."));
		return;
	}

	if (!ScoreboardWidget)
	{
		ScoreboardWidget = CreateWidget<UAFScoreboardWidget>(this, ScoreboardWidgetClass);
		if (ScoreboardWidget)
		{
			ScoreboardWidget->AddToViewport(5000);
			ScoreboardWidget->SetVisibility(ESlateVisibility::Visible);
		}
	}

	if (ScoreboardWidget)
	{
		if (AAFGameState* GS = GetWorld() ? GetWorld()->GetGameState<AAFGameState>() : nullptr)
		{
			ScoreboardWidget->SetMatchResult(GS->MatchResult);
		}

		SetInputMode(FInputModeUIOnly());
		bShowMouseCursor = true;

		bScoreboardShown = true;
	}
}

void AAFPlayerController::OnTimerChanged(int32 NewRemainingTime)
{
	if (!IsLocalController()) return;

	if (NewRemainingTime <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PC] RemainingTime hit 0 -> showing scoreboard"));
		ShowScoreboardIfNeeded();
	}
}
