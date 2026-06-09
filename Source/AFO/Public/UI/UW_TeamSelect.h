#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Player/AFPlayerState.h"
#include "UW_TeamSelect.generated.h"

class UButton;
class UTextBlock;
class UVerticalBox;

UCLASS()
class AFO_API UUW_TeamSelect : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void OnAnyPlayerNameChanged(AAFPlayerState* ChangedPS);

	void BindPlayerDelegates();


protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnClickToRed();

	UFUNCTION()
	void OnClickToBlue();

	UFUNCTION()
	void OnClickNext();

	UFUNCTION()
	void RefreshUI();

	UFUNCTION()
	void OnAnyPlayerTeamChanged(AAFPlayerState* ChangedPS);

	FString BuildTeamList(uint8 TeamId) const;


	void RebuildTeamLists();


	void RebuildPlayerList();


	void TryBindGameState();

private:
	FTimerHandle BindRetryHandle;
	bool bBound = false;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BtnToRed;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BtnToBlue;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BtnNext;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TxtCounts;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> VBPlayerList;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_RedCount;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_RedList;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_BlueCount;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_BlueList;
};
