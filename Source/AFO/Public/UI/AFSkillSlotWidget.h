// AFSkillSlotWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/AFGameTypes.h"
#include "AFSkillSlotWidget.generated.h"

class AFSkillComponent;

UCLASS()
class AFO_API UAFSkillSlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UAFSkillSlotWidget(const FObjectInitializer& ObjectInitializer);

	// 슬롯에 데이터를 채우는 핵심 함수
	UFUNCTION(BlueprintCallable, Category = "AF | UI")
	void SetSkillSlotInfo(FAFSkillInfo NewSkillInfo, FName InRowName);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetSkillComponent(UAFSkillComponent* InComponent) { MySkillComponent = InComponent; }


protected:
	// 블루프린트 위젯에 있는 Image 이름을 'SkillIconImage'로 지으면 자동으로 연결됨
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> SkillIcon;

	// 현재 슬롯이 들고 있는 데이터 (나중에 툴팁에 전달용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AF | Skill")
	FAFSkillInfo MySkillData;

	// 위젯이 관리할 스킬의 실제 행 이름 (예: "Mage_Q")
	UPROPERTY()
	FName MyRowName;

	// NativeTick에서 로그 출력 주기를 조절하기 위한 변수
	float LastLogTime = 0.f;


	// UI 업데이트를 위해 NativeTick을 오버라이드합니다.
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* CooldownBar;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* CooldownText;

	// 위젯 생성 시 또는 외부에서 할당해줘야 합니다.
	UPROPERTY()
	class UAFSkillComponent* MySkillComponent;


protected:
	// Tick 대신 사용할 타이머 핸들
	FTimerHandle CooldownUpdateTimerHandle;

	// 쿨타임을 실제로 갱신할 함수
	void UpdateCooldownVisual();

public:
	// 컴포넌트 주입 시 타이머 시작
	void StartUIUpdate();

};
