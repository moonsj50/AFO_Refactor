#include "Game/AFLobbyGameState.h"
#include "Net/UnrealNetwork.h"

void AAFLobbyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAFLobbyGameState, ConnectedPlayers);
	DOREPLIFETIME(AAFLobbyGameState, RedCount);
	DOREPLIFETIME(AAFLobbyGameState, BlueCount);
}

void AAFLobbyGameState::OnRep_Counts()
{
	OnCountsChanged.Broadcast();
}