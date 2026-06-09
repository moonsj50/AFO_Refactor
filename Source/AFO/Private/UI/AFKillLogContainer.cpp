// AFKillLogContainer.cpp

#include "UI/AFKillLogContainer.h"
#include "UI/AFKillLogEntry.h"
#include "Components/VerticalBox.h"

void UAFKillLogContainer::AddKillLog(const FString& Killer, FLinearColor KillerColor, const FString& Victim, FLinearColor VictimColor)
{
    if (!KillLogEntryClass || !KillLogList) return;

    // 1. 개별 엔트리 위젯 생성
    UAFKillLogEntry* NewEntry = CreateWidget<UAFKillLogEntry>(this, KillLogEntryClass);
    if (NewEntry)
    {
        // 2. 데이터 세팅
        NewEntry->SetKillLog(Killer, KillerColor, Victim, VictimColor);

        // 3. 리스트(VerticalBox)에 추가
        KillLogList->AddChildToVerticalBox(NewEntry);

        // 4. 실무 팁: 로그가 너무 많아지면 가장 오래된 것 삭제 (최대 5개 유지)
        if (KillLogList->GetChildrenCount() > 5)
        {
            KillLogList->RemoveChildAt(0);
        }
    }
}



