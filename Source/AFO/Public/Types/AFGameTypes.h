// AFGameTypes.h

#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "AFGameTypes.generated.h" 

//UENUM(BlueprintType)
//enum class EAFCharacterType : uint8
//{
//    Mage,
//    DarkKnight,
//    WereWolf,
//    Aurora,
//    Archer
//};

USTRUCT(BlueprintType)
struct FAFSkillInfo : public FTableRowBase
{
    GENERATED_BODY()

    // --- 기본 정보 ---
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill | Basic")
    FText SkillName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill | Basic")
    FText SkillDescription;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill | Basic")
    TObjectPtr<UTexture2D> SkillIcon;

    // --- 전투 수치 ---
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill | Stats")
    float Damage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill | Stats")
    float ManaCost;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill | Stats")
    float SkillRange; // 공격 범위

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill | Stats")
    float Cooldown;
};



USTRUCT(BlueprintType)
struct FAFPlayerCharacterStatRow : public FTableRowBase
{
    GENERATED_BODY()

public:
    FAFPlayerCharacterStatRow() : Maxhp(100.f), Attack(10.f), MoveSpeed(600.f) {}

    // --- 생존 관련 ---
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat | Vitality")
    float Maxhp;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat | Vitality")
    float Maxmana;

    // --- 전투 관련 ---
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat | Combat")
    float Attack;

    // --- 기동성 관련 ---
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat | Mobility")
    float MoveSpeed;

};
