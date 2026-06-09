#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "AFTeamSelectGameMode.generated.h"

UCLASS()
class AFO_API AAFTeamSelectGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AAFTeamSelectGameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	bool RequestSetTeam(AController* Requester, uint8 NewTeamId);
	bool CanAdvanceToCharacterSelect() const;
	bool AdvanceToCharacterSelect();
	virtual FString InitNewPlayer(APlayerController* NewPlayerController,
		const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal) override;


private:
	int32 GetTeamCount(uint8 TeamId) const;
	void RebuildTeamIndices();
	void UpdateLobbyCounts();

	UPROPERTY(EditDefaultsOnly, Category = "Travel")
	FString CharacterSelectMapURL = TEXT("/Game/01_ArenaFighter/01_Levels/AFOCharacterSelectMap");
};
