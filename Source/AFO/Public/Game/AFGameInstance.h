#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "AFGameInstance.generated.h"

UCLASS()
class AFO_API UAFGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "AFO|Login")
	FString PendingPlayerName;

	UPROPERTY(BlueprintReadWrite, Category = "AFO|Login")
	FString PendingServerIP;
};
