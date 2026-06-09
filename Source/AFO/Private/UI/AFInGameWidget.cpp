// AFInGameWidget.cpp


#include "UI/AFInGameWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Game/AFGameState.h"
#include "Kismet/GameplayStatics.h"
#include "AFO/Public/Player/AFPlayerState.h"
#include "GameFramework/PlayerController.h"
#include "AFO/Public/Components/AFAttributeComponent.h"
#include "Player/AFPlayerController.h"


// ====================
// 1. 초기화 및 바인딩 진입점
// ====================

void UAFInGameWidget::NativeConstruct()
{
	UUserWidget::NativeConstruct();

	// 1. 타이머 텍스트 초기화
	if (GameTimer) GameTimer->SetText(FText::FromString("Wait..."));

	AAFGameState* GS = Cast<AAFGameState>(GetWorld()->GetGameState());
	if (GS)
	{
		// 2. 타이머 델리게이트 연결
		UpdateGameTimerText(GS->RemainingTimeSeconds);
		GS->OnTimerChanged.AddUniqueDynamic(this, &UAFInGameWidget::UpdateGameTimerText);

		// 3. 점수/플레이어 배열 변화 델리게이트 연결 (AddUniqueDynamic 필수)
		GS->OnPlayerArrayChanged.AddUniqueDynamic(this, &UAFInGameWidget::HandlePlayerArrayChanged);

		// 4. 초기 바인딩 시도 (한 번만 수행)
		HandlePlayerArrayChanged();
	}
}

void UAFInGameWidget::HandlePlayerArrayChanged()
{
	AGameStateBase* GS = GetWorld()->GetGameState();
	if (!GS) return;

	// 현재 접속 중인 모든 플레이어를 대상으로 초기화 시도
	// bTeamUIInitialized 체크를 제거하고 내부에서 개별 플레이어별로 체크합니다.
	InitializeTeamUI(GS->PlayerArray);
	UpdateTeamKillDeathScore(0, nullptr);
}

void UAFInGameWidget::CheckAndInitializeUI()
{
	if (bTeamUIInitialized)
	{
		GetWorld()->GetTimerManager().ClearTimer(InitTimerHandle);
		return;
	}

	AGameStateBase* GS = GetWorld()->GetGameState();
	if (!GS)
	{
		return;
	}

	// 2v2 게임이라고 가정하고 최소 2명의 플레이어를 기다립니다.
	if (GS->PlayerArray.Num() < 2)
	{
		UE_LOG(LogTemp, Warning, TEXT("WAITING FOR ALL PLAYERS: Found %d/%d PlayerStates."), GS->PlayerArray.Num(), 2);
		return;
	}

	// ==========================================================
	// ★★★ 핵심 수정: 모든 PlayerState의 팀 ID가 복제되었는지 확인 ★★★
	// ==========================================================
	bool bAllPlayerStatesReady = true;
	for (APlayerState* PS : GS->PlayerArray)
	{
		AAFPlayerState* AFPS = Cast<AAFPlayerState>(PS);
		// TeamID가 초기값(보통 0)이 아닌 1 이상이거나, AFPS 캐스팅 자체가 실패하면 준비 안 됨
		// 혹은 TeamID가 0이나 1 (RED/BLUE)로 확실히 설정되었는지 확인
		if (!AFPS || (AFPS->GetTeamID() != 0 && AFPS->GetTeamID() != 1))
		{
			bAllPlayerStatesReady = false;
			// UE_LOG(LogTemp, Warning, TEXT("WAITING: PlayerState %s TeamID not yet replicated."), AFPS ? *AFPS->GetPlayerName() : TEXT("NULL"));
			break;
		}
	}

	if (!bAllPlayerStatesReady)
	{
		UE_LOG(LogTemp, Warning, TEXT("WAITING: PlayerStates found, but Team IDs are not fully replicated yet."));
		return;
	}

	// 모든 플레이어 (2명)가 PlayerArray에 있고, TeamID도 복제되었다면 초기화 실행
	InitializeTeamUI(GS->PlayerArray);

	// InitializeTeamUI 내부에서 bTeamUIInitialized = true 가 설정되어 다음 틱에 타이머가 Clear됩니다.
}

void UAFInGameWidget::InitializeTeamUI(TArray<APlayerState*> AllPlayerStates)
{
	// 기존의 bTeamUIInitialized 체크는 삭제하거나, 
	// 방의 최대 인원수(예: 4명)가 모두 찼을 때만 작동하도록 변경해야 합니다.
	// if (bTeamUIInitialized) return; 

	APlayerController* LocalPC = GetOwningPlayer();
	FString InstanceName = LocalPC ? LocalPC->GetName() : TEXT("UNKNOWN_CLIENT");

	// UE_LOG(LogTemp, Warning, TEXT("[%s] Attempting to bind %d PlayerStates."), *InstanceName, AllPlayerStates.Num());

	for (APlayerState* PS : AllPlayerStates)
	{
		AAFPlayerState* AFPS = Cast<AAFPlayerState>(PS);
		if (!AFPS) continue;

		// ★ 핵심 수정: 이미 바인딩을 완료한 플레이어는 다시 처리하지 않음 (중복 바인딩 방지)
		if (BoundPlayerStates.Contains(AFPS))
		{
			continue;
		}

		UE_LOG(LogTemp, Warning, TEXT("[%s] New Player Detected! Binding: %s (Team %d)"),
			*InstanceName, *AFPS->GetPlayerName(), AFPS->GetTeamID());

		// TeamPlayerStates Map 업데이트 (AddUnique 사용으로 중복 방지)
		TeamPlayerStates.FindOrAdd(AFPS->GetTeamID()).AddUnique(AFPS);

		// ==============
		// 1. 이름 설정
		// ==============
		FText PlayerNameText = FText::FromString(AFPS->GetPlayerName());
		if (AFPS->GetTeamID() == 0) // RED
		{
			if (AFPS->GetTeamIndex() == 1 && RedPlayerName01) RedPlayerName01->SetText(PlayerNameText);
			if (AFPS->GetTeamIndex() == 2 && RedPlayerName02) RedPlayerName02->SetText(PlayerNameText);
		}
		else // BLUE
		{
			if (AFPS->GetTeamIndex() == 1 && BluePlayerName01) BluePlayerName01->SetText(PlayerNameText);
			if (AFPS->GetTeamIndex() == 2 && BluePlayerName02) BluePlayerName02->SetText(PlayerNameText);
		}

		// ==============
		// 2. 델리게이트 바인딩 (로컬/스코어보드/팀스코어)
		// ==============

		// A. 로컬 전용 (HUD)
		if (AFPS->GetOwner() == LocalPC)
		{
			AFPS->OnHealthChanged.AddUniqueDynamic(this, &UAFInGameWidget::UpdateMyHealthBar);
			AFPS->OnManaChanged.AddUniqueDynamic(this, &UAFInGameWidget::UpdateMyManaBar);
			UpdateMyHealthBar(AFPS->GetCurrentHealth(), AFPS->GetMaxHealth(), AFPS);
			UpdateMyManaBar(AFPS->GetCurrentMana(), AFPS->GetMaxMana(), AFPS);
		}

		// B. 스코어보드 개별 바인딩
		AFPS->OnHealthChanged.AddUniqueDynamic(this, &UAFInGameWidget::UpdatePlayerHealthBar);
		AFPS->OnManaChanged.AddUniqueDynamic(this, &UAFInGameWidget::UpdatePlayerManaBar);
		AFPS->OnKillCountChanged.AddUniqueDynamic(this, &UAFInGameWidget::UpdatePlayerKillCount);
		AFPS->OnDeathCountChanged.AddUniqueDynamic(this, &UAFInGameWidget::UpdatePlayerDeathCount);

		// C. 팀 총합 스코어 바인딩
		// AFPS->OnKillCountChanged.AddUniqueDynamic(this, &UAFInGameWidget::UpdateTeamKillDeathScore);
		// AFPS->OnDeathCountChanged.AddUniqueDynamic(this, &UAFInGameWidget::UpdateTeamKillDeathScore);

		// 초기값 즉시 반영
		UpdatePlayerHealthBar(AFPS->GetCurrentHealth(), AFPS->GetMaxHealth(), AFPS);
		UpdatePlayerManaBar(AFPS->GetCurrentMana(), AFPS->GetMaxMana(), AFPS);
		UpdatePlayerKillCount(AFPS->GetKillCount(), AFPS);
		UpdatePlayerDeathCount(AFPS->GetDeathCount(), AFPS);

		// ★ 바인딩 완료 목록에 추가
		BoundPlayerStates.Add(AFPS);
	}

	// 팀 점수 최종 갱신
	UpdateTeamKillDeathScore(0, nullptr);

	// 최대 인원이 다 찼을 때만 초기화 완료 플래그 설정 (예: 4명 기준)
	if (BoundPlayerStates.Num() >= 4)
	{
		bTeamUIInitialized = true;
		UE_LOG(LogTemp, Warning, TEXT("[%s] ALL PLAYERS BOUND (%d/4)."), *InstanceName, BoundPlayerStates.Num());
	}
}

// 팀 스코어보드 핸들러
void UAFInGameWidget::UpdatePlayerHealthBar(float CurrentHealth, float MaxHealth, AAFPlayerState* TargetPS)
{
	if (!TargetPS || MaxHealth <= 0.f) return;
	float Percent = CurrentHealth / MaxHealth;

	UProgressBar* TargetHPBar = nullptr;

	if (TargetPS->GetTeamID() == 0) // RED 팀
	{
		TargetHPBar = (TargetPS->GetTeamIndex() == 1) ? RedPlayer1HP : RedPlayer2HP;

		// ★★★ RED 팀 포인터 유효성 확인 로그 ★★★
		if (!TargetHPBar)
		{
			// 이 로그가 특정 클라이언트에서만 출력되면 UMG 런타임 바인딩 실패 확정
			UE_LOG(LogTemp, Error, TEXT("FATAL_UMG_BINDING: Red HP Bar (Index %d) is NULL on client %s!"), TargetPS->GetTeamIndex(), *GetOwningPlayer()->GetName());
		}
	}
	else // BLUE 팀
	{
		TargetHPBar = (TargetPS->GetTeamIndex() == 1) ? BluePlayer1HP : BluePlayer2HP;
	}

	if (TargetHPBar)
	{
		TargetHPBar->SetPercent(Percent);
		// ★★★ 업데이트 성공 로그 ★★★
		UE_LOG(LogTemp, Warning, TEXT("SCOREBOARD UPDATE SUCCESS: Team %d Index %d HP %.2f"), TargetPS->GetTeamID(), TargetPS->GetTeamIndex(), CurrentHealth);
	}
	else
	{
		// ★★★ 업데이트 실패 (UMG 포인터 문제) 로그 ★★★
		UE_LOG(LogTemp, Warning, TEXT("SCOREBOARD UPDATE FAILED: Team %d Index %d. TargetHPBar is NULL."), TargetPS->GetTeamID(), TargetPS->GetTeamIndex());
	}
}

void UAFInGameWidget::UpdatePlayerManaBar(float CurrentMana, float MaxMana, AAFPlayerState* TargetPS)
{
	if (!TargetPS || MaxMana <= 0.f) return;
	float Percent = CurrentMana / MaxMana;

	UProgressBar* TargetMPBar = nullptr;

	if (TargetPS->GetTeamID() == 0) // RED 팀
	{
		TargetMPBar = (TargetPS->GetTeamIndex() == 1) ? RedPlayer1MP : RedPlayer2MP;
	}
	else // BLUE 팀
	{
		TargetMPBar = (TargetPS->GetTeamIndex() == 1) ? BluePlayer1MP : BluePlayer2MP;
	}

	if (TargetMPBar)
	{
		TargetMPBar->SetPercent(Percent);
	}
}

void UAFInGameWidget::UpdatePlayerKillCount(int32 NewKillCount, AAFPlayerState* TargetPS)
{
	if (!TargetPS) return;

	FText NewKillText = FText::AsNumber(NewKillCount);

	if (TargetPS->GetTeamID() == 0) // RED 팀
	{
		if (TargetPS->GetTeamIndex() == 1 && RedPlayer1Kill) RedPlayer1Kill->SetText(NewKillText);
		if (TargetPS->GetTeamIndex() == 2 && RedPlayer2Kill) RedPlayer2Kill->SetText(NewKillText);
	}
	else // BLUE 팀
	{
		if (TargetPS->GetTeamIndex() == 1 && BluePlayer1Kill) BluePlayer1Kill->SetText(NewKillText);
		if (TargetPS->GetTeamIndex() == 2 && BluePlayer2Kill) BluePlayer2Kill->SetText(NewKillText);
	}
}

void UAFInGameWidget::UpdatePlayerDeathCount(int32 NewDeathCount, AAFPlayerState* TargetPS)
{
	if (!TargetPS) return;

	FText NewDeathText = FText::AsNumber(NewDeathCount);

	if (TargetPS->GetTeamID() == 0) // RED 팀
	{
		if (TargetPS->GetTeamIndex() == 1 && RedPlayer1Death) RedPlayer1Death->SetText(NewDeathText);
		if (TargetPS->GetTeamIndex() == 2 && RedPlayer2Death) RedPlayer2Death->SetText(NewDeathText);
	}
	else // BLUE 팀
	{
		if (TargetPS->GetTeamIndex() == 1 && BluePlayer1Death) BluePlayer1Death->SetText(NewDeathText);
		if (TargetPS->GetTeamIndex() == 2 && BluePlayer2Death) BluePlayer2Death->SetText(NewDeathText);
	}
}

// 팀 총합 스코어 갱신 핸들러
void UAFInGameWidget::UpdateTeamKillDeathScore(int32 NewValue, AAFPlayerState* TargetPS)
{
	AAFGameState* GS = Cast<AAFGameState>(GetWorld()->GetGameState());
	if (!GS) return;

	// ★ 클라이언트 로그: 현재 내 화면의 이름과 서버에서 받은 실제 점수를 출력
	FString ClientName = GetOwningPlayer() ? GetOwningPlayer()->GetName() : TEXT("Unknown");

	UE_LOG(LogTemp, Log, TEXT("[CLIENT:%s] Updating UI Scoreboard - Real Server Values: R_Kill=%d, B_Kill=%d"),
		*ClientName, GS->TeamRedKillScore, GS->TeamBlueKillScore);

	if (RedKillScore) RedKillScore->SetText(FText::AsNumber(GS->TeamRedKillScore));
	if (RedDeathScore) RedDeathScore->SetText(FText::AsNumber(GS->TeamRedDeathScore));
	if (BlueKillScore) BlueKillScore->SetText(FText::AsNumber(GS->TeamBlueKillScore));
	if (BlueDeathScore) BlueDeathScore->SetText(FText::AsNumber(GS->TeamBlueDeathScore));
}

// 자신 전용 델리게이트 핸들러
void UAFInGameWidget::UpdateMyHealthBar(float NewHealth, float MaxHealth, AAFPlayerState* TargetPS)
{
	// 1. ProgressBar (PlayerHP) 업데이트
	if (PlayerHP)
	{
		float HealthPercent = MaxHealth > 0.f ? (NewHealth / MaxHealth) : 0.f;
		PlayerHP->SetPercent(HealthPercent);
	}

	// 2. TextBlock (Text_PlayerHP) 업데이트
	if (Text_PlayerHP)
	{
		FText HealthText = FText::Format(
			NSLOCTEXT("HealthUI", "HealthFormat", "{0} / {1}"),
			FText::AsNumber(FMath::RoundToInt(NewHealth)),
			FText::AsNumber(FMath::RoundToInt(MaxHealth))
		);
		Text_PlayerHP->SetText(HealthText);
	}

	UE_LOG(LogTemp, Log, TEXT("Local UI Health Updated: %.2f / %.2f"), NewHealth, MaxHealth);
}

void UAFInGameWidget::UpdateMyManaBar(float NewMana, float MaxMana, AAFPlayerState* TargetPS)
{
	// 1. ProgressBar (PlayerMP) 업데이트
	if (PlayerMP)
	{
		float ManaPercent = MaxMana > 0.f ? (NewMana / MaxMana) : 0.f;
		PlayerMP->SetPercent(ManaPercent);
	}

	// 2. TextBlock (Text_PlayerMP) 업데이트
	if (Text_PlayerMP)
	{
		FText ManaText = FText::Format(
			NSLOCTEXT("ManaUI", "ManaFormat", "{0} / {1}"),
			FText::AsNumber(FMath::RoundToInt(NewMana)),
			FText::AsNumber(FMath::RoundToInt(MaxMana))
		);
		Text_PlayerMP->SetText(ManaText);
	}

	UE_LOG(LogTemp, Log, TEXT("Local UI Mana Updated: %.2f / %.2f"), NewMana, MaxMana);
}

// 타이머 델리게이트 핸들러
void UAFInGameWidget::UpdateGameTimerText(int32 NewTime)
{
	if (!IsValid(GameTimer)) return;

	// "MM:SS" 포맷으로 변환
	int32 Minutes = NewTime / 60;
	int32 Seconds = NewTime % 60;

	FString TimeString = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
	GameTimer->SetText(FText::FromString(TimeString));

	// 30초 미만일 때 빨간색으로 변경하는 연출
	if (NewTime <= 30)
	{
		GameTimer->SetColorAndOpacity(FSlateColor(FLinearColor::Red));
	}

	if (NewTime <= 0)
	{
		ShowGameResult();
	}
}


void UAFInGameWidget::ShowGameResult()
{
	AAFGameState* GS = Cast<AAFGameState>(GetWorld()->GetGameState());
	if (!GS) return;

	APlayerController* PC = GetOwningPlayer();
	AAFPlayerController* AFPC = Cast<AAFPlayerController>(PC);
	AAFPlayerState* MyPS = PC ? PC->GetPlayerState<AAFPlayerState>() : nullptr;
	if (!MyPS) return;

	// ★★★ 핵심 수정: GameState의 복제된 점수를 직접 참조 ★★★
	int32 FinalRedKills = GS->TeamRedKillScore;
	int32 FinalBlueKills = GS->TeamBlueKillScore;

	bool bIsDraw = (FinalRedKills == FinalBlueKills);
	bool bIWin = false;

	if (!bIsDraw)
	{
		if (FinalRedKills > FinalBlueKills) bIWin = (MyPS->GetTeamID() == 0);
		else bIWin = (MyPS->GetTeamID() == 1);
	}

	// 3. 위젯 생성 및 출력
	TSubclassOf<UUserWidget> ResultClass = nullptr;

	if (bIsDraw)
	{
		ResultClass = DrawWidgetClass;
	}
	else
	{
		ResultClass = bIWin ? VictoryWidgetClass : LoseWidgetClass;
	}
	
	if (ResultClass)
	{
		UUserWidget* ResultWidget = CreateWidget<UUserWidget>(GetWorld(), ResultClass);
		if (ResultWidget)
		{
			AFPC->Client_ClearRespawnWidget();
			ResultWidget->AddToViewport();
		}
	}
}




// PlayerState를 찾았을 때 델리게이트를 바인딩하는 최종 함수
void UAFInGameWidget::BindToPlayerState(AAFPlayerState* PS)
{
	// 타이머가 작동 중이라면 중지
	if (PlayerStateCheckTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(PlayerStateCheckTimerHandle);
		UE_LOG(LogTemp, Warning, TEXT("!!! PlayerState Timer Cleared !!!"));
	}

	// 기존 델리게이트 해제 
	PS->OnHealthChanged.RemoveDynamic(this, &UAFInGameWidget::UpdateMyHealthBar);
	PS->OnManaChanged.RemoveDynamic(this, &UAFInGameWidget::UpdateMyManaBar);

	// 1. 델리게이트에 C++ 함수 바인딩 (자신 전용 핸들러)
	PS->OnHealthChanged.AddDynamic(this, &UAFInGameWidget::UpdateMyHealthBar);
	PS->OnManaChanged.AddDynamic(this, &UAFInGameWidget::UpdateMyManaBar);

	// 2. 초기 값 설정 (TargetPS 인자 추가)
	UpdateMyHealthBar(PS->GetCurrentHealth(), PS->GetMaxHealth(), PS);
	UpdateMyManaBar(PS->GetCurrentMana(), PS->GetMaxMana(), PS);

	UE_LOG(LogTemp, Warning, TEXT("!!! Local HUD SUCCESSFULLY Bound & Initialized via Timer !!!"));

	// 3. 모든 플레이어의 상태를 바인딩하기 위해 InitializeTeamUI 호출
	if (AGameStateBase* GS = GetWorld()->GetGameState())
	{
		InitializeTeamUI(GS->PlayerArray);
	}
}

// 타이머가 호출하는 반복 체크 함수
void UAFInGameWidget::CheckAndBindPlayerState()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		// 위젯 소유 컨트롤러가 사라졌다면 타이머 중지
		GetWorld()->GetTimerManager().ClearTimer(PlayerStateCheckTimerHandle);
		return;
	}

	if (AAFPlayerState* PS = PC->GetPlayerState<AAFPlayerState>())
	{
		// PlayerState를 찾으면 바인딩 함수 호출
		BindToPlayerState(PS);
	}
	else
	{
		// 아직 못 찾았으면 다음 주기까지 기다립니다.
		// UE_LOG(LogTemp, Log, TEXT("Waiting for PlayerState..."));
	}
}





