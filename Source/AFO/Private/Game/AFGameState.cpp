#include "Game/AFGameState.h"
#include "Net/UnrealNetwork.h"
#include "Game/AFGameMode.h"
#include "Player/AFPlayerState.h"

AAFGameState::AAFGameState()
{
	RemainingTimeSeconds = 300;
}

void AAFGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, TeamRedKillScore);
	DOREPLIFETIME(ThisClass, TeamBlueKillScore);
	DOREPLIFETIME(ThisClass, TeamRedDeathScore);
	DOREPLIFETIME(ThisClass, TeamBlueDeathScore);

	DOREPLIFETIME(ThisClass, RemainingTimeSeconds);
	DOREPLIFETIME(ThisClass, CurrentGamePhase);

	DOREPLIFETIME(ThisClass, TeamPlayerStatesReplicated);
	DOREPLIFETIME(ThisClass, MatchResult);
}

void AAFGameState::SetRemainingTime(int32 NewTime)
{
	if (!HasAuthority()) return;

	RemainingTimeSeconds = NewTime;
	OnTimerChanged.Broadcast(RemainingTimeSeconds);
	ForceNetUpdate();
}

void AAFGameState::SetGamePhase(EAFGamePhase NewPhase)
{
	if (!HasAuthority()) return;

	CurrentGamePhase = NewPhase;
	OnGamePhaseChanged.Broadcast(CurrentGamePhase);
	ForceNetUpdate();
}

void AAFGameState::StartGameTimer()
{
	if (!HasAuthority()) return;

	GetWorldTimerManager().SetTimer(
		GameTimerHandle,
		this,
		&AAFGameState::UpdateTimer,
		1.0f,
		true
	);
}

void AAFGameState::UpdateTimer()
{
	if (!HasAuthority()) return;

	if (RemainingTimeSeconds > 0)
	{
		--RemainingTimeSeconds;
		OnTimerChanged.Broadcast(RemainingTimeSeconds);
		ForceNetUpdate();
		return;
	}

	GetWorldTimerManager().ClearTimer(GameTimerHandle);

	if (AAFGameMode* GM = GetWorld()->GetAuthGameMode<AAFGameMode>())
	{
		GM->EndRound();
	}
}

void AAFGameState::OnRep_RemainingTime()
{
	if (HasAuthority()) return;
	OnTimerChanged.Broadcast(RemainingTimeSeconds);
}

void AAFGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);

	if (!HasAuthority()) return;

	if (AAFPlayerState* AFPS = Cast<AAFPlayerState>(PlayerState))
	{
		TeamPlayerStatesReplicated.Add(AFPS);
	}

	OnPlayerArrayChanged.Broadcast();
	ForceNetUpdate();
}

void AAFGameState::OnRep_TeamPlayerArray()
{
	if (HasAuthority()) return;
	OnPlayerArrayChanged.Broadcast();
}

void AAFGameState::AddTeamScore(uint8 TeamID, bool bIsKill)
{
	if (!HasAuthority()) return;

	if (TeamID == 0)
	{
		if (bIsKill) ++TeamRedKillScore;
		else         ++TeamRedDeathScore;
	}
	else
	{
		if (bIsKill) ++TeamBlueKillScore;
		else         ++TeamBlueDeathScore;
	}

	OnPlayerArrayChanged.Broadcast();
	ForceNetUpdate();
}

void AAFGameState::OnRep_TeamScore()
{
	if (HasAuthority()) return;
	OnPlayerArrayChanged.Broadcast();
}

void AAFGameState::SetMatchResult(EAFTeamId WinnerTeam)
{
	if (!HasAuthority()) return;

	MatchResult.WinnerTeam = WinnerTeam;
	MatchResult.RedKills = TeamRedKillScore;
	MatchResult.BlueKills = TeamBlueKillScore;

	OnMatchResultChanged.Broadcast(MatchResult);
	ForceNetUpdate();
}

void AAFGameState::OnRep_GamePhase()
{
	if (HasAuthority()) return;
	OnGamePhaseChanged.Broadcast(CurrentGamePhase);
}

void AAFGameState::OnRep_MatchResult()
{
	if (HasAuthority()) return;
	OnMatchResultChanged.Broadcast(MatchResult);
}
