#pragma once

#include "Blueprint/UserWidget.h"
#include "UW_TitleLayout.generated.h"

class UButton;
class UEditableText;

/**
 *
 */
UCLASS()
class AFO_API UUW_TitleLayout : public UUserWidget
{
	GENERATED_BODY()

public:
	UUW_TitleLayout(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnPlayButtonClicked();

	UFUNCTION()
	void OnExitButtonClicked();

	UFUNCTION()
	void OnOptionButtonClicked();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = USTitleWidget, Meta = (AllowPrivateAccess, BindWidget))
	TObjectPtr<UButton> PlayButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = USTitleWidget, Meta = (AllowPrivateAccess, BindWidget))
	TObjectPtr<UButton> ExitButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = USTitleWidget, Meta = (AllowPrivateAccess, BindWidget))
	TObjectPtr<UButton> OptionButton;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = USLobbyLevelUI, Meta = (AllowPrivateAccess, BindWidget))
	TObjectPtr<UEditableText> ServerIPEditableText;

	UPROPERTY(meta = (BindWidget))
	class UEditableText* Text_PlayerName;

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = USLobbyLevelUI, Meta = (AllowPrivateAccess, BindWidget))
	//TObjectPtr<UEditableText> ServerIPEditableText;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> OptionsWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> OptionsWidget;

private:
	void ShowOptions();
	void HideOptions();
};