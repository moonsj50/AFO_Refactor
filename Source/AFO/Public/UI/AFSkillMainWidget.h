// AFSkillMainWidget.cpp

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/AFSkillSlotWidget.h"
#include "AFSkillMainWidget.generated.h"

/**
 * 
 */
UCLASS()
class AFO_API UAFSkillMainWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
    // 1. 위젯 바인딩: 
    UPROPERTY(meta = (BindWidget))
    class UAFSkillSlotWidget* Slot_0;

    UPROPERTY(meta = (BindWidget))
    class UAFSkillSlotWidget* Slot_1;

    UPROPERTY(meta = (BindWidget))
    class UAFSkillSlotWidget* Slot_2;

    UPROPERTY(meta = (BindWidget))
    class UAFSkillSlotWidget* Slot_3;

    UPROPERTY(meta = (BindWidget))
    class UAFSkillSlotWidget* Slot_4;

    // 슬롯들을 루프로 돌리기 위해 관리하는 배열
    UPROPERTY()
    TArray<class UAFSkillSlotWidget*> SlotArray;

public:
    // 위젯이 초기화될 때 호출되는 함수 (BP의 Construct와 유사)
    virtual void NativeConstruct() override;

    // 캐릭터로부터 스킬 데이터를 받아 UI를 갱신하는 함수
    UFUNCTION(BlueprintCallable, Category = "AFO|UI")
    void UpdateSkillSlots(const TArray<FAFSkillInfo>& CharacterSkills, const TArray<FName>& SkillRowNames);

    void UpdateAllSlotsComponent(class UAFSkillComponent* InSkillComp);
};
