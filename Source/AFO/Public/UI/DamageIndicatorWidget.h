// DamageIndicatorWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DamageIndicatorWidget.generated.h"

UCLASS()
class AFO_API UDamageIndicatorWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // BlueprintImplementableEvent: C++에서 정의만 하고, 실제 구현(로직)은 블루프린트에서 함
    UFUNCTION(BlueprintImplementableEvent, Category = "AFO|UI")
    void ReceiveSetDamageText(float DamageAmount, FLinearColor TextColor);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AFO|UI", meta = (AllowPrivateAccess = "true"))
    FVector TargetWorldLocation;
};