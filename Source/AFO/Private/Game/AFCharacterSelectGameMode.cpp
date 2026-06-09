#include "Game/AFCharacterSelectGameMode.h"
#include "Game/AFLobbyGameState.h"
#include "Player/AFPlayerState.h"
#include "Engine/World.h"
#include "GameFramework/GameState.h"
#include "Controller/AFLobbyPlayerController.h"

AAFCharacterSelectGameMode::AAFCharacterSelectGameMode()
{
	bUseSeamlessTravel = true;
	bStartPlayersAsSpectators = true;

	GameStateClass = AAFLobbyGameState::StaticClass();
	PlayerStateClass = AAFPlayerState::StaticClass();
	PlayerControllerClass = AAFLobbyPlayerController::StaticClass();

	CharacterOptions =
	{
		{0, FText::FromString(TEXT("Character A"))},
		{1, FText::FromString(TEXT("Character B"))},
		{2, FText::FromString(TEXT("Character C"))},
		{3, FText::FromString(TEXT("Character D"))},
		{4, FText::FromString(TEXT("Character E"))},
	};
}

bool AAFCharacterSelectGameMode::IsCharacterTakenInTeam(uint8 TeamId, uint8 CharacterId, APlayerState* Except) const
{
	if (!GameState) return false;

	for (APlayerState* PS : GameState->PlayerArray)
	{
		if (PS == Except) continue;

		if (AAFPlayerState* AFPS = Cast<AAFPlayerState>(PS))
		{
			if (AFPS->GetTeamID() == TeamId &&
				AFPS->HasSelectedCharacter() &&
				AFPS->GetSelectedCharacterId() == CharacterId)
			{
				return true;
			}
		}
	}
	return false;
}

bool AAFCharacterSelectGameMode::RequestSelectCharacter(AController* Requester, uint8 CharacterId)
{
	if (!HasAuthority() || !Requester) return false;

	const FAFCharacterOption* Found =
		CharacterOptions.FindByPredicate([&](const FAFCharacterOption& O)
			{
				return O.Id == CharacterId;
			});

	if (!Found) return false;

	AAFPlayerState* PS = Requester->GetPlayerState<AAFPlayerState>();
	if (!PS) return false;

	if (IsCharacterTakenInTeam(PS->GetTeamID(), CharacterId, PS))
		return false;

	PS->SetSelectedCharacter_Server(CharacterId);
	PS->SetSelectedCharacterName_Server(Found->DisplayName);
	PS->SetReady_Server(false);

	PS->ForceNetUpdate();
	TryStartBattle();
	return true;
}

bool AAFCharacterSelectGameMode::RequestSetReady(AController* Requester, bool bNewReady)
{
	if (!HasAuthority() || !Requester) return false;

	AAFPlayerState* PS = Requester->GetPlayerState<AAFPlayerState>();
	if (!PS) return false;

	if (!PS->HasSelectedCharacter())
	{
		return false;
	}

	PS->SetReady_Server(bNewReady);
	PS->ForceNetUpdate();

	TryStartBattle();
	return true;
}

bool AAFCharacterSelectGameMode::IsAllPickedAndReady() const
{
	if (!GameState) return false;

	const int32 Total = GameState->PlayerArray.Num();
	if (Total < 2) return false;

	for (APlayerState* PS : GameState->PlayerArray)
	{
		AAFPlayerState* AFPS = Cast<AAFPlayerState>(PS);
		if (!AFPS) return false;

		if (!AFPS->HasSelectedCharacter()) return false;
		if (!AFPS->IsReady()) return false;
	}
	return true;
}

void AAFCharacterSelectGameMode::TryStartBattle()
{
	if (!IsAllPickedAndReady()) return;

	GetWorld()->ServerTravel(BattleZoneURL, true);
}
