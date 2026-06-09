#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Game/AFGameState.h"
#include "AFPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UAFInGameWidget;  	// InGameUI
class UAFESCWidget;          	// InGameUI
class UAFScoreboardWidget;

UCLASS()
class AFO_API AAFPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AAFPlayerController();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputMappingContext* InputMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* MoveAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* JumpAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* LookAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* SprintAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* AttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* ESC;   	// InGameUI
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* SkillEAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* SkillQAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* HeavyAttackAction;


	UFUNCTION(BlueprintCallable, Category = "UI")
	void RefreshSkillUI(class UAFSkillComponent* InSkillComp);
	

	
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UAFScoreboardWidget> ScoreboardWidgetClass;

	UPROPERTY()
	UAFScoreboardWidget* ScoreboardWidget;

	UFUNCTION()
	void TryBindGameState();

	UFUNCTION()
	void OnMatchResultChanged(const FAFMatchResult& NewResult);

	UFUNCTION()
	void OnGamePhaseChanged(EAFGamePhase NewPhase);

	UFUNCTION()
	void OnPlayerArrayChanged();

	UFUNCTION()
	void OnTimerChanged(int32 NewRemainingTime);

	bool bScoreboardShown = false;

	void ShowScoreboardIfNeeded();


	// === InGameUI ===  //

	UPROPERTY(BlueprintReadOnly, Category = "UI")
	TObjectPtr<class UAFSkillMainWidget> SkillMainWidget;


	UPROPERTY(EditAnywhere, Category = "AF | UI")
	TSubclassOf<UUserWidget> SkillMainWidgetClass;


	// �ΰ��� HUD ���� Ŭ����
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AFO|Widgets")
	TSubclassOf<UAFInGameWidget> InGameWidgetClass;

	// ESC �޴� ���� Ŭ����
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AFO|Widgets")
	TSubclassOf<UAFESCWidget> ESCWidgetClass;

	// ������ �ΰ��� HUD �ν��Ͻ�
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "AFO|Widgets")
	TObjectPtr<UAFInGameWidget> InGameWidget;

	// ������ ESC �޴� �ν��Ͻ�
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "AFO|Widgets")
	TObjectPtr<UAFESCWidget> ESCWidget;

public:
	// ESC �޴� Ȱ��ȭ ����
	UPROPERTY(VisibleInstanceOnly, Category = "AFO|Widgets")
	bool bIsESCMenuOpen = false;

	// ESC �޴� ��� �Լ�
	void ToggleESCMenu();

private:
	// �Է� ������Ʈ ����
	virtual void SetupInputComponent() override;



protected:
	// 리스폰 위젯 클래스를 블루프린트에서 할당할 수 있게 선언
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UAFRespawnWidget> RespawnWidgetClass;

public:
	// 서버가 클라이언트에게 호출하는 함수
	UFUNCTION(Client, Reliable)
	void Client_ShowRespawnWidget(float Duration);

protected:
	// 생성된 위젯을 저장해둘 변수
	UPROPERTY()
	class UAFRespawnWidget* CurrentRespawnWidget;

public:
	// 위젯 제거 함수 추가
	UFUNCTION(Client, Reliable)
	void Client_ClearRespawnWidget();



public:
	// 서버가 호출할 킬 로그 표시 RPC
	UFUNCTION(Client, Reliable)
	void Client_ShowKillLog(const FString& KillerName, FLinearColor KillerColor, const FString& VictimName, FLinearColor VictimColor);

protected:
	UPROPERTY(EditAnywhere, Category = "AFO|UI")
	TSubclassOf<class UAFKillLogContainer> KillLogContainerClass;

	UPROPERTY()
	class UAFKillLogContainer* KillLogContainer;
};
