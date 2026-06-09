// AFBuffItem.cpp

#include "Gimmick/AFBuffItem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Character/AFPlayerCharacter.h"
#include "Player/AFPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Components/AFAttributeComponent.h"


AAFBuffItem::AAFBuffItem()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	SetRootComponent(CollisionComponent);
	CollisionComponent->SetSphereRadius(50.f);

	// 서버에서만 Overlap 이벤트를 실행하도록 설정
	CollisionComponent->SetCollisionProfileName(TEXT("Trigger"));
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AAFBuffItem::OnOverlapBegin);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	BuffEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("BuffEffect"));
	BuffEffect->SetupAttachment(RootComponent);
}

void AAFBuffItem::BeginPlay()
{
	Super::BeginPlay();

	// 나이아가라 시스템 안의 'Color' 파라미터 이름을 맞춰야 함 (예: "UserColor")
	if (BuffEffect)
	{
		BuffEffect->SetNiagaraVariableLinearColor(TEXT("User.BaseColor"), BuffColor);
	}
}



void AAFBuffItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// 아이템이 둥둥 떠있거나 회전하는 연출 (클라에서도 돌아감)
	AddActorLocalRotation(FRotator(0.f, 100.f * DeltaTime, 0.f));
}

void AAFBuffItem::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // 1. 서버 권한 및 대상 유효성 검사
    if (!HasAuthority() || !OtherActor) return;

    // 2. 캐릭터 캐스팅
    AAFPlayerCharacter* TargetCharacter = Cast<AAFPlayerCharacter>(OtherActor);
    if (!TargetCharacter) return;

    // 3. PlayerState 가져오기 (체력/마나 실제 데이터는 PS에 있으므로)
    AAFPlayerState* PS = TargetCharacter->GetPlayerState<AAFPlayerState>();
    if (!PS) return;

    // 4. 버프 타입에 따른 로직 수행
    switch (BuffType)
    {
    case EBuffType::Heal:
    {
        // 체력 회복 (BuffValue만큼, 최대치 제한)
        float NewHealth = FMath::Clamp(PS->GetCurrentHealth() + HealAmount, 0.0f, PS->GetMaxHealth());
        PS->SetHealth(NewHealth, PS->GetMaxHealth());
        
        // 마나 회복 (Heal 아이템은 마나도 같이 채워줌)
        PS->AddMana(HealAmount);

        UE_LOG(LogTemp, Log, TEXT("[BUFF] %s Healed: HP/MP +%f"), *PS->GetPlayerName(), HealAmount);
    }
    break;

    case EBuffType::Attack:
    {
        UE_LOG(LogTemp, Log, TEXT("Overlap: Attack Buff Item Picked Up!")); // 이 로그가 뜨는지 확인
        if (UAFAttributeComponent* Attr = TargetCharacter->FindComponentByClass<UAFAttributeComponent>())
        {
            Attr->ApplyAttackBuff(AttackMultiplier, BuffDuration);
            Attr->Multicast_ApplyAura(AuraEffect, BuffColor, BuffDuration);
            UE_LOG(LogTemp, Log, TEXT("Overlap: Attack Buff Item Adapted!")); // 이 로그가 뜨는지 확인
        }
    }
    break;

    case EBuffType::Speed:
    {
        UAFAttributeComponent* Attr = TargetCharacter->FindComponentByClass<UAFAttributeComponent>();
        TargetCharacter->ApplySpeedBuff(SpeedMultiplier, BuffDuration); // 10초간 BuffValue배 만큼 빨라짐
        Attr->Multicast_ApplyAura(AuraEffect, BuffColor, BuffDuration);
        UE_LOG(LogTemp, Log, TEXT("[BUFF] %s Speed Boosted!"), *PS->GetPlayerName());
    }
    break;
    }

    // 5. 시각적 피드백 및 파괴
    // 서버에서 Destroy()하면 연결된 클라이언트들에서도 자동으로 사라집니다.
    Multicast_PlayPickupEffects();
    Destroy();
}

void AAFBuffItem::Multicast_PlayPickupEffects_Implementation()
{
    if (PickupEffect) // 에디터에서 할당한 나이아가라 시스템
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), PickupEffect, GetActorLocation());
    }

    if (PickupSound) // 에디터에서 할당한 USoundBase
    {
        UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());
    }
}
