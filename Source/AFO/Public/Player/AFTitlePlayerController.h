#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AFTitlePlayerController.generated.h"

UCLASS()
class AFO_API AAFTitlePlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    //void JoinServer();
    void JoinServer(const FString& InIPAddress, const FString& InPlayerName);

//protected:
//    UFUNCTION(Server, Reliable)
//    void ServerTravelToBattleZone();

private:
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = ASUIPlayerController, Meta = (AllowPrivateAccess))
    TSubclassOf<UUserWidget> UIWidgetClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = ASUIPlayerController, Meta = (AllowPrivateAccess))
    TObjectPtr<UUserWidget> UIWidgetInstance;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BGM", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USoundBase> TitleBGM;

    UPROPERTY()
    TObjectPtr<UAudioComponent> TitleBGMComponent;

};