// AFPlayerState.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AFPlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerNameChanged, class AAFPlayerState*, ChangedPlayer);

// 체력/마나 변경 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHealthManaChanged, float, CurrentValue, float, MaxValue, class AAFPlayerState*, ChangedPlayer);

// 킬/데스 변경 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStatCountChanged, int32, NewValue, class AAFPlayerState*, ChangedPlayer);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTeamInfoChanged, AAFPlayerState*, ChangedPlayer);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSelectedCharacterChanged, AAFPlayerState*, ChangedPlayer);
UCLASS()
class AFO_API AAFPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AAFPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void CopyProperties(APlayerState* PlayerState) override;
	virtual void OverrideWith(APlayerState* PlayerState) override;
	UPROPERTY(BlueprintAssignable)
	FOnPlayerNameChanged OnPlayerNameChanged;

	virtual void OnRep_PlayerName() override;

	UPROPERTY(BlueprintAssignable)
	FOnTeamInfoChanged OnTeamInfoChanged;

	UPROPERTY(BlueprintAssignable)
	FOnSelectedCharacterChanged OnSelectedCharacterChanged;

protected:
	UPROPERTY(Replicated)
	float MaxHealth;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth)
	float CurrentHealth; 

	UPROPERTY(Replicated, EditDefaultsOnly, Category = "Mana")
	float MaxMana = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentMana)
	float CurrentMana;

	UPROPERTY(ReplicatedUsing = OnRep_KillCount)
	int32 KillCount;

	UPROPERTY(ReplicatedUsing = OnRep_DeathCount)
	int32 DeathCount;

	UPROPERTY(ReplicatedUsing = OnRep_TeamInfo)
	uint8 TeamID;

	UPROPERTY(ReplicatedUsing = OnRep_TeamInfo)
	uint8 TeamIndex;


	UFUNCTION()
	void OnRep_TeamInfo();

	UPROPERTY(ReplicatedUsing = OnRep_IsDead)
	bool bIsDead = false;

	UPROPERTY(ReplicatedUsing = OnRep_SelectedCharacter) 
	uint8 SelectedCharacterId = 255;

	UPROPERTY(ReplicatedUsing = OnRep_Ready)
	bool bReady = false;

	UFUNCTION()
	void OnRep_CurrentHealth();
	UFUNCTION()
	void OnRep_CurrentMana();
	UFUNCTION()
	void OnRep_KillCount();
	UFUNCTION()
	void OnRep_DeathCount();

	UFUNCTION()
	void OnRep_IsDead();

	UFUNCTION() 
	void OnRep_SelectedCharacter();

	UFUNCTION()
	void OnRep_Ready();


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AFO|Character")
	TArray<FText> CharacterDisplayNames;

//  ===============================
//  Public API (Getter / Setter)
// ================================

public:
	// 델리게이트 이벤트
	FOnHealthManaChanged OnHealthChanged;
	FOnHealthManaChanged OnManaChanged;
	FOnStatCountChanged OnKillCountChanged;
	FOnStatCountChanged OnDeathCountChanged;


	// Getter
	float GetCurrentHealth() const { return CurrentHealth; }
	float GetMaxHealth() const { return MaxHealth; }
	float GetCurrentMana() const { return CurrentMana; }
	float GetMaxMana() const { return MaxMana; }
	UFUNCTION(BlueprintPure, Category = "AFO|Stats")
	int32 GetKillCount() const { return KillCount; }
	UFUNCTION(BlueprintPure, Category = "AFO|Stats")
	int32 GetDeathCount() const { return DeathCount; }
	UFUNCTION(BlueprintPure, Category = "AFO|Stats")
	uint8 GetTeamID() const { return TeamID; }
	UFUNCTION(BlueprintPure, Category = "AFO|Stats")
	uint8 GetTeamIndex() const { return TeamIndex; }

	bool IsDead() const { return bIsDead; }

	bool HasSelectedCharacter() const { return SelectedCharacterId != 255; }
	uint8 GetSelectedCharacterId() const { return SelectedCharacterId; }
	bool IsReady() const { return bReady; }

	//Setter (서버전용)
	void SetHealth(float NewHealth, float NewMaxHealth);
	void SetMana(float NewMana, float NewMaxMana);
	void IncrementKillCount();
	void IncrementDeathCount();
	void SetTeamInfo(uint8 NewTeamID, uint8 NewTeamIndex);

	void SetDead(bool bNewDead);
	void ResetForRespawn();


	void SetSelectedCharacter_Server(uint8 InId);
	void SetReady_Server(bool bNewReady);
	void ResetLobbySelection_Server();

	// 추가 함수

	void AddMana(float Amount);
	bool ConsumeMana(float Amount);
	UFUNCTION(BlueprintPure, Category = "AFO|Character")
	FText GetSelectedCharacterName() const;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_SelectedCharacterName)
	FText SelectedCharacterName;

	UFUNCTION()
	void OnRep_SelectedCharacterName();

public:
	void SetSelectedCharacterName_Server(const FText& InName);

};