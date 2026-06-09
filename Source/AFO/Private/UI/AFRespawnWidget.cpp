// AFRespawnWidget.cpp


#include "UI/AFRespawnWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"


void UAFRespawnWidget::InitRespawnTimer(float InDuration)
{
	MaxDuration = InDuration;
	ElapsedTime = 0.0f;

	if (RespawnProgressBar)
	{
		RespawnProgressBar->SetPercent(1.0f); // 처음에 가득 채움
	}
}

void UAFRespawnWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	ElapsedTime += InDeltaTime;

	// 1.0에서 0.0으로 줄어드는 비율 계산
	float NewPercent = FMath::Clamp(1.0f - (ElapsedTime / MaxDuration), 0.0f, 1.0f);

	if (RespawnProgressBar)
	{
		RespawnProgressBar->SetPercent(NewPercent);
	}

	// 숫자로 보여주기
	if (Text_RespawnTime)
	{
		int32 RemainingSeconds = FMath::CeilToInt(MaxDuration - ElapsedTime);
		RemainingSeconds = FMath::Max(0, RemainingSeconds); // 0 이하로 내려가지 않게

		FText FormattedText = FText::Format(
			NSLOCTEXT("UI", "RespawnWaitFormat", "Please wait {0}s."),
			FText::AsNumber(RemainingSeconds)
		);
		Text_RespawnTime->SetText(FormattedText);
	}

	// 시간이 다 되면 스스로 제거
	if (ElapsedTime >= MaxDuration)
	{
		RemoveFromParent();
	}
}
