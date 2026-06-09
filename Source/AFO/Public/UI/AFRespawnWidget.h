// AFRespawnWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AFRespawnWidget.generated.h"


UCLASS()
class AFO_API UAFRespawnWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 초기 설정 함수 (리스폰 시간 설정)
	void InitRespawnTimer(float InDuration);

protected:
	// 매 프레임 업데이트 (게이지 줄이기용)
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// UMG의 ProgressBar와 연결
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* RespawnProgressBar;

	// 남은 시간을 텍스트로 보여주기
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_RespawnTime;

private:
	float MaxDuration;
	float ElapsedTime;
};
