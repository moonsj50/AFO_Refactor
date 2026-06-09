#include "UI/UW_CharacterSelect.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Kismet/GameplayStatics.h"
#include "Controller/AFLobbyPlayerController.h"
#include "GameFramework/GameState.h"
#include "Game/AFCharacterSelectGameMode.h"
#include "Player/AFPlayerState.h"
#include "TimerManager.h"

void UUW_CharacterSelect::NativeConstruct()
{
	Super::NativeConstruct();

	if (BtnChar0) BtnChar0->OnClicked.AddDynamic(this, &ThisClass::OnClickChar0);
	if (BtnChar1) BtnChar1->OnClicked.AddDynamic(this, &ThisClass::OnClickChar1);
	if (BtnChar2) BtnChar2->OnClicked.AddDynamic(this, &ThisClass::OnClickChar2);
	if (BtnChar3) BtnChar3->OnClicked.AddDynamic(this, &ThisClass::OnClickChar3);
	if (BtnChar4) BtnChar4->OnClicked.AddDynamic(this, &ThisClass::OnClickChar4);
	if (BtnReady) BtnReady->OnClicked.AddDynamic(this, &ThisClass::OnClickReady);


	if (AAFCharacterSelectGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<AAFCharacterSelectGameMode>() : nullptr)
	{
		const auto& Opt = GM->GetCharacterOptions();
		if (Opt.Num() >= 5)
		{
			if (TxtChar0) TxtChar0->SetText(Opt[0].DisplayName);
			if (TxtChar1) TxtChar1->SetText(Opt[1].DisplayName);
			if (TxtChar2) TxtChar2->SetText(Opt[2].DisplayName);
			if (TxtChar3) TxtChar3->SetText(Opt[3].DisplayName);
			if (TxtChar4) TxtChar4->SetText(Opt[4].DisplayName);
		}
	}

	if (APlayerController* PC = GetOwningPlayer())
	{
		if (AAFPlayerState* PS = PC->GetPlayerState<AAFPlayerState>())
		{
			if (!PS->OnTeamInfoChanged.IsAlreadyBound(this, &ThisClass::RefreshUI_FromDelegate))
			{
				PS->OnTeamInfoChanged.AddDynamic(this, &ThisClass::RefreshUI_FromDelegate);
			}
		}
		else
		{
			GetWorld()->GetTimerManager().SetTimer(
				RefreshTimerHandle,
				this,
				&ThisClass::RefreshUI,
				0.2f,
				true
			);
		}
	}

	RefreshUI();

	GetWorld()->GetTimerManager().SetTimer(RefreshTimerHandle, this, &ThisClass::RefreshUI, 0.5f, true);
}

void UUW_CharacterSelect::OnClickChar0() { SelectCharacter(0); }
void UUW_CharacterSelect::OnClickChar1() { SelectCharacter(1); }
void UUW_CharacterSelect::OnClickChar2() { SelectCharacter(2); }
void UUW_CharacterSelect::OnClickChar3() { SelectCharacter(3); }
void UUW_CharacterSelect::OnClickChar4() { SelectCharacter(4); }

void UUW_CharacterSelect::SelectCharacter(uint8 Id)
{
	if (AAFLobbyPlayerController* PC = GetOwningPlayer<AAFLobbyPlayerController>())
	{
		PC->ServerRequestSelectCharacter(Id);
	}
}

void UUW_CharacterSelect::OnClickReady()
{
	AAFPlayerState* MyPS = nullptr;
	if (APlayerController* PC = GetOwningPlayer())
	{
		MyPS = PC->GetPlayerState<AAFPlayerState>();
	}

	const bool bNewReady = (MyPS) ? !MyPS->IsReady() : true;

	if (AAFLobbyPlayerController* LPC = GetOwningPlayer<AAFLobbyPlayerController>())
	{
		LPC->ServerRequestSetReady(bNewReady);
	}
}

void UUW_CharacterSelect::RefreshUI()
{
	AAFPlayerState* MyPS = nullptr;
	if (APlayerController* PC = GetOwningPlayer())
	{
		MyPS = PC->GetPlayerState<AAFPlayerState>();
	}

	if (TxtReady)
	{
		const FString S = (MyPS && MyPS->IsReady()) ? TEXT("READY: ON") : TEXT("READY: OFF");
		TxtReady->SetText(FText::FromString(S));
	}

	RebuildPlayerList();
}

void UUW_CharacterSelect::RefreshUI_FromDelegate(AAFPlayerState* ChangedPS)
{
	RefreshUI();
}

void UUW_CharacterSelect::RebuildPlayerList()
{
	if (!VBPlayerList) return;

	VBPlayerList->ClearChildren();

	AGameStateBase* GS = GetWorld() ? GetWorld()->GetGameState() : nullptr;
	if (!GS) return;

	for (APlayerState* PS : GS->PlayerArray)
	{
		AAFPlayerState* AFPS = Cast<AAFPlayerState>(PS);
		if (!AFPS) continue;

		const TCHAR* TeamStr = (AFPS->GetTeamID() == 0) ? TEXT("RED") : TEXT("BLUE");
		const FString PickStr = AFPS->HasSelectedCharacter() ? FString::FromInt((int32)AFPS->GetSelectedCharacterId()) : TEXT("None");
		const TCHAR* ReadyStr = AFPS->IsReady() ? TEXT("READY") : TEXT("NOT READY");

		const FString Line = FString::Printf(TEXT("%s | %s | Pick:%s | %s"),
			*AFPS->GetPlayerName(), TeamStr, *PickStr, ReadyStr);

		UTextBlock* NewText = NewObject<UTextBlock>(this);
		NewText->SetText(FText::FromString(Line));
		VBPlayerList->AddChild(NewText);
	}
}
