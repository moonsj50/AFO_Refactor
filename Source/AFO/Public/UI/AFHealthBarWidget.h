// AFHealthBarWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "AFHealthBarWidget.generated.h"

UCLASS()
class AFO_API UAFHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 위젯이 생성될 때 캐릭터를 전달받아 감시를 시작합니다.
	UFUNCTION(BlueprintCallable, Category = "AFO|UI")
	void BindToCharacter(AActor* OwningActor);

protected:
	/** AttributeComponent의 체력 변경 시 호출될 콜백 함수 */
	UFUNCTION()
	void HandleHealthChanged(float CurrentHealth, float MaxHealth, class AAFPlayerState* ChangedPlayer);

	/** 블루프린트에서 UI를 업데이트하기 위한 이벤트 */
	UFUNCTION(BlueprintImplementableEvent, Category = "AFO|UI")
	void OnUpdateHealthVisual(float Percent);

	/** 블루프린트에서 팀 색상을 적용하기 위한 이벤트 */
	UFUNCTION(BlueprintImplementableEvent, Category = "AFO|UI")
	void OnUpdateTeamVisual(FLinearColor TeamColor);

	// 팀 정보가 올 때까지 반복 실행될 함수
	void UpdateInitialState();

	UFUNCTION()
	void AttemptBind();

private:
	UPROPERTY()
	TWeakObjectPtr<AActor> TargetActor;

	FTimerHandle InitializationTimerHandle;

	bool bIsInitialized = false;

public:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "AFO|UI")
	class UProgressBar* HealthPB;
};