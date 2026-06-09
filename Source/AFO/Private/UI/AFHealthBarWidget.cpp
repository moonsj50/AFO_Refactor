// AFHealthBarWidget.cpp


#include "UI/AFHealthBarWidget.h"
#include "Components/AFAttributeComponent.h"
#include "Player/AFPlayerState.h"
#include "GameFramework/Pawn.h"

void UAFHealthBarWidget::BindToCharacter(AActor* OwningActor)
{
	if (!OwningActor || bIsInitialized) return;

	TargetActor = OwningActor;

	// InGameWidget처럼 타이머를 사용하여 반복 체크 (0.1~0.2초 간격)
	GetWorld()->GetTimerManager().SetTimer(InitializationTimerHandle, this, &UAFHealthBarWidget::AttemptBind, 0.2f, true);
}



void UAFHealthBarWidget::HandleHealthChanged(float CurrentHealth, float MaxHealth, class AAFPlayerState* ChangedPlayer)
{
    if (MaxHealth <= 0.f) return;

    float Percent = CurrentHealth / MaxHealth;

    // [핵심] C++에서 직접 프로그레스 바 업데이트
    if (HealthPB)
    {
        HealthPB->SetPercent(Percent);
        UE_LOG(LogTemp, Log, TEXT("[HealthBar] Direct UI Update: %.2f%%"), Percent * 100.f);
    }

    // 기존 이벤트는 연출용(애니메이션 등)으로 남겨두고 호출만 해줍니다.
    OnUpdateHealthVisual(Percent);
}


void UAFHealthBarWidget::UpdateInitialState()
{
	AActor* Actor = TargetActor.Get();
	if (!Actor)
	{
		GetWorld()->GetTimerManager().ClearTimer(InitializationTimerHandle);
		return;
	}

	APawn* OwningPawn = Cast<APawn>(Actor);
	if (!OwningPawn) return;

	// 내 PlayerState와 상대방 PlayerState를 모두 가져옵니다.
	APlayerController* LocalPC = GetWorld()->GetFirstPlayerController();
	AAFPlayerState* LocalPS = LocalPC ? LocalPC->GetPlayerState<AAFPlayerState>() : nullptr;
	AAFPlayerState* TargetPS = OwningPawn->GetPlayerState<AAFPlayerState>();

	// ★ 핵심: 둘 다 존재해야 하며, 팀 ID가 초기값(보통 255)이 아닐 때만 실행합니다.
	if (LocalPS && TargetPS && TargetPS->GetTeamID() != 255)
	{
		// 1. 체력 변경 델리게이트 연결 (AddUnique로 중복 방지)
		TargetPS->OnHealthChanged.AddUniqueDynamic(this, &UAFHealthBarWidget::HandleHealthChanged);

		// 2. 초기 값 즉시 반영
		HandleHealthChanged(TargetPS->GetCurrentHealth(), TargetPS->GetMaxHealth(), TargetPS);

		// 3. 팀 색상 설정 (내 팀과 같으면 파랑, 다르면 빨강)
		FLinearColor TeamColor = (LocalPS->GetTeamID() == TargetPS->GetTeamID()) ? FLinearColor::Blue : FLinearColor::Red;
		OnUpdateTeamVisual(TeamColor);

		// 바인딩에 성공했으므로 타이머를 중지합니다.
		GetWorld()->GetTimerManager().ClearTimer(InitializationTimerHandle);

		UE_LOG(LogTemp, Log, TEXT("HealthBar Successfully Bound to: %s (Team: %d)"), *Actor->GetName(), TargetPS->GetTeamID());
	}
}



void UAFHealthBarWidget::AttemptBind()
{
    if (bIsInitialized)
    {
        GetWorld()->GetTimerManager().ClearTimer(InitializationTimerHandle);
        return;
    }

    AActor* Actor = TargetActor.Get();
    if (!Actor) return;

    APawn* OwningPawn = Cast<APawn>(Actor);
    if (!OwningPawn) return;

    AAFPlayerState* TargetPS = OwningPawn->GetPlayerState<AAFPlayerState>();
    APlayerController* LocalPC = GetWorld()->GetFirstPlayerController();
    AAFPlayerState* LocalPS = LocalPC ? LocalPC->GetPlayerState<AAFPlayerState>() : nullptr;

    // 1. InGameWidget처럼 TeamID와 PlayerState가 완전히 복제되었는지 확인
    if (TargetPS && LocalPS && TargetPS->GetTeamID() != 255 && LocalPS->GetTeamID() != 255)
    {
        float CurrHP = TargetPS->GetCurrentHealth();
        float MaxHP = TargetPS->GetMaxHealth();

        if (MaxHP > 100.1f)
        {
            // 델리게이트 연결
            TargetPS->OnHealthChanged.RemoveDynamic(this, &UAFHealthBarWidget::HandleHealthChanged);
            TargetPS->OnHealthChanged.AddDynamic(this, &UAFHealthBarWidget::HandleHealthChanged);

            // 팀 색상 결정
            FLinearColor TeamColor = (LocalPS->GetTeamID() == TargetPS->GetTeamID()) ? FLinearColor::Blue : FLinearColor::Red;

            // [핵심] 초기 상태를 C++에서 직접 강제 설정
            if (HealthPB)
            {
                HealthPB->SetPercent(CurrHP / MaxHP);
                HealthPB->SetFillColorAndOpacity(TeamColor);
            }

            // 블루프린트 이벤트도 필요한 경우를 위해 호출
            OnUpdateTeamVisual(TeamColor);

            bIsInitialized = true;
            GetWorld()->GetTimerManager().ClearTimer(InitializationTimerHandle);

            UE_LOG(LogTemp, Warning, TEXT("[HealthBar] Successfully Bound and UI Set via C++"));
        }
    }
}