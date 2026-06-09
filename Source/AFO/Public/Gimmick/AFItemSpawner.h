// AFItemSpawner.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AFItemSpawner.generated.h"

UCLASS()
class AFO_API AAFItemSpawner : public AActor
{
	GENERATED_BODY()
	
public:
    AAFItemSpawner();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, Category = "Spawn Settings")
    TArray<TSubclassOf<class AAFBuffItem>> ItemClasses;

    // 스폰 포인트 액터
    UPROPERTY(EditAnywhere, Category = "Spawn Settings")
    TArray<TObjectPtr<AActor>> SpawnPoints;

    // 스폰 주기 (30초)
    UPROPERTY(EditAnywhere, Category = "Spawn Settings")
    float SpawnInterval = 30.0f;

    // 한 번에 스폰할 아이템 개수 (2개)
    UPROPERTY(EditAnywhere, Category = "Spawn Settings")
    int32 SpawnCount = 2;

private:
    // 현재 월드에 생성된 아이템들을 관리 (다음 주기 때 제거용)
    UPROPERTY()
    TArray<TObjectPtr<class AAFBuffItem>> CurrentSpawnedItems;

    FTimerHandle SpawnTimerHandle;

    void RefreshItems(); // 기존 아이템 삭제 및 새 아이템 생성 메인 로직
    void ClearExistingItems();
    void SpawnRandomItems();

};
