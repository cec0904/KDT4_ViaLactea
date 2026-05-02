#include "Monster/Boss/Gimmick/VL_LightningProjectile.h"

#include "Base/Data/VFX/VL_VFXDataAsset.h"
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "Engine/OverlapResult.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

#include "CustomLog/CustomLog.h"

// Sets default values
AVL_LightningProjectile::AVL_LightningProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true; // 멀티플레이어 복제 활성화

	DamageSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DamageSphere"));
	RootComponent = DamageSphere;

	// 기본 충돌 설정
	DamageSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DamageSphere->SetCollisionObjectType(ECC_WorldDynamic);
	DamageSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	DamageSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	StrikeVFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("StrikeVFX"));
	StrikeVFX->SetupAttachment(RootComponent);
	StrikeVFX->bAutoActivate = false;
}

void AVL_LightningProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AVL_LightningProjectile, VFXData);
}

void AVL_LightningProjectile::InitializeStrike(UVL_VFXDataAsset* InAsset, FGameplayTag InTag, float InDamage)
{
	if (!HasAuthority()) return;

	VFXData.DataAsset = InAsset;
	VFXData.Tag = InTag;
	StrikeDamage = InDamage;
    OnRep_VFXData();

    ApplyInstantDamage();
}

void AVL_LightningProjectile::OnRep_VFXData()
{
    UpdateProjectileState();
}

void AVL_LightningProjectile::UpdateProjectileState()
{
    if (!VFXData.IsValid()) return;

    const FVLEffectInfo* Info = VFXData.DataAsset->GetEffectInfoByTag(VFXData.Tag);
    if (!Info) return;

    if (DamageSphere)
    {
        float TargetRadius = Info->CollisionRadius;

        DamageSphere->SetSphereRadius(TargetRadius);

        // 중요: 반지름 설정 후 즉시 물리 엔진에 업데이트 알림
        DamageSphere->UpdateOverlaps();
    }

    // 2. 이펙트 설정
    if (UNiagaraSystem* NS = Info->NiagaraSystem.Get())
    {
        StrikeVFX->SetAsset(NS);
        StrikeVFX->Activate();
    }
    else // 소프트 포인터가 로드되지 않았을 경우 (가급적 프리로딩 권장)
    {
        StrikeVFX->SetAsset(Info->NiagaraSystem.LoadSynchronous());
        StrikeVFX->Activate();
    }

    // 3. 사운드 재생
    USoundBase* Sound = Info->Sound.Get();
    if (!Sound) Sound = Info->Sound.LoadSynchronous();
    if (Sound)
    {
        UGameplayStatics::SpawnSoundAttached(Sound, RootComponent);
    }
}

void AVL_LightningProjectile::BeginPlay()
{
    Super::BeginPlay();

    
}

void AVL_LightningProjectile::ApplyInstantDamage()
{
    if (!HasAuthority()) return;

    // 1. 현재 설정된 구체의 반지름을 가져옴
    float Radius = DamageSphere->GetScaledSphereRadius();
    FVector Center = GetActorLocation();

    // 2. 물리 쿼리를 위한 설정
    TArray<FOverlapResult> Overlaps;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    QueryParams.AddIgnoredActor(GetInstigator()); // 시전자 제외

    // 3. 물리 엔진에 직접 구체 범위 내의 대상을 즉시 검색 요청 (채널은 Pawn)
    bool bHit = GetWorld()->OverlapMultiByChannel(
        Overlaps,
        Center,
        FQuat::Identity,
        ECC_Pawn, // 또는 프로젝트에서 설정한 캐릭터 콜리전 채널
        FCollisionShape::MakeSphere(Radius),
        QueryParams
    );

    if (bHit)
    {
        for (const FOverlapResult& Overlap : Overlaps)
        {
            AActor* Victim = Overlap.GetActor();
            if (Victim)
            {
                UGameplayStatics::ApplyDamage(
                    Victim,
                    StrikeDamage,
                    GetInstigatorController(),
                    this,
                    UDamageType::StaticClass()
                );
            }
        }
    }

    // 4. 후처리
    DamageSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SetLifeSpan(2.0f);
}