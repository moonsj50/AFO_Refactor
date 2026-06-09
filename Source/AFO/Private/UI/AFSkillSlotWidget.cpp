// AFSkillSlotWidget.cpp


#include "UI/AFSkillSlotWidget.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/AFSkillComponent.h"
#include "TimerManager.h"

UAFSkillSlotWidget::UAFSkillSlotWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // C++ 위젯에서 Tick을 활성화하기 위한 필수 설정
   // bHasScriptImplementedTick = true;
}

void UAFSkillSlotWidget::SetSkillSlotInfo(FAFSkillInfo NewSkillInfo, FName InRowName)
{
    MySkillData = NewSkillInfo;
    MyRowName = InRowName;

    // [핵심 추가] 아이콘 이미지를 위젯에 셋팅 (이 코드가 없어서 안 보였던 것임)
    if (SkillIcon)
    {
        if (MySkillData.SkillIcon)
        {
            // TObjectPtr은 Get()을 통해 일반 포인터로 변환하거나 직접 대입 가능합니다.
            SkillIcon->SetBrushFromTexture(MySkillData.SkillIcon);
            SkillIcon->SetVisibility(ESlateVisibility::Visible);

            UE_LOG(LogTemp, Log, TEXT("@@@ [Slot::SetInfo] Icon Applied: %s"), *MyRowName.ToString());
        }
    }

    // 쿨타임 타이머 시작
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(CooldownUpdateTimerHandle);
        GetWorld()->GetTimerManager().SetTimer(
            CooldownUpdateTimerHandle,
            this,
            &UAFSkillSlotWidget::UpdateCooldownVisual,
            0.03f,
            true
        );

        UE_LOG(LogTemp, Warning, TEXT("@@@ [UI] Timer Started for Cooldown: %s"), *MyRowName.ToString());
    }
}

void UAFSkillSlotWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // 1. 컴포넌트 유효성 검사 및 자가 치유
    if (!MySkillComponent)
    {
        APawn* OwningPawn = GetOwningPlayerPawn();
        if (OwningPawn)
        {
            MySkillComponent = OwningPawn->FindComponentByClass<UAFSkillComponent>();
            if (MySkillComponent)
            {
                UE_LOG(LogTemp, Warning, TEXT("@@@ [Slot::Tick] Success! SkillComponent found for Slot: %s"), *MyRowName.ToString());
            }
        }
    }

    // 2. 핵심 로직 분기점 로그
    if (MySkillComponent && !MyRowName.IsNone())
    {
        float RemainingRatio = MySkillComponent->GetCooldownRemainingRatio(MyRowName);
        float RemainingTime = MySkillComponent->GetRemainingTime(MyRowName);

        // [중요 디버그] RemainingRatio가 0보다 클 때만 로그를 찍어 실제 연산 확인
        if (RemainingRatio > 0.001f)
        {
            // 2초마다 현재 연산 상태 출력 (로그 도배 방지)
            static float CooldownLogTimer = 0.f;
            CooldownLogTimer += InDeltaTime;
            if (CooldownLogTimer >= 2.0f)
            {
                UE_LOG(LogTemp, Log, TEXT("@@@ [Slot::Tick] %s Cooldown: %.2f (Ratio: %.4f)"),
                    *MyRowName.ToString(), RemainingTime, RemainingRatio);
                CooldownLogTimer = 0.f;
            }

            // 3. ProgressBar 업데이트 로그
            if (CooldownBar)
            {
                CooldownBar->SetPercent(RemainingRatio);
                CooldownBar->SetVisibility(ESlateVisibility::Visible);
            }
            else
            {
                // 바인딩 안됨
                static float ErrorLogTimer = 0.f;
                ErrorLogTimer += InDeltaTime;
                if (ErrorLogTimer >= 5.0f)
                {
                    UE_LOG(LogTemp, Error, TEXT("@@@ [Slot::Tick] CooldownBar is NULL for %s"), *MyRowName.ToString());
                    ErrorLogTimer = 0.f;
                }
            }

            // 4. 텍스트 업데이트
            if (CooldownText)
            {
                CooldownText->SetVisibility(ESlateVisibility::HitTestInvisible);
                CooldownText->SetText(FText::AsNumber(FMath::CeilToInt(RemainingTime)));
            }
        }
        else
        {
            // 쿨타임이 0일 때 UI 정리
            if (CooldownBar) CooldownBar->SetVisibility(ESlateVisibility::Hidden);
            if (CooldownText) CooldownText->SetVisibility(ESlateVisibility::Hidden);
        }
    }
    else
    {
        // 왜 쿨타임 계산에 진입하지 못하는지 이유 출력
        static float FailLogTimer = 0.f;
        FailLogTimer += InDeltaTime;
        if (FailLogTimer >= 3.0f)
        {
            UE_LOG(LogTemp, Error, TEXT("@@@ [Slot::Tick] Fail Entry - Comp: %s, RowName: %s"),
                MySkillComponent ? TEXT("Valid") : TEXT("NULL"),
                *MyRowName.ToString());
            FailLogTimer = 0.f;
        }
    }
}

void UAFSkillSlotWidget::StartUIUpdate()
{
    // 0.03초마다(약 30fps) UI를 갱신하는 타이머 시작
    GetWorld()->GetTimerManager().SetTimer(CooldownUpdateTimerHandle, this, &UAFSkillSlotWidget::UpdateCooldownVisual, 0.03f, true);
}

void UAFSkillSlotWidget::UpdateCooldownVisual()
{
    // MySkillComponent가 유효한지 확인 (Valid 체크는 시니어의 기본)
    if (!IsValid(MySkillComponent)) return;

    float Ratio = MySkillComponent->GetCooldownRemainingRatio(MyRowName);
    float Remaining = MySkillComponent->GetRemainingTime(MyRowName);

    if (CooldownBar)
    {
        // 쿨타임이 있을 때만 보이게 처리
        CooldownBar->SetPercent(Ratio);
        CooldownBar->SetVisibility(Ratio > 0.f ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    }

    if (CooldownText)
    {
        if (Remaining > 0.f)
        {
            CooldownText->SetVisibility(ESlateVisibility::HitTestInvisible);
            CooldownText->SetText(FText::AsNumber(FMath::CeilToInt(Remaining)));
        }
        else
        {
            CooldownText->SetVisibility(ESlateVisibility::Hidden);
        }
    }
}