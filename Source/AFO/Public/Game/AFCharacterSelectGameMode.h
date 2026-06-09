#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "AFCharacterSelectGameMode.generated.h"

USTRUCT(BlueprintType)
struct FAFCharacterOption
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	uint8 Id = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<APawn> PawnClass;
};

UCLASS()
class AFO_API AAFCharacterSelectGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AAFCharacterSelectGameMode();

	bool RequestSelectCharacter(AController* Requester, uint8 CharacterId);
	bool RequestSetReady(AController* Requester, bool bNewReady);

	UFUNCTION(BlueprintCallable)
	const TArray<FAFCharacterOption>& GetCharacterOptions() const { return CharacterOptions; }

private:
	bool IsCharacterTakenInTeam(uint8 TeamId, uint8 CharacterId, APlayerState* Except) const;
	bool IsAllPickedAndReady() const;
	void TryStartBattle();

private:
	UPROPERTY(EditDefaultsOnly, Category = "Travel")
	FString BattleZoneURL = TEXT("/Game/01_ArenaFighter/01_Levels/AFOBattleZone");

	UPROPERTY(EditDefaultsOnly, Category = "Character")
	TArray<FAFCharacterOption> CharacterOptions;
};
