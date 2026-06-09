// AFItemSpawner.cpp


#include "Gimmick/AFItemSpawner.h"
#include "Gimmick/AFBuffItem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"


AAFItemSpawner::AAFItemSpawner()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
}

void AAFItemSpawner::BeginPlay()
{
    Super::BeginPlay();
    if (HasAuthority())
    {
        GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AAFItemSpawner::RefreshItems, SpawnInterval, true);
        // 시작하자마자 첫 스폰
        RefreshItems();
    }
}

void AAFItemSpawner::RefreshItems()
{
    ClearExistingItems();
    SpawnRandomItems();
}


void AAFItemSpawner::ClearExistingItems()
{
    for (auto& Item : CurrentSpawnedItems)
    {
        if (IsValid(Item))
        {
            Item->Destroy();
        }
    }
    CurrentSpawnedItems.Empty();
}


void AAFItemSpawner::SpawnRandomItems()
{
    // 방어적 코드: 등록된 클래스나 포인트가 부족하면 실행 안 함
    if (ItemClasses.Num() == 0 || SpawnPoints.Num() < SpawnCount)
    {
        UE_LOG(LogTemp, Warning, TEXT("AFItemSpawner: 아이템 클래스나 스폰 포인트가 부족합니다!"));
        return;
    }

    // 이번 턴에 사용할 포인트들과 클래스들을 복사 (비복원 추출을 위함)
    TArray<TObjectPtr<AActor>> AvailablePoints = SpawnPoints;
    TArray<TSubclassOf<AAFBuffItem>> AvailableClasses = ItemClasses;

    for (int32 i = 0; i < SpawnCount; ++i)
    {
        if (AvailablePoints.Num() == 0 || AvailableClasses.Num() == 0) break;

        // 1. 랜덤 포인트 선택 및 제거 (중복 위치 방지)
        int32 PointIdx = FMath::RandRange(0, AvailablePoints.Num() - 1);
        AActor* SelectedPoint = AvailablePoints[PointIdx];
        AvailablePoints.RemoveAt(PointIdx);

        // 2. 랜덤 아이템 클래스 선택 및 제거 (중복 아이템 종류 방지)
        int32 ClassIdx = FMath::RandRange(0, AvailableClasses.Num() - 1);
        TSubclassOf<AAFBuffItem> SelectedClass = AvailableClasses[ClassIdx];
        AvailableClasses.RemoveAt(ClassIdx);

        // 3. 스폰 (타겟 포인트의 위치와 회전값 그대로 사용)
        if (SelectedPoint && SelectedClass)
        {
            FVector Location = SelectedPoint->GetActorLocation();
            FRotator Rotation = SelectedPoint->GetActorRotation();

            FActorSpawnParameters Params;
            Params.Owner = this;
            Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

            AAFBuffItem* NewItem = GetWorld()->SpawnActor<AAFBuffItem>(SelectedClass, Location, Rotation, Params);
            if (NewItem)
            {
                CurrentSpawnedItems.Add(NewItem);
            }
        }
    }
}

