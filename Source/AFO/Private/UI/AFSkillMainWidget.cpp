// AFSkillMainWidget.h


#include "UI/AFSkillMainWidget.h"
#include "Components/AFSkillComponent.h"

void UAFSkillMainWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 2. 슬롯 위젯들을 배열에 순서대로 담습니다.
    SlotArray.Empty();

    // 유효성 검사 후 추가 (isnotvalid 문제를 여기서 체크 가능)
    if (Slot_0) SlotArray.Add(Slot_0);
    if (Slot_1) SlotArray.Add(Slot_1);
    if (Slot_2) SlotArray.Add(Slot_2);
    if (Slot_3) SlotArray.Add(Slot_3);
    if (Slot_4) SlotArray.Add(Slot_4);

    UE_LOG(LogTemp, Warning, TEXT("[AFO] SkillMain Initialized. Slots Linked: %d"), SlotArray.Num());


    APawn* OwningPawn = GetOwningPlayerPawn();
    if (OwningPawn)
    {
        UAFSkillComponent* SkillComp = OwningPawn->FindComponentByClass<UAFSkillComponent>();

        // 모든 슬롯 위젯들에게 컴포넌트 주입 (SlotQ, SlotE 등 BindWidget 된 변수들)
        if (SkillComp)
        {
            if (Slot_0) Slot_0->SetSkillComponent(SkillComp);
            if (Slot_1) Slot_1->SetSkillComponent(SkillComp);
            if (Slot_2) Slot_2->SetSkillComponent(SkillComp);
            if (Slot_3) Slot_3->SetSkillComponent(SkillComp);
            if (Slot_4) Slot_4->SetSkillComponent(SkillComp);
            // ... 나머지 슬롯들도 동일하게
        }
    }
}

void UAFSkillMainWidget::UpdateSkillSlots(const TArray<FAFSkillInfo>& CharacterSkills, const TArray<FName>& SkillRowNames)
{
    // [방어 코드] 만약 배열이 비어있다면 여기서 한 번 더 초기화 시도
    if (SlotArray.Num() == 0)
    {
        if (Slot_0) SlotArray.Add(Slot_0);
        if (Slot_1) SlotArray.Add(Slot_1);
        if (Slot_2) SlotArray.Add(Slot_2);
        if (Slot_3) SlotArray.Add(Slot_3);
        if (Slot_4) SlotArray.Add(Slot_4);

        UE_LOG(LogTemp, Warning, TEXT("[AFO] SlotArray was empty, Re-initialized! Count: %d"), SlotArray.Num());
    }

    if (SlotArray.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("[AFO] Critical: SlotArray is still Empty even after re-init!"));
        return;
    }

    // 캐릭터 데이터와 슬롯 개수 중 작은 값만큼 루프
    int32 MaxLoop = FMath::Min3(SlotArray.Num(), CharacterSkills.Num(), SkillRowNames.Num());
    for (int32 i = 0; i < MaxLoop; ++i)
    {
        if (SlotArray[i])
        {
            SlotArray[i]->SetSkillSlotInfo(CharacterSkills[i], SkillRowNames[i]);
            UE_LOG(LogTemp, Log, TEXT("[AFO] Slot  updated: "));
        }
    }
}

void UAFSkillMainWidget::UpdateAllSlotsComponent(UAFSkillComponent* InSkillComp)
{
    if (!InSkillComp) return;

    // 'Slot' 대신 'SlotPtr' 등으로 이름을 변경하여 멤버 변수와의 충돌을 피합니다.
    for (UAFSkillSlotWidget* SlotPtr : SlotArray)
    {
        if (SlotPtr)
        {
            SlotPtr->SetSkillComponent(InSkillComp);
            SlotPtr->StartUIUpdate();
        }
    }
}