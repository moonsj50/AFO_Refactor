#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "AFLobbyGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLobbyCountsChanged);

UCLASS()
class AFO_API AAFLobbyGameState : public AGameState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing = OnRep_Counts, BlueprintReadOnly)
	int32 ConnectedPlayers = 0;

	UPROPERTY(ReplicatedUsing = OnRep_Counts, BlueprintReadOnly)
	int32 RedCount = 0;

	UPROPERTY(ReplicatedUsing = OnRep_Counts, BlueprintReadOnly)
	int32 BlueCount = 0;

	UPROPERTY(BlueprintAssignable)
	FLobbyCountsChanged OnCountsChanged;

	UFUNCTION()
	void OnRep_Counts();
};