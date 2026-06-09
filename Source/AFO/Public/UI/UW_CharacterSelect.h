#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Player/AFPlayerState.h"
#include "UW_CharacterSelect.generated.h"

class UButton;
class UTextBlock;
class UVerticalBox;

UCLASS()
class AFO_API UUW_CharacterSelect : public UUserWidget
{
	GENERATED_BODY()

public:
	FTimerHandle BindRetryHandle;
	bool bBound = false;
	void TryBindGameState();

protected:
	virtual void NativeConstruct() override;

	UFUNCTION() void OnClickChar0();
	UFUNCTION() void OnClickChar1();
	UFUNCTION() void OnClickChar2();
	UFUNCTION() void OnClickChar3();
	UFUNCTION() void OnClickChar4();
	UFUNCTION() void OnClickReady();
	UFUNCTION() void RefreshUI();
	UFUNCTION() void RefreshUI_FromDelegate(AAFPlayerState* ChangedPS);

private:
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> BtnChar0;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> BtnChar1;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> BtnChar2;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> BtnChar3;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> BtnChar4;

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> TxtChar0;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> TxtChar1;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> TxtChar2;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> TxtChar3;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> TxtChar4;

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> BtnReady;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> TxtReady;

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UVerticalBox> VBPlayerList;

	void SelectCharacter(uint8 Id);
	void RebuildPlayerList();

	FTimerHandle RefreshTimerHandle;
};
