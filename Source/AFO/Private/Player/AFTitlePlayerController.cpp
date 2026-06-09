#include "Player/AFTitlePlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "Game/AFGameInstance.h"

void AAFTitlePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController() == false)
	{
		return;
	}

	if (IsValid(UIWidgetClass) == true)
	{
		UIWidgetInstance = CreateWidget<UUserWidget>(this, UIWidgetClass);
		if (IsValid(UIWidgetInstance) == true)
		{
			UIWidgetInstance->AddToViewport();

			FInputModeUIOnly Mode;
			Mode.SetWidgetToFocus(UIWidgetInstance->GetCachedWidget());
			SetInputMode(Mode);

			bShowMouseCursor = true;
		}
	}

	if (IsValid(TitleBGM) && TitleBGMComponent == nullptr)
	{
		TitleBGMComponent = UGameplayStatics::SpawnSound2D(this, TitleBGM, 1.0f, 1.0f, 0.0f);
		if (TitleBGMComponent)
		{
			TitleBGMComponent->bIsUISound = true;
		}
	}
}

void AAFTitlePlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (TitleBGMComponent)
	{
		TitleBGMComponent->Stop();
		TitleBGMComponent = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void AAFTitlePlayerController::JoinServer(const FString& InIPAddress, const FString& InPlayerName)
{
	FString IP = InIPAddress.TrimStartAndEnd();
	FString Name = InPlayerName.TrimStartAndEnd();
	if (Name.IsEmpty())
	{
		Name = TEXT("DefaultPlayer");
	}

	if (UAFGameInstance* GI = GetGameInstance<UAFGameInstance>())
	{
		GI->PendingPlayerName = Name;
		GI->PendingServerIP = IP;
	}

	FInputModeGameOnly GameMode;
	SetInputMode(GameMode);
	bShowMouseCursor = false;

	if (IsValid(UIWidgetInstance))
	{
		UIWidgetInstance->RemoveFromParent();
		UIWidgetInstance = nullptr;
	}

	if (TitleBGMComponent)
	{
		TitleBGMComponent->Stop();
		TitleBGMComponent = nullptr;
	}

	if (IP.IsEmpty())
	{
		const FString MapURL = TEXT("/Game/01_ArenaFighter/01_Levels/AFOTeamSelect");
		UGameplayStatics::OpenLevel(GetWorld(), FName(*MapURL), true, TEXT("listen"));
		return;
	}

	const FString Address = FString::Printf(TEXT("%s:7777"), *IP);
	UE_LOG(LogTemp, Warning, TEXT("[JoinServer] Traveling to: %s (Name=%s)"), *Address, *Name);
	ClientTravel(Address, TRAVEL_Absolute);
}
//void AAFTitlePlayerController::JoinServer()
//{
//	FName NextLevelName = FName(TEXT("AFOBattleZone"));
//	UGameplayStatics::OpenLevel(GetWorld(), NextLevelName, true);
//
//	FInputModeGameOnly GameMode;
//	SetInputMode(GameMode);
//
//	bShowMouseCursor = false;
// }

//void AAFTitlePlayerController::JoinServer(const FString& InIPAddress)
//{
//	ClientTravel("127.0.0.1", TRAVEL_Absolute);
//}