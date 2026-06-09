#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Game/AFGameState.h"
#include "AFScoreboardWidget.generated.h"

class UScrollBox;
class UTextBlock;
class UImage;
class UAFScoreboardRowWidget;

UCLASS()
class AFO_API UAFScoreboardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void SetMatchResult(const FAFMatchResult& InResult);

	UFUNCTION()
	void RefreshPlayerList();

	UFUNCTION()
	void RefreshTotals();

protected:
	virtual void NativeConstruct() override;

private:
	void RefreshResultText();
	void RefreshWinnerImages();

private:
	FAFMatchResult CachedResult;

	UPROPERTY(EditDefaultsOnly, Category = "Scoreboard")
	TSubclassOf<UAFScoreboardRowWidget> RowWidgetClass;

	// === Wrapper ScrollBox (디자이너 구조 유지용) : 안 써도 되지만 있어도 됨 ===
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
	TObjectPtr<UScrollBox> SB_Red;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
	TObjectPtr<UScrollBox> SB_Blue;

	// === 실제로 Row가 들어갈 곳 (Top/Bottom) ===
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
	TObjectPtr<UScrollBox> SB_RedTop;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
	TObjectPtr<UScrollBox> SB_RedBottom;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
	TObjectPtr<UScrollBox> SB_BlueTop;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
	TObjectPtr<UScrollBox> SB_BlueBottom;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> TB_RedTotalKills;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> TB_BlueTotalKills;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
	TObjectPtr<UTextBlock> TB_Result;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
	TObjectPtr<UImage> Win_Image;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
	TObjectPtr<UImage> Red_Image;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
	TObjectPtr<UImage> Blue_Image;
};
