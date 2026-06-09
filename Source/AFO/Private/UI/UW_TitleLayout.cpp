#include "UI/UW_TitleLayout.h"
#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "Player/AFTitlePlayerController.h"

UUW_TitleLayout::UUW_TitleLayout(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UUW_TitleLayout::NativeConstruct()
{
	Super::NativeConstruct();

	if (PlayButton)
	{
		PlayButton->OnClicked.AddDynamic(this, &ThisClass::OnPlayButtonClicked);
	}

	if (ExitButton)
	{
		ExitButton->OnClicked.AddDynamic(this, &ThisClass::OnExitButtonClicked);
	}

	if (OptionButton)
	{
		OptionButton->OnClicked.AddDynamic(this, &ThisClass::OnOptionButtonClicked);
	}
}

//void UUW_TitleLayout::OnPlayButtonClicked()
//{
//	AAFTitlePlayerController* PlayerController = GetOwningPlayer<AAFTitlePlayerController>();
//	if (IsValid(PlayerController) == true)
//	{
//		// FText ServerIP = ServerIPEditableText->GetText();
//		PlayerController->JoinServer();
//	}
//}

void UUW_TitleLayout::OnPlayButtonClicked()
{
	AAFTitlePlayerController* PC = GetOwningPlayer<AAFTitlePlayerController>();
	if (!IsValid(PC)) return;

	FString IP;
	if (ServerIPEditableText)
	{
		IP = ServerIPEditableText->GetText().ToString().TrimStartAndEnd();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ServerIPEditableText is null. Check BindWidget name."));
	}

	// ★ 이름 가져오기
	FString PlayerName = Text_PlayerName ? Text_PlayerName->GetText().ToString().TrimStartAndEnd() : TEXT("DefaultPlayer");

	PC->JoinServer(IP, PlayerName);

}


void UUW_TitleLayout::OnExitButtonClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}

void UUW_TitleLayout::OnOptionButtonClicked()
{
	if (OptionsWidget && OptionsWidget->IsInViewport())
	{
		HideOptions();
	}
	else
	{
		ShowOptions();
	}
}

void UUW_TitleLayout::ShowOptions()
{
	APlayerController* PC = GetOwningPlayer();
	if (!IsValid(PC)) return;

	if (!OptionsWidget)
	{
		if (!OptionsWidgetClass)
		{
			UE_LOG(LogTemp, Error, TEXT("[Options] OptionsWidgetClass is NULL. Set it in WBP_TitleLayout."));
			return;
		}

		OptionsWidget = CreateWidget<UUserWidget>(PC, OptionsWidgetClass);
		if (!OptionsWidget)
		{
			UE_LOG(LogTemp, Error, TEXT("[Options] CreateWidget failed."));
			return;
		}
	}

	OptionsWidget->AddToViewport(100);

	FInputModeGameAndUI Mode;
	Mode.SetHideCursorDuringCapture(false);
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	Mode.SetWidgetToFocus(OptionsWidget->TakeWidget());
	PC->SetInputMode(Mode);
	PC->bShowMouseCursor = true;
}

void UUW_TitleLayout::HideOptions()
{
	APlayerController* PC = GetOwningPlayer();
	if (!IsValid(PC)) return;

	if (OptionsWidget)
	{
		OptionsWidget->RemoveFromParent();
	}

	FInputModeGameAndUI Mode;
	Mode.SetHideCursorDuringCapture(false);
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	Mode.SetWidgetToFocus(this->TakeWidget());
	PC->SetInputMode(Mode);
	PC->bShowMouseCursor = true;
}