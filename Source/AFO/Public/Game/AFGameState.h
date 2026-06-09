// AFGameState.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "AFGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimerChangedDelegate, int32, NewRemainingTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPlayerArrayChangedDelegate);

// GamePhase (너 기존 그대로)
UENUM(BlueprintType)
enum class EAFGamePhase : uint8
{
	EAF_Title            UMETA(DisplayName = "Title"),
	EAF_Loading          UMETA(DisplayName = "Loading"),
	EAF_Lobby            UMETA(DisplayName = "Lobby"),
	EAF_CharacterSelect  UMETA(DisplayName = "CharacterSelect"),
	EAF_InGame           UMETA(DisplayName = "InGame"),
	EAF_GameOver         UMETA(DisplayName = "GameOver"),
};

UENUM(BlueprintType)
enum class EAFTeamId : uint8
{
	Red  UMETA(DisplayName = "Red"),
	Blue UMETA(DisplayName = "Blue"),
	None UMETA(DisplayName = "None"),
};

USTRUCT(BlueprintType)
struct FAFMatchResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	EAFTeamId WinnerTeam = EAFTeamId::None;

	UPROPERTY(BlueprintReadOnly)
	int32 RedKills = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 BlueKills = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchResultChanged, const FAFMatchResult&, NewResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGamePhaseChangedDelegate, EAFGamePhase, NewPhase);

UCLASS()
class AFO_API AAFGameState : public AGameState
{
	GENERATED_BODY()

public:
	AAFGameState();

	UPROPERTY(BlueprintAssignable, Category = "Networking")
	FPlayerArrayChangedDelegate OnPlayerArrayChanged;

	UPROPERTY(ReplicatedUsing = OnRep_TeamPlayerArray, VisibleAnywhere, Category = "Networking")
	TArray<class AAFPlayerState*> TeamPlayerStatesReplicated;

	UFUNCTION()
	void OnRep_TeamPlayerArray();

	virtual void AddPlayerState(APlayerState* PlayerState) override;

	UPROPERTY(BlueprintAssignable, Category = "AFO|Events")
	FOnTimerChangedDelegate OnTimerChanged;

	UPROPERTY(ReplicatedUsing = OnRep_TeamScore, BlueprintReadOnly, Category = "AFO|Score")
	int32 TeamRedKillScore = 0;

	UPROPERTY(ReplicatedUsing = OnRep_TeamScore, BlueprintReadOnly, Category = "AFO|Score")
	int32 TeamBlueKillScore = 0;

	UPROPERTY(ReplicatedUsing = OnRep_TeamScore, BlueprintReadOnly, Category = "AFO|Score")
	int32 TeamRedDeathScore = 0;

	UPROPERTY(ReplicatedUsing = OnRep_TeamScore, BlueprintReadOnly, Category = "AFO|Score")
	int32 TeamBlueDeathScore = 0;

	UFUNCTION()
	void OnRep_TeamScore();

	UPROPERTY(BlueprintAssignable, Category = "AFO|Match")
	FOnMatchResultChanged OnMatchResultChanged;

	UPROPERTY(ReplicatedUsing = OnRep_MatchResult, BlueprintReadOnly, Category = "AFO|Match")
	FAFMatchResult MatchResult;

	UFUNCTION()
	void OnRep_MatchResult();

	void SetMatchResult(EAFTeamId WinnerTeam);

	UFUNCTION(BlueprintPure, Category = "AFO|Match")
	EAFTeamId GetWinnerTeam() const { return MatchResult.WinnerTeam; }

	void AddTeamScore(uint8 TeamID, bool bIsKill);

	UPROPERTY(BlueprintAssignable, Category = "AFO|Phase")
	FOnGamePhaseChangedDelegate OnGamePhaseChanged;

	UFUNCTION()
	void OnRep_GamePhase();

protected:
	FTimerHandle GameTimerHandle;

	UPROPERTY(ReplicatedUsing = OnRep_GamePhase, BlueprintReadOnly, Category = "AFO|Phase")
	EAFGamePhase CurrentGamePhase = EAFGamePhase::EAF_Title;

private:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_RemainingTime();

public:
	UPROPERTY(ReplicatedUsing = OnRep_RemainingTime, BlueprintReadOnly, Category = "AFO|Time")
	int32 RemainingTimeSeconds = 300;

	void StartGameTimer();
	void UpdateTimer();
	void SetRemainingTime(int32 NewTime);
	void SetGamePhase(EAFGamePhase NewPhase);

	UFUNCTION(BlueprintPure, Category = "AFO|Time")
	int32 GetRemainingTimeSeconds() const { return RemainingTimeSeconds; }

	UFUNCTION(BlueprintPure, Category = "AFO|Phase")
	EAFGamePhase GetCurrentGamePhase() const { return CurrentGamePhase; }
};

