#include "UI/AFScoreboardWidget.h"
#include "UI/AFScoreboardRowWidget.h"
#include "Player/AFPlayerState.h"
#include "GameFramework/GameStateBase.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UAFScoreboardWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UE_LOG(LogTemp, Warning, TEXT("[Scoreboard] Construct Wrapper(Red=%s Blue=%s) Split(RT=%s RB=%s BT=%s BB=%s)"),
		SB_Red ? TEXT("OK") : TEXT("NULL"),
		SB_Blue ? TEXT("OK") : TEXT("NULL"),
		SB_RedTop ? TEXT("OK") : TEXT("NULL"),
		SB_RedBottom ? TEXT("OK") : TEXT("NULL"),
		SB_BlueTop ? TEXT("OK") : TEXT("NULL"),
		SB_BlueBottom ? TEXT("OK") : TEXT("NULL")
	);

	RefreshPlayerList();
	RefreshTotals();
	RefreshResultText();
	RefreshWinnerImages();
}

void UAFScoreboardWidget::SetMatchResult(const FAFMatchResult& InResult)
{
	CachedResult = InResult;
	RefreshResultText();
	RefreshTotals();
	RefreshWinnerImages();
}

void UAFScoreboardWidget::RefreshResultText()
{
	if (!TB_Result) return;

	FString ResultStr = TEXT("RESULT");
	switch (CachedResult.WinnerTeam)
	{
	case EAFTeamId::Red:  ResultStr = TEXT("RED WIN");  break;
	case EAFTeamId::Blue: ResultStr = TEXT("BLUE WIN"); break;
	default:              ResultStr = TEXT("DRAW");     break;
	}

	TB_Result->SetText(FText::FromString(ResultStr));
}

void UAFScoreboardWidget::RefreshWinnerImages()
{
	if (!Win_Image || !Red_Image || !Blue_Image) return;

	Win_Image->SetVisibility(ESlateVisibility::Hidden);
	Red_Image->SetVisibility(ESlateVisibility::Hidden);
	Blue_Image->SetVisibility(ESlateVisibility::Hidden);

	switch (CachedResult.WinnerTeam)
	{
	case EAFTeamId::Red:
		Win_Image->SetVisibility(ESlateVisibility::Visible);
		Red_Image->SetVisibility(ESlateVisibility::Visible);
		break;
	case EAFTeamId::Blue:
		Win_Image->SetVisibility(ESlateVisibility::Visible);
		Blue_Image->SetVisibility(ESlateVisibility::Visible);
		break;
	default:
		break;
	}
}

void UAFScoreboardWidget::RefreshTotals()
{
	if (!TB_RedTotalKills || !TB_BlueTotalKills) return;

	TB_RedTotalKills->SetText(FText::FromString(
		FString::Printf(TEXT("RED : %d"), CachedResult.RedKills)
	));

	TB_BlueTotalKills->SetText(FText::FromString(
		FString::Printf(TEXT("BLUE : %d"), CachedResult.BlueKills)
	));
}

void UAFScoreboardWidget::RefreshPlayerList()
{
	if (!SB_RedTop || !SB_RedBottom || !SB_BlueTop || !SB_BlueBottom)
	{
		UE_LOG(LogTemp, Error, TEXT("[Scoreboard] Split ScrollBoxes are NULL. (Check IsVariable / BindWidget)"));
		return;
	}

	SB_RedTop->ClearChildren();
	SB_RedBottom->ClearChildren();
	SB_BlueTop->ClearChildren();
	SB_BlueBottom->ClearChildren();

	AGameStateBase* GS = GetWorld() ? GetWorld()->GetGameState() : nullptr;
	if (!GS) return;

	if (!RowWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[Scoreboard] RowWidgetClass is NULL."));
		return;
	}

	TArray<AAFPlayerState*> RedPlayers;
	TArray<AAFPlayerState*> BluePlayers;

	for (APlayerState* PS : GS->PlayerArray)
	{
		AAFPlayerState* AFPS = Cast<AAFPlayerState>(PS);
		if (!AFPS) continue;

		if (AFPS->GetTeamID() == 0) RedPlayers.Add(AFPS);
		else                       BluePlayers.Add(AFPS);
	}

	auto SortByIndex = [](const AAFPlayerState& A, const AAFPlayerState& B)
		{
			return A.GetTeamIndex() < B.GetTeamIndex();
		};
	RedPlayers.Sort([&](const AAFPlayerState& A, const AAFPlayerState& B) { return SortByIndex(A, B); });
	BluePlayers.Sort([&](const AAFPlayerState& A, const AAFPlayerState& B) { return SortByIndex(A, B); });

	auto AddRow = [&](UScrollBox* Target, AAFPlayerState* PS)
		{
			if (!Target || !PS) return;

			UAFScoreboardRowWidget* Row = CreateWidget<UAFScoreboardRowWidget>(GetOwningPlayer(), RowWidgetClass);
			if (!Row) return;

			Row->Init(PS);
			Target->AddChild(Row);
		};

	for (int32 i = 0; i < RedPlayers.Num(); ++i)
	{
		AddRow((i == 0) ? SB_RedTop : SB_RedBottom, RedPlayers[i]);
	}
	for (int32 i = 0; i < BluePlayers.Num(); ++i)
	{
		AddRow((i == 0) ? SB_BlueTop : SB_BlueBottom, BluePlayers[i]);
	}
}
