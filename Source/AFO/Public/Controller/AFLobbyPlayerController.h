// AFLobbyPlayerController.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "AFLobbyPlayerController.generated.h"

class UUserWidget;

UCLASS()
class AFO_API AAFLobbyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void PostSeamlessTravel() override;
	virtual void BeginPlayingState() override;
	virtual void ReceivedPlayer() override;

	UFUNCTION(Server, Reliable)
	void ServerRequestSetTeam(uint8 NewTeamId);

	UFUNCTION(Server, Reliable)
	void ServerRequestAdvanceToCharacterSelect();

	UFUNCTION(Server, Reliable)
	void ServerRequestSelectCharacter(uint8 CharacterId);

	UFUNCTION(Server, Reliable)
	void ServerRequestSetReady(bool bNewReady);

	UFUNCTION(Client, Reliable)
	void ClientShowMessage(const FString& Msg);

	UFUNCTION(Server, Reliable)
	void ServerSetPlayerName(const FString& InName);

private:
	void EnsureUI();
	void SetupUIForCurrentMap();
	void ClearCurrentUI();
	void SetUIInputMode(bool bUIOnly);
	void HandlePostLoadMap(UWorld* LoadedWorld);

private:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> TeamSelectWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> CharacterSelectWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> CurrentWidget;

private:
	FDelegateHandle PostLoadMapHandle;
	FTimerHandle EnsureUITimer;
};
