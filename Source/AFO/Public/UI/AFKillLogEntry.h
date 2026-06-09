// AFKillLogEntry.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AFKillLogEntry.generated.h"

UCLASS()

class AFO_API UAFKillLogEntry : public UUserWidget
{
    GENERATED_BODY()

protected:
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* KillerNameText;

    UPROPERTY(meta = (BindWidget))
    class UImage* KillIconImage;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* VictimNameText;

    UPROPERTY(Transient, meta = (BindWidgetAnim))
    class UWidgetAnimation* AppearAnim;

    FTimerHandle DestroyTimerHandle;

public:
    // 데이터를 세팅하는 함수
    void SetKillLog(const FString& KillerName, FLinearColor KillerColor, const FString& VictimName, FLinearColor VictimColor);


    // 초기 설정 및 애니메이션 실행
    virtual void NativeConstruct() override;

    // 제거 함수
    void RemoveSelf();
};

