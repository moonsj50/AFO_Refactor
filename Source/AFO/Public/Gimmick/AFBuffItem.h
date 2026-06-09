// AFBuffItem.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AFBuffItem.generated.h"

class UNiagaraComponent;

UENUM(BlueprintType)
enum class EBuffType : uint8
{
	Heal      UMETA(DisplayName = "Heal"),
	Attack UMETA(DisplayName = "Attack"),
	Speed       UMETA(DisplayName = "Speed")
};

UCLASS()
class AFO_API AAFBuffItem : public AActor
{
	GENERATED_BODY()
	
public:	

	AAFBuffItem();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class USphereComponent* CollisionComponent;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	class UStaticMeshComponent* MeshComponent;

	UPROPERTY(EditAnywhere, Category = "Buff")
	EBuffType BuffType;

	// 각각의 버프 수치와 버프 지속 시간
	UPROPERTY(EditAnywhere, Category = "Buff | Settings")
	float HealAmount = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Buff | Settings")
	float AttackMultiplier = 1.5f;

	UPROPERTY(EditAnywhere, Category = "Buff | Settings")
	float SpeedMultiplier = 1.3f;

	UPROPERTY(EditAnywhere, Category = "Buff | Settings")
	float BuffDuration = 10.0f;

	UPROPERTY(VisibleAnywhere, Category = "Effects")
	UNiagaraComponent* BuffEffect;

	UPROPERTY(EditAnywhere, Category = "Effects")
	FLinearColor BuffColor;

	// 서버에서만 충돌을 처리하도록 설정
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// 나이아가라 시스템을 담을 변수
	UPROPERTY(EditAnywhere, Category = "Effects")
	class UNiagaraSystem* PickupEffect;

	// 캐릭터에게 지속적으로 붙어있을 오오라 이펙트
	UPROPERTY(EditAnywhere, Category = "Effects")
	class UNiagaraSystem* AuraEffect;

	// 사운드 파일을 담을 변수
	UPROPERTY(EditAnywhere, Category = "Effects")
	class USoundBase* PickupSound;

	UFUNCTION(NetMulticast,Reliable)
	void Multicast_PlayPickupEffects();
public:
	// 아이템 회전 등 연출용 Tick
	virtual void Tick(float DeltaTime) override;
	
	
};
