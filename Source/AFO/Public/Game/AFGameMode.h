#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "AFGameMode.generated.h"

class AAFGameState;
class APlayerController;
class AController;

UCLASS()
class AFO_API AAFGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AAFGameMode();

	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
	virtual void HandleSeamlessTravelPlayer(AController*& C) override;

	void ReportKill(AController* KillerController);
	void EndRound();

	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	void HandlePlayerDeath(AController* VictimController, AController* KillerController);

private:
	int32 RoundDuration;

	TMap<APlayerController*, uint8> PlayerTeams;
	TMap<TWeakObjectPtr<AController>, FTimerHandle> RespawnTimers;
	void StartRound();
	AAFGameState* GetAFGameState() const;

	float RespawnDelay = 10.0f;







	// 캐릭터 선택 화면 구현 전 임시 캐릭터 설정 함수

//protected:
//	// 첫 번째 플레이어용 캐릭터 클래스 (에디터에서 설정)
//	UPROPERTY(EditAnywhere, Category = "Classes")
//	TSubclassOf<APawn> FirstCharacterClass;
//
//	// 두 번째 플레이어용 캐릭터 클래스
//	UPROPERTY(EditAnywhere, Category = "Classes")
//	TSubclassOf<APawn> SecondCharacterClass;
//
//	// 세 번째 플레이어용 캐릭터 클래스
//	UPROPERTY(EditAnywhere, Category = "Classes")
//	TSubclassOf<APawn> ThirdCharacterClass;
//
//	// 네 번째 플레이어용 캐릭터 클래스
//	UPROPERTY(EditAnywhere, Category = "Classes")
//	TSubclassOf<APawn> FourthCharacterClass;
//
//	// 현재 접속한 인원수를 체크하기 위한 변수
//	int32 PlayerCount = 0;

public:
	// 플레이어가 스폰될 클래스를 결정하는 언리얼 엔진 엔진 함수 오버라이드
	//virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;




	// 게임 시작을 알리는 함수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Battle")
	int32 StartDelayTime = 5;

	UPROPERTY(EditDefaultsOnly, Category = "Character")
	TArray<TSubclassOf<APawn>> CharacterPawnClasses;

	FTimerHandle TimerHandle_GameStart;
};