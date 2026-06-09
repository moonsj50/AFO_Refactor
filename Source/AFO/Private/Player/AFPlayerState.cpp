// AFPlayerState.cpp


#include "Player/AFPlayerState.h"
#include "GameFramework/PlayerController.h"
#include "Player/AFPlayerController.h"
#include "Net/UnrealNetwork.h"
#include "Game/AFLobbyGameState.h"

AAFPlayerState::AAFPlayerState()
{
	MaxHealth = 100.0f;
	MaxMana = 1000.0f;
	CurrentHealth = MaxHealth;
	CurrentMana = MaxMana;
	KillCount = 0;
	DeathCount = 0;
	TeamID = 0; // Default RED
	TeamIndex = 1; // Default Index 1
	bIsDead = false;
}

void AAFPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAFPlayerState, CurrentHealth);
	DOREPLIFETIME(AAFPlayerState, MaxHealth);
	DOREPLIFETIME(AAFPlayerState, CurrentMana);
	DOREPLIFETIME(AAFPlayerState, MaxMana);
	DOREPLIFETIME(AAFPlayerState, KillCount);
	DOREPLIFETIME(AAFPlayerState, DeathCount);
	DOREPLIFETIME(AAFPlayerState, TeamID);
	DOREPLIFETIME(AAFPlayerState, TeamIndex);
	DOREPLIFETIME(AAFPlayerState, bIsDead);
	DOREPLIFETIME(AAFPlayerState, SelectedCharacterId);
	DOREPLIFETIME(AAFPlayerState, bReady);
	DOREPLIFETIME(AAFPlayerState, SelectedCharacterName);
}

void AAFPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	if (AAFPlayerState* NewPS = Cast<AAFPlayerState>(PlayerState))
	{
		NewPS->SetPlayerName(GetPlayerName());
		NewPS->TeamID = TeamID;
		NewPS->TeamIndex = TeamIndex;
		NewPS->SelectedCharacterId = SelectedCharacterId;
		NewPS->SelectedCharacterName = SelectedCharacterName;
		NewPS->bReady = bReady;

		NewPS->KillCount = KillCount;
		NewPS->DeathCount = DeathCount;
	}
}


void AAFPlayerState::OnRep_PlayerName()
{
	Super::OnRep_PlayerName();
	OnPlayerNameChanged.Broadcast(this);
}

void AAFPlayerState::OverrideWith(APlayerState* PlayerState)
{
	Super::OverrideWith(PlayerState);

	if (AAFPlayerState* OldPS = Cast<AAFPlayerState>(PlayerState))
	{
		TeamID = OldPS->TeamID;
		TeamIndex = OldPS->TeamIndex;

		SelectedCharacterId = OldPS->SelectedCharacterId;
		bReady = OldPS->bReady;
	}
}

// =========================
// OnRep 함수 구현
// =========================
void AAFPlayerState::OnRep_CurrentHealth()
{
			OnHealthChanged.Broadcast(CurrentHealth, MaxHealth, this);
}

void AAFPlayerState::OnRep_CurrentMana()
{
	OnManaChanged.Broadcast(CurrentMana, MaxMana, this);
}

void AAFPlayerState::OnRep_KillCount()
{
	OnKillCountChanged.Broadcast(KillCount, this);
}

void AAFPlayerState::OnRep_DeathCount()
{
	OnDeathCountChanged.Broadcast(DeathCount, this);
}

void AAFPlayerState::OnRep_IsDead()
{
	// 여기다가 UI 갱신
}

void AAFPlayerState::OnRep_SelectedCharacter()
{
	OnSelectedCharacterChanged.Broadcast(this);
}

void AAFPlayerState::OnRep_Ready()
{

}

// =========================
// Setter 구현
// =========================

void AAFPlayerState::SetHealth(float NewHealth, float NewMaxHealth)
{
	if (!HasAuthority()) return;

	// 값이 실제로 변경되었는지 확인
	bool bHealthChanged = (CurrentHealth != NewHealth || MaxHealth != NewMaxHealth);

	CurrentHealth = FMath::Clamp(NewHealth, 0.f, NewMaxHealth);
	MaxHealth = NewMaxHealth;

	// 서버에서 UI를 갱신하기 위해 수동으로 OnRep 함수 호출
	if (bHealthChanged)
	{
		OnRep_CurrentHealth();
		// 클라이언트들은 엔진의 복제 시스템에 의해 OnRep_CurrentHealth가 호출됩니다.
	}
}

void AAFPlayerState::SetMana(float NewMana, float NewMaxMana)
{
	if (!HasAuthority()) return;

	bool bManaChanged = (CurrentMana != NewMana || MaxMana != NewMaxMana);

	CurrentMana = FMath::Clamp(NewMana, 0.f, NewMaxMana);
	MaxMana = NewMaxMana;

	if (bManaChanged)
	{
		OnRep_CurrentMana();
	}
}

void AAFPlayerState::IncrementKillCount()
{
	if (!HasAuthority()) return;

	KillCount++;
	// 서버에서 UI 갱신을 위해 수동 호출
	OnRep_KillCount();
}

void AAFPlayerState::IncrementDeathCount()
{
	if (!HasAuthority()) return;

	DeathCount++;
	// 서버에서 UI 갱신을 위해 수동 호출
	OnRep_DeathCount();
}

void AAFPlayerState::OnRep_TeamInfo()
{
	OnTeamInfoChanged.Broadcast(this);

	if (UWorld* World = GetWorld())
	{
		if (AAFLobbyGameState* LGS = World->GetGameState<AAFLobbyGameState>())
		{
			LGS->OnCountsChanged.Broadcast();
		}
	}
}

void AAFPlayerState::SetTeamInfo(uint8 NewTeamID, uint8 NewTeamIndex)
{
	if (!HasAuthority()) return;

	TeamID = NewTeamID;
	TeamIndex = NewTeamIndex;

	ForceNetUpdate();

	OnRep_TeamInfo();
}


void AAFPlayerState::SetDead(bool bNewDead)
{
	if (!HasAuthority()) return;

	if (bIsDead == bNewDead)
	{
		return;
	}

	bIsDead = bNewDead;

	// 서버에서도 즉시 반응이 필요하면 호출하라 하네요
	OnRep_IsDead();
}

void AAFPlayerState::ResetForRespawn()
{
	if (!HasAuthority()) return;

	SetDead(false);
	SetHealth(MaxHealth, MaxHealth);
	SetMana(MaxMana, MaxMana);
}

void AAFPlayerState::SetSelectedCharacter_Server(uint8 InId)
{
	if (!HasAuthority()) return;
	SelectedCharacterId = InId;
	OnRep_SelectedCharacter();
}

void AAFPlayerState::SetReady_Server(bool bNewReady)
{
	if (!HasAuthority()) return;
	bReady = bNewReady;
	OnRep_Ready();
}

void AAFPlayerState::ResetLobbySelection_Server()
{
	if (!HasAuthority()) return;
	SelectedCharacterId = 255;
	bReady = false;
	OnRep_SelectedCharacter();
	OnRep_Ready();
}

FText AAFPlayerState::GetSelectedCharacterName() const
{
	if (!SelectedCharacterName.IsEmpty())
	{
		return SelectedCharacterName;
	}

	if (HasSelectedCharacter() && CharacterDisplayNames.IsValidIndex(SelectedCharacterId))
	{
		return CharacterDisplayNames[SelectedCharacterId];
	}

	return FText::FromString(TEXT("Unknown"));
}

void AAFPlayerState::AddMana(float Amount)
{
	if (!HasAuthority()) return;

	SetMana(CurrentMana + Amount, MaxMana);
}

bool AAFPlayerState::ConsumeMana(float Amount)
{
	if (!HasAuthority()) return false;

	if (CurrentMana < Amount)
	{
		return false;
	}

	SetMana(CurrentMana - Amount, MaxMana);
	return true;
}

void AAFPlayerState::OnRep_SelectedCharacterName()
{
	OnSelectedCharacterChanged.Broadcast(this);
}


void AAFPlayerState::SetSelectedCharacterName_Server(const FText& InName)
{
	if (!HasAuthority()) return;
	SelectedCharacterName = InName;
	OnRep_SelectedCharacterName();
}