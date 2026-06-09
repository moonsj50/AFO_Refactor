#include "Controller/AFLobbyPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "Game/AFTeamSelectGameMode.h"
#include "Game/AFCharacterSelectGameMode.h"
#include "UObject/UObjectGlobals.h"
#include "TimerManager.h"
#include "Game/AFGameInstance.h"
#include "Player/AFPlayerState.h"

void AAFLobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();
	if (!IsLocalController()) return;

	PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(
		this, &ThisClass::HandlePostLoadMap
	);

	EnsureUI();
}

void AAFLobbyPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (PostLoadMapHandle.IsValid())
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);
		PostLoadMapHandle.Reset();
	}

	if (GetWorld())
	{
		GetWorldTimerManager().ClearTimer(EnsureUITimer);
	}

	Super::EndPlay(EndPlayReason);
}

void AAFLobbyPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	if (!IsLocalController()) return;

	UE_LOG(LogTemp, Warning, TEXT("[UI] ReceivedPlayer -> EnsureUI"));
	EnsureUI();

	if (UAFGameInstance* GI = GetGameInstance<UAFGameInstance>())
	{
		const FString Name = GI->PendingPlayerName.TrimStartAndEnd();
		if (!Name.IsEmpty())
		{
			ServerSetPlayerName(Name);
		}
	}
}

void AAFLobbyPlayerController::ServerSetPlayerName_Implementation(const FString& InName)
{
	if (AAFPlayerState* PS = GetPlayerState<AAFPlayerState>())
	{
		PS->SetPlayerName(InName);
		PS->ForceNetUpdate();

		UE_LOG(LogTemp, Warning, TEXT("[ServerSetPlayerName] %s"), *InName);
	}
}

void AAFLobbyPlayerController::PostSeamlessTravel()
{
	Super::PostSeamlessTravel();
	if (!IsLocalController()) return;

	UE_LOG(LogTemp, Warning, TEXT("[UI] PostSeamlessTravel -> EnsureUI"));
	EnsureUI();
}

void AAFLobbyPlayerController::BeginPlayingState()
{
	Super::BeginPlayingState();
	if (!IsLocalController()) return;

	UE_LOG(LogTemp, Warning, TEXT("[UI] BeginPlayingState -> EnsureUI"));
	EnsureUI();
}

void AAFLobbyPlayerController::HandlePostLoadMap(UWorld* LoadedWorld)
{
	if (!IsLocalController()) return;
	if (!LoadedWorld || LoadedWorld != GetWorld()) return;

	UE_LOG(LogTemp, Warning, TEXT("[UI] PostLoadMapWithWorld -> EnsureUI"));
	EnsureUI();
}

void AAFLobbyPlayerController::EnsureUI()
{
	if (!GetWorld()) return;

	GetWorldTimerManager().SetTimerForNextTick([this]()
		{
			if (IsValid(this))
			{
				SetupUIForCurrentMap();
			}
		});

	GetWorldTimerManager().SetTimer(
		EnsureUITimer,
		this,
		&ThisClass::SetupUIForCurrentMap,
		0.2f,
		true
	);
}

void AAFLobbyPlayerController::ClearCurrentUI()
{
	if (CurrentWidget)
	{
		CurrentWidget->RemoveFromParent();
		CurrentWidget = nullptr;
	}
}

void AAFLobbyPlayerController::SetUIInputMode(bool bUIOnly)
{
	if (bUIOnly)
	{
		FInputModeGameAndUI Mode;
		Mode.SetHideCursorDuringCapture(false);
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(Mode);
		bShowMouseCursor = true;
	}
	else
	{
		FInputModeGameOnly Mode;
		SetInputMode(Mode);
		bShowMouseCursor = false;
	}
}

void AAFLobbyPlayerController::SetupUIForCurrentMap()
{
	const FString MapName = UGameplayStatics::GetCurrentLevelName(this, true);

	UE_LOG(LogTemp, Warning, TEXT("[UI] SetupUI Map=%s Local=%d PC=%s TeamWBP=%s CharWBP=%s Cur=%s"),
		*MapName,
		IsLocalController() ? 1 : 0,
		*GetClass()->GetName(),
		TeamSelectWidgetClass ? TEXT("OK") : TEXT("NULL"),
		CharacterSelectWidgetClass ? TEXT("OK") : TEXT("NULL"),
		CurrentWidget ? TEXT("EXIST") : TEXT("NONE"));

	TSubclassOf<UUserWidget> DesiredClass = nullptr;

	if (MapName == TEXT("AFOTeamSelect"))
	{
		DesiredClass = TeamSelectWidgetClass;
	}
	else if (MapName == TEXT("AFOCharacterSelectMap"))
	{
		DesiredClass = CharacterSelectWidgetClass;
	}

	if (!DesiredClass)
	{
		ClearCurrentUI();
		SetUIInputMode(false);
		GetWorldTimerManager().ClearTimer(EnsureUITimer);
		return;
	}

	if (CurrentWidget && CurrentWidget->IsA(DesiredClass))
	{
		SetUIInputMode(true);
		GetWorldTimerManager().ClearTimer(EnsureUITimer);
		return;
	}

	ClearCurrentUI();

	CurrentWidget = CreateWidget<UUserWidget>(this, DesiredClass);
	if (!CurrentWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("[UI] CreateWidget FAILED Map=%s Class=%s"),
			*MapName, *DesiredClass->GetName());
		return;
	}

	CurrentWidget->AddToViewport();
	SetUIInputMode(true);

	GetWorldTimerManager().ClearTimer(EnsureUITimer);

	UE_LOG(LogTemp, Warning, TEXT("[UI] Widget Added: %s"), *DesiredClass->GetName());
}

void AAFLobbyPlayerController::ServerRequestSetTeam_Implementation(uint8 NewTeamId)
{
	if (!GetWorld()) return;

	AGameModeBase* GMBase = GetWorld()->GetAuthGameMode();
	if (!GMBase) return;

	AAFTeamSelectGameMode* GM = Cast<AAFTeamSelectGameMode>(GMBase);
	if (!GM) return;

	if (!GM->RequestSetTeam(this, NewTeamId))
	{
		ClientShowMessage(TEXT("팀당 최대 2명만 가능합니다."));
	}
}

void AAFLobbyPlayerController::ServerRequestAdvanceToCharacterSelect_Implementation()
{
	if (AAFTeamSelectGameMode* GM = GetWorld()->GetAuthGameMode<AAFTeamSelectGameMode>())
	{
		if (!GM->AdvanceToCharacterSelect())
		{
			ClientShowMessage(TEXT("조건 미달: 2명 접속 + RED1/BLUE1가 되어야 합니다."));
		}
	}
}

void AAFLobbyPlayerController::ServerRequestSelectCharacter_Implementation(uint8 CharacterId)
{
	if (AAFCharacterSelectGameMode* GM = GetWorld()->GetAuthGameMode<AAFCharacterSelectGameMode>())
	{
		if (!GM->RequestSelectCharacter(this, CharacterId))
		{
			ClientShowMessage(TEXT("같은 팀 내 캐릭터 중복은 불가합니다."));
		}
	}
}

void AAFLobbyPlayerController::ServerRequestSetReady_Implementation(bool bNewReady)
{
	if (AAFCharacterSelectGameMode* GM = GetWorld()->GetAuthGameMode<AAFCharacterSelectGameMode>())
	{
		if (!GM->RequestSetReady(this, bNewReady))
		{
			ClientShowMessage(TEXT("캐릭터 선택 후에만 Ready 할 수 있습니다."));
		}
	}
}

void AAFLobbyPlayerController::ClientShowMessage_Implementation(const FString& Msg)
{
	ClientMessage(Msg);
}
