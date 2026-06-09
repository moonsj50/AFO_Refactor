// AFSkillComponent.cpp


#include "Components/AFSkillComponent.h"
#include "Net/UnrealNetwork.h"


UAFSkillComponent::UAFSkillComponent()
{
    PrimaryComponentTick.bCanEverTick = false; 
    SetIsReplicatedByDefault(true);
}

void UAFSkillComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    // CooldownDataArray를 클라이언트로 전송합니다.
    DOREPLIFETIME(UAFSkillComponent, CooldownDataArray);
}
bool UAFSkillComponent::CanUseSkill(FName SkillRowName) const
{
    // 서버/클라이언트 모두에서 CooldownDataArray를 통해 확인 가능하도록 수정
    for (const auto& Info : CooldownDataArray)
    {
        if (Info.SkillRowName == SkillRowName)
        {
            // 종료 시간이 현재 시간보다 뒤에 있다면 아직 쿨타임 중
            return GetWorld()->GetTimeSeconds() >= Info.ServerEndTime;
        }
    }
    return true;
}

void UAFSkillComponent::StartCooldown(FName SkillRowName, float CooldownTime)
{
    // 서버가 아니면 실행 금지
    if (!GetOwner()->HasAuthority()) return;

    // 서버의 현재 시간 기준 종료 시점 계산
    float EndTime = GetWorld()->GetTimeSeconds() + CooldownTime;

    int32 FoundIdx = INDEX_NONE;
    for (int32 i = 0; i < CooldownDataArray.Num(); ++i)
    {
        if (CooldownDataArray[i].SkillRowName == SkillRowName)
        {
            FoundIdx = i;
            break;
        }
    }

    if (FoundIdx != INDEX_NONE)
    {
        CooldownDataArray[FoundIdx].ServerEndTime = EndTime;
        CooldownDataArray[FoundIdx].MaxCooldown = CooldownTime;
    }
    else
    {
        FAFCooldownInfo NewInfo;
        NewInfo.SkillRowName = SkillRowName;
        NewInfo.ServerEndTime = EndTime;
        NewInfo.MaxCooldown = CooldownTime;
        CooldownDataArray.Add(NewInfo);
    }

    // [참고] 서버 전용 타이머가 필요하다면 여기서 OnCooldownFinished를 호출하도록 셋팅
    // 하지만 UI 동기화는 CooldownDataArray 리플리케이션만으로도 충분합니다.

    UE_LOG(LogTemp, Warning, TEXT("@@@ [Server] Cooldown Sync Started: %s (Time: %.1fs)"), *SkillRowName.ToString(), CooldownTime);
}

float UAFSkillComponent::GetCooldownRemainingRatio(FName SkillRowName) const
{
    for (const auto& Info : CooldownDataArray)
    {
        if (Info.SkillRowName == SkillRowName)
        {
            float Remaining = Info.ServerEndTime - GetWorld()->GetTimeSeconds();
            if (Info.MaxCooldown <= 0.f) return 0.f;
            return FMath::Clamp(Remaining / Info.MaxCooldown, 0.f, 1.f);
        }
    }
    return 0.f;
}

float UAFSkillComponent::GetRemainingTime(FName SkillRowName) const
{
    for (const auto& Info : CooldownDataArray)
    {
        if (Info.SkillRowName == SkillRowName)
        {
            // 클라이언트에서도 현재 서버 동기화 시간을 기준으로 계산 가능
            float Remaining = Info.ServerEndTime - GetWorld()->GetTimeSeconds();
            return FMath::Max(Remaining, 0.f);
        }
    }
    return 0.f;
}

void UAFSkillComponent::OnCooldownFinished(FName SkillRowName)
{
    UE_LOG(LogTemp, Log, TEXT("Skill Cooldown Finished: %s"), *SkillRowName.ToString());
}
