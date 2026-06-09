#include "UI/AFScoreboardRowWidget.h"
#include "Player/AFPlayerState.h"
#include "Components/TextBlock.h"

void UAFScoreboardRowWidget::Init(AAFPlayerState* InPS)
{
	PlayerStateRef = InPS;
	if (!PlayerStateRef) return;

	PlayerStateRef->OnPlayerNameChanged.RemoveAll(this);
	PlayerStateRef->OnPlayerNameChanged.AddUniqueDynamic(this, &ThisClass::OnNameChanged);

	PlayerStateRef->OnTeamInfoChanged.RemoveAll(this);
	PlayerStateRef->OnTeamInfoChanged.AddUniqueDynamic(this, &ThisClass::OnTeamChanged);

	PlayerStateRef->OnSelectedCharacterChanged.RemoveAll(this);
	PlayerStateRef->OnSelectedCharacterChanged.AddUniqueDynamic(this, &ThisClass::OnCharacterChanged);

	RefreshTexts();
}

void UAFScoreboardRowWidget::NativeDestruct()
{
	if (PlayerStateRef)
	{
		PlayerStateRef->OnPlayerNameChanged.RemoveAll(this);
		PlayerStateRef->OnTeamInfoChanged.RemoveAll(this);
		PlayerStateRef->OnSelectedCharacterChanged.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UAFScoreboardRowWidget::RefreshTexts()
{
	if (!PlayerStateRef) return;

	const FString PlayerName = PlayerStateRef->GetPlayerName();
	const FString CharacterName = PlayerStateRef->GetSelectedCharacterName().ToString();
	const int32 Kills = PlayerStateRef->GetKillCount();
	const int32 Deaths = PlayerStateRef->GetDeathCount();

	if (Text_PlayerName)
		Text_PlayerName->SetText(FText::FromString(FString::Printf(TEXT("Player : %s"), *PlayerName)));

	if (Text_CharacterName)
		Text_CharacterName->SetText(FText::FromString(FString::Printf(TEXT("Character : %s"), *CharacterName)));

	if (Text_Kills)
		Text_Kills->SetText(FText::FromString(FString::Printf(TEXT("Kill : %d"), Kills)));

	if (Text_Deaths)
		Text_Deaths->SetText(FText::FromString(FString::Printf(TEXT("Death : %d"), Deaths)));
}

void UAFScoreboardRowWidget::OnNameChanged(AAFPlayerState* ChangedPS)
{
	if (ChangedPS == PlayerStateRef)
	{
		RefreshTexts();
	}
}

void UAFScoreboardRowWidget::OnTeamChanged(AAFPlayerState* ChangedPS)
{
	if (ChangedPS == PlayerStateRef)
	{
		RefreshTexts();
	}
}

void UAFScoreboardRowWidget::OnCharacterChanged(AAFPlayerState* ChangedPS)
{
	if (ChangedPS == PlayerStateRef)
	{
		RefreshTexts();
	}
}
