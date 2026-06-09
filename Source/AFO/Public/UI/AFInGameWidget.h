// AFInGameWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AFO/Public/Game/AFGameState.h"
#include "AFInGameWidget.generated.h"

class UTextBlock;
class UProgressBar;
class AAFPlayerState;

UCLASS()
class AFO_API UAFInGameWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// =============================
	// 1. 초기화 및 바인딩 진입점
	// =============================
	void InitializeTeamUI(TArray<APlayerState*> AllPlayerStates);

// =============================
// 2. 델리게이트 핸들러
// =============================

	// 팀 스코어보드 핸들러
	UFUNCTION()
	void UpdatePlayerHealthBar(float CurrentHealth, float MaxHealth, class AAFPlayerState* TargetPS);
	UFUNCTION()
	void UpdatePlayerManaBar(float CurrentMana, float MaxMana, class AAFPlayerState* TargetPS);
	UFUNCTION()
	void UpdatePlayerKillCount(int32 NewKillCount, AAFPlayerState* TargetPS);
	UFUNCTION()
	void UpdatePlayerDeathCount(int32 NewDeathCount, AAFPlayerState* TargetPS);
	
	// 팀 총합 스코어 갱신 핸들러
	UFUNCTION()
	void UpdateTeamKillDeathScore(int32 NewValue, class AAFPlayerState* TargetPS);

	// 자신 전용 델리게이트 핸들러
	UFUNCTION()
	void UpdateMyHealthBar(float NewHealth, float MaxHealth, AAFPlayerState* TargetPS);
	UFUNCTION()
	void UpdateMyManaBar(float NewMana, float MaxMana, AAFPlayerState* TargetPS);

	// 타이머 델리게이트 핸들러
	UFUNCTION()
	void UpdateGameTimerText(int32 NewTime);

// =============================
// 3. UMG 위젯 변수
// =============================
protected:

	// A. 플레이어 이름 (TextBlock)
	UPROPERTY(meta = (BindWidget)) class UTextBlock* RedPlayerName01;
	UPROPERTY(meta = (BindWidget)) class UTextBlock* RedPlayerName02;
	UPROPERTY(meta = (BindWidget)) class UTextBlock* BluePlayerName01;
	UPROPERTY(meta = (BindWidget)) class UTextBlock* BluePlayerName02;

	// B. 플레이어 HP/MP (ProgressBar)
	UPROPERTY(meta = (BindWidget)) class UProgressBar* RedPlayer1HP;
	UPROPERTY(meta = (BindWidget)) class UProgressBar* RedPlayer2HP;
	UPROPERTY(meta = (BindWidget)) class UProgressBar* RedPlayer1MP;
	UPROPERTY(meta = (BindWidget)) class UProgressBar* RedPlayer2MP;

	UPROPERTY(meta = (BindWidget)) class UProgressBar* BluePlayer1HP;
	UPROPERTY(meta = (BindWidget)) class UProgressBar* BluePlayer2HP;
	UPROPERTY(meta = (BindWidget)) class UProgressBar* BluePlayer1MP;
	UPROPERTY(meta = (BindWidget)) class UProgressBar* BluePlayer2MP;

	// C. 플레이어 킬/데스 (TextBlock)
	UPROPERTY(meta = (BindWidget)) class UTextBlock* RedPlayer1Kill;
	UPROPERTY(meta = (BindWidget)) class UTextBlock* RedPlayer1Death;
	UPROPERTY(meta = (BindWidget)) class UTextBlock* RedPlayer2Kill;
	UPROPERTY(meta = (BindWidget)) class UTextBlock* RedPlayer2Death;
	UPROPERTY(meta = (BindWidget)) class UTextBlock* BluePlayer1Kill;
	UPROPERTY(meta = (BindWidget)) class UTextBlock* BluePlayer1Death;
	UPROPERTY(meta = (BindWidget)) class UTextBlock* BluePlayer2Kill;
	UPROPERTY(meta = (BindWidget)) class UTextBlock* BluePlayer2Death;

	// D. 팀 총합 스코어 (TextBlock)
	UPROPERTY(meta = (BindWidget)) class UTextBlock* RedKillScore;
	UPROPERTY(meta = (BindWidget)) class UTextBlock* BlueKillScore;
	UPROPERTY(meta = (BindWidget)) class UTextBlock* RedDeathScore;
	UPROPERTY(meta = (BindWidget)) class UTextBlock* BlueDeathScore;

	// E. 자신의 캐릭터 HP/MP
		// [Healt, Mana Bar]
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> PlayerHP;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> PlayerMP;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_PlayerHP;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_PlayerMP;

	// F. 타이머
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> GameTimer;


private:
	// 모든 유효한 PlayerState를 팀 및 인덱스별로 저장. 팀 총합 스코어 계산
	TMap<uint8, TArray<AAFPlayerState*>> TeamPlayerStates;

protected:
	virtual void NativeConstruct() override;

	// PlayerState가 유효한지 반복 확인하기 위한 타이머 핸들
	FTimerHandle PlayerStateCheckTimerHandle;

	// PlayerState를 확인하고 바인딩하는 함수
	UFUNCTION()
	void CheckAndBindPlayerState();
	void BindToPlayerState(AAFPlayerState* PS);

	bool bTeamUIInitialized = false; // 초기화 플래그

	void CheckAndInitializeUI();
	FTimerHandle InitTimerHandle;

	UFUNCTION()
	void HandlePlayerArrayChanged();


	// 이미 UI 바인딩이 완료된 PlayerState들을 저장 (중복 방지)
	UPROPERTY()
	TSet<class AAFPlayerState*> BoundPlayerStates;

	// 최대 인원수 (GameState 등에서 가져오도록 설정 가능)
	int32 MaxPlayerCount = 4;



	protected:
		// 게임 승리 패배 결과 위젯
		UPROPERTY(EditDefaultsOnly, Category = "UI|Result")
		TSubclassOf<UUserWidget> VictoryWidgetClass;

		UPROPERTY(EditDefaultsOnly, Category = "UI|Result")
		TSubclassOf<UUserWidget> LoseWidgetClass;

		UPROPERTY(EditDefaultsOnly, Category = "UI|Result")
		TSubclassOf<UUserWidget> DrawWidgetClass;

		UFUNCTION()
		void ShowGameResult();

		int32 RedTotalKills = 0;
		int32 RedTotalDeaths = 0;
		int32 BlueTotalKills = 0;
		int32 BlueTotalDeaths = 0;
};
