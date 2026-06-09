// AFESCWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AFESCWidget.generated.h"

class UButton;

UCLASS()
class AFO_API UAFESCWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Resume;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Option;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Exit;

	virtual void NativeConstruct() override;

	// --- 버튼 클릭 이벤트 핸들러 ---
	UFUNCTION()
	void OnResumeClicked();

	UFUNCTION()
	void OnOptionClicked(); // (현재 기능 없음)

	UFUNCTION()
	void OnExitClicked();

public:
	// ESC 창을 닫는 외부 호출 함수 (플레이어 컨트롤러에서 사용)
	UFUNCTION(BlueprintCallable, Category = "AFO|UI")
	void CloseESCMenu();
};
