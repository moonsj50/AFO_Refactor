// AFFloatingDamagemanager.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AFFloatingDamageManager.generated.h"

class UDamageIndicatorWidget;

UCLASS()
class AFO_API AAFFloatingDamageManager : public AActor
{
	GENERATED_BODY()
	
public:
	AAFFloatingDamageManager();

	/** * 외부에서 데미지 표시를 요청할 때 호출
		 * @param Damage 데미지 양
		 * @param WorldLocation 데미지가 발생한 월드 위치
		 * @param bIsEnemyDamage true면 내가 적을 때린 것(파랑), false면 내가 맞은 것(빨강)
		 * @param bIsCritical 치명타 여부 (나중에 확장할 때 사용)
		 */
	UFUNCTION(BlueprintCallable, Category = "AFO|UI")
	void ShowDamage(float Damage, FVector WorldLocation, bool bIsEnemyDamage, bool bIsCritical = false);

protected:
	// 에디터에서 할당할 위젯 클래스
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AFO|UI")
	TSubclassOf<UDamageIndicatorWidget> DamageWidgetClass;

private:
	// 월드 좌표를 화면 좌표로 변환하여 띄워주는 내부 로직
	void SpawnDamageWidget(float Damage, FVector WorldLocation, FLinearColor Color, bool bIsCritical);
	
};
