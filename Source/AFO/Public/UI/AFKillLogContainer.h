// AFKillLogContainer.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AFKillLogContainer.generated.h"

UCLASS()
class AFO_API UAFKillLogContainer : public UUserWidget
{
	GENERATED_BODY()
	
protected:
    // 킬 로그들이 담길 수직 박스 (WBP에서 이 이름으로 VerticalBox를 만들어야 함)
    UPROPERTY(meta = (BindWidget))
    class UVerticalBox* KillLogList;

    // 생성할 개별 로그 위젯 클래스 (에디터에서 WBP_KillLogEntry 할당)
    UPROPERTY(EditAnywhere, Category = "AFO|UI")
    TSubclassOf<class UAFKillLogEntry> KillLogEntryClass;

public:
    // 새로운 킬 로그를 추가하는 함수
    void AddKillLog(const FString& Killer, FLinearColor KillerColor, const FString& Victim, FLinearColor VictimColor);

};
