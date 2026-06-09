#include "UI/AFESCWidget.h"
#include "Components/Button.h"
#include "Kismet/KismetSystemLibrary.h" 
#include "Kismet/GameplayStatics.h" 
#include "Player/AFPlayerController.h"

void UAFESCWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 버튼 이벤트 바인딩 시 유효성 검사 필수
	if (IsValid(Button_Resume))
	{
		Button_Resume->OnClicked.AddDynamic(this, &UAFESCWidget::OnResumeClicked);
	}
	if (IsValid(Button_Option))
	{
		// 옵션 버튼은 현재 기능 없음
		Button_Option->OnClicked.AddDynamic(this, &UAFESCWidget::OnOptionClicked);
	}
	if (IsValid(Button_Exit))
	{
		Button_Exit->OnClicked.AddDynamic(this, &UAFESCWidget::OnExitClicked);
	}
}

void UAFESCWidget::OnResumeClicked()
{
	CloseESCMenu();
}

void UAFESCWidget::CloseESCMenu()
{
	if (AAFPlayerController* AFPC = Cast<AAFPlayerController>(GetOwningPlayer()))
	{
		// 메뉴가 열려있을 때만 닫기 로직을 수행합니다.
		if (AFPC->bIsESCMenuOpen)
		{
			AFPC->ToggleESCMenu();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AAFPlayerController를 가져올 수 없어 ESC 메뉴를 닫을 수 없습니다."));
	}
}

void UAFESCWidget::OnOptionClicked()
{
	// 현재 기능 없음.
	UE_LOG(LogTemp, Log, TEXT("Option 버튼 클릭: 기능 구현 예정"));
}

void UAFESCWidget::OnExitClicked()
{
	// 서버 연결 해제 및 타이틀 화면으로 복귀
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->ClientTravel("/Game/01_ArenaFighter/01_Levels/TitleMenu", ETravelType::TRAVEL_Absolute);

	}

	// 게임을 완전히 종료
	// UKismetSystemLibrary::QuitGame(GetWorld(), GetOwningPlayer(), EQuitPreference::Quit, false);
}

