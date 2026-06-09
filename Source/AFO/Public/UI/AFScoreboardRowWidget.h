#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AFScoreboardRowWidget.generated.h"

class UTextBlock;
class AAFPlayerState;

UCLASS()
class AFO_API UAFScoreboardRowWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void Init(AAFPlayerState* InPS);

protected:
	virtual void NativeDestruct() override;

private:
	void RefreshTexts();

	UFUNCTION()
	void OnNameChanged(AAFPlayerState* ChangedPS);

	UFUNCTION()
	void OnTeamChanged(AAFPlayerState* ChangedPS);

	UFUNCTION()
	void OnCharacterChanged(AAFPlayerState* ChangedPS);

private:
	UPROPERTY(Transient)
	TObjectPtr<AAFPlayerState> PlayerStateRef;

private:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Scoreboard", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> Text_PlayerName;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Scoreboard", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> Text_CharacterName;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Scoreboard", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> Text_Kills;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Scoreboard", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> Text_Deaths;
};
