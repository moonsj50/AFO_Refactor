// AFFloatingDamagemanager.cpp


#include "UI/AFFloatingDamageManager.h"
#include "UI/DamageIndicatorWidget.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"


AAFFloatingDamageManager::AAFFloatingDamageManager()
{
    PrimaryActorTick.bCanEverTick = false;
}


void AAFFloatingDamageManager::ShowDamage(float Damage, FVector WorldLocation, bool bIsEnemyDamage, bool bIsCritical)
{
    // 1. 색상 결정 (요구사항: 공격 시 파란색, 피격 시 빨간색)
    // bIsEnemyDamage가 true라는 것은 '내가 적을 맞췄다'는 뜻으로 가정합니다.
    FLinearColor DisplayColor = bIsEnemyDamage ? FLinearColor::Blue : FLinearColor::Red;

    // 2. 만약 크리티컬이라면 색상을 노란색으로 덮어쓰거나 나중에 로직 추가 가능
    if (bIsCritical)
    {
        // DisplayColor = FLinearColor::Yellow; 
    }

    SpawnDamageWidget(Damage, WorldLocation, DisplayColor, bIsCritical);
}


void AAFFloatingDamageManager::SpawnDamageWidget(float Damage, FVector WorldLocation, FLinearColor Color, bool bIsCritical)
{
    if (!DamageWidgetClass) return;


    // 랜덤 오프셋 계산 (좌우 -30~30, 위 0~20)
    FVector RandomOffset = FVector(
        FMath::RandRange(-30.f, 30.f),
        FMath::RandRange(-30.f, 30.f),
        FMath::RandRange(0.f, 20.f)
    );
    FVector FinalLocation = WorldLocation + RandomOffset;


    // 위젯 생성
    UDamageIndicatorWidget* NewWidget = CreateWidget<UDamageIndicatorWidget>(GetWorld(), DamageWidgetClass);
    if (NewWidget)
    {

        NewWidget->TargetWorldLocation = FinalLocation;
        // C++에서 선언한 이벤트 호출 (블루프린트에서 가시적인 처리를 하도록 넘김)
        NewWidget->ReceiveSetDamageText(Damage, Color);

        // 화면에 추가
        NewWidget->AddToViewport(10);

        // 4. [중요] 위젯이 월드 위치를 따라가게 하려면 
        // 위젯 내부에서 ProjectWorldLocationToWidgetPosition 같은 함수를 써야 합니다.
        // 이 부분은 블루프린트에서 처리하는 것이 훨씬 유연합니다.
    }
}