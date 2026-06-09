// AFKillLogEntry.cpp



#include "UI/AFKillLogEntry.h"
#include "Components/TextBlock.h"


void UAFKillLogEntry::NativeConstruct()
{
    Super::NativeConstruct();

    // 1. 나타나기 애니메이션 실행 (우측 슬라이드 + 페이드 인)
    if (AppearAnim)
    {
        PlayAnimation(AppearAnim);
    }

    // 2. 3초 뒤에 삭제 함수 호출 예약
    GetWorld()->GetTimerManager().SetTimer(DestroyTimerHandle, this, &UAFKillLogEntry::RemoveSelf, 3.0f, false);
}

void UAFKillLogEntry::RemoveSelf()
{
    RemoveFromParent();
}

void UAFKillLogEntry::SetKillLog(const FString& KillerName, FLinearColor KillerColor, const FString& VictimName, FLinearColor VictimColor)

{

    if (KillerNameText)

    {

        KillerNameText->SetText(FText::FromString(KillerName));

        KillerNameText->SetColorAndOpacity(FSlateColor(KillerColor));

    }



    if (VictimNameText)

    {

        VictimNameText->SetText(FText::FromString(VictimName));

        VictimNameText->SetColorAndOpacity(FSlateColor(VictimColor));

    }



    // 일정 시간 후 자동으로 파괴되는 애니메이션을 실행하거나 RemoveFromParent를 호출하게 설계하세요.

}