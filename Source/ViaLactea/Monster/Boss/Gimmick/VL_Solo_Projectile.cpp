#include "Monster/Boss/Gimmick/VL_Solo_Projectile.h"
#include "Components/SphereComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

#include "Base/Data/VFX/VL_VFXDataAsset.h"

#include "CustomLog/CustomLog.h"
// Sets default values
AVL_Solo_Projectile::AVL_Solo_Projectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
    SetReplicateMovement(true);

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
    CollisionComponent->InitSphereRadius(1000.f);
    CollisionComponent->SetCollisionProfileName(TEXT("Projectile"));

    CollisionComponent->SetNotifyRigidBodyCollision(true);

    SetRootComponent(CollisionComponent);

    NiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComp"));
    NiagaraComponent->SetupAttachment(RootComponent);

    NiagaraComponent->bAutoActivate = false;
}

// Called when the game starts or when spawned
void AVL_Solo_Projectile::BeginPlay()
{
	Super::BeginPlay();
	//if (GetNetMode() != NM_DedicatedServer)
	//{
	//	InitCosmetics();
	//}
	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(DamageTimerHandle, this, &AVL_Solo_Projectile::ApplyTickDamage, DamageInterval, true);
		GetWorldTimerManager().SetTimer(LifeSpanTimerHandle, this, &AVL_Solo_Projectile::Explode, LifeSpan, false);
	}

}

void AVL_Solo_Projectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AVL_Solo_Projectile, VFXData);

}

void AVL_Solo_Projectile::InitializeProjectile(UVL_VFXDataAsset* InDataAsset, FGameplayTag InLoopTag, FGameplayTag InExplosionTag)
{
    if (!HasAuthority()) return;

    VFXData.DataAsset = InDataAsset;
    VFXData.Tag = InLoopTag;
    SavedExplosionTag = InExplosionTag;
    // 이전에 구현했던 이펙트 켜는 로직 (InitCosmetics 등) 실행
    if (GetNetMode() != NM_DedicatedServer)
    {
        OnRep_VFXData();
    }
}

void AVL_Solo_Projectile::ApplyTickDamage()
{
    if (!HasAuthority()) return;

    TArray<AActor*> OverlappingActors;
    CollisionComponent->GetOverlappingActors(OverlappingActors);

    for (AActor* Actor : OverlappingActors)
    {
        if (Actor && Actor != this && Actor != GetOwner())
        {
            UGameplayStatics::ApplyDamage(Actor, DamageAmount, GetInstigatorController(), this, nullptr);
        }
    }
}

void AVL_Solo_Projectile::Explode()
{
    if (!HasAuthority()) return;

    GetWorldTimerManager().ClearTimer(DamageTimerHandle);

    // 1. 모든 클라이언트(자신 포함)에게 폭발 이펙트 재생 명령
    Multicast_Explode(SavedExplosionTag);

    CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    float DamageDelay = 1.0f;
    GetWorldTimerManager().SetTimer(ExplosionDamageTimerHandle, this, &AVL_Solo_Projectile::ExecuteExplosionDamage, DamageDelay, false);
}

void AVL_Solo_Projectile::ExecuteExplosionDamage()
{
    if (!HasAuthority()) return;

    GetWorldTimerManager().ClearTimer(ExplosionDamageTimerHandle);

    float ExplosionRadius = CollisionComponent->GetScaledSphereRadius();
    TArray<AActor*> IgnoreActors;
    if (AActor* OwnerActor = GetOwner())
    {
        IgnoreActors.Add(this);
        IgnoreActors.Add(OwnerActor);
    }
    // 실제 데미지 판정 수행
    UGameplayStatics::ApplyRadialDamage(
        this,
        DamageAmount * 5.0f,
        GetActorLocation(),
        ExplosionRadius,
        nullptr,
        IgnoreActors,
        this,
        GetInstigatorController()
    );

    // 모든 처리가 끝났으므로 액터 수명 설정 (VFX가 완전히 끝날 시간 확보)
    SetLifeSpan(1.5f);
}

void AVL_Solo_Projectile::OnRep_VFXData()
{
	InitCosmetics();
}

void AVL_Solo_Projectile::InitCosmetics()
{
    if (VFXData.IsValid())
    {
        const FVLEffectInfo* Info = VFXData.DataAsset->GetEffectInfoByTag(VFXData.Tag);
        if (Info)
        {
            // 지속형 VFX 스폰
            if (UNiagaraSystem* NS = Info->NiagaraSystem.Get())
            {
                NiagaraComponent->SetAsset(NS);
                NiagaraComponent->SetRelativeLocation(Info->SpawnOffset);
                NiagaraComponent->Activate();
            }
            else
            {

                UNiagaraSystem* LoadedNS = Info->NiagaraSystem.LoadSynchronous();
                NiagaraComponent->SetAsset(LoadedNS);
                NiagaraComponent->Activate();
            }

            // 지속형 사운드 재생 (필요 시)
            if (USoundBase* Sound = Info->Sound.Get())
            {
                UGameplayStatics::SpawnSoundAttached(Sound, RootComponent);
            }
            else
            {
                USoundBase* LoadedSound = Info->Sound.LoadSynchronous();
                UGameplayStatics::SpawnSoundAttached(LoadedSound, RootComponent);
            }
        }
    }
}

void AVL_Solo_Projectile::Multicast_Explode_Implementation(FGameplayTag ExplosionTag)
{
    if (NiagaraComponent)
    {
        NiagaraComponent->Deactivate();
    }
    // 2. 전달받은 폭발 태그로 VFX/Sound 불러오기
    if (VFXData.DataAsset && ExplosionTag.IsValid())
    {
        const FVLEffectInfo* Info = VFXData.DataAsset->GetEffectInfoByTag(ExplosionTag);
        if (Info)
        {
            FVector SpawnLoc = GetActorLocation() + Info->SpawnOffset;

            // --- 폭발 나이아가라 스폰 ---
            // 액터가 곧 Destroy 되므로, 액터에 붙이지 않고 독립적으로(AtLocation) 스폰합니다.
            if (UNiagaraSystem* NS = Info->NiagaraSystem.Get())
            {
                UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), NS, SpawnLoc, GetActorRotation());
            }
            else if (!Info->NiagaraSystem.IsNull()) // 아직 로드되지 않은 경우 방어 코드
            {
                UNiagaraSystem* LoadedNS = Info->NiagaraSystem.LoadSynchronous();
                UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), LoadedNS, SpawnLoc, GetActorRotation());
            }

            // --- 폭발 사운드 재생 ---
            if (USoundBase* Sound = Info->Sound.Get())
            {
                UGameplayStatics::PlaySoundAtLocation(this, Sound, SpawnLoc);
            }
            else if (!Info->Sound.IsNull())
            {
                USoundBase* LoadedSound = Info->Sound.LoadSynchronous();
                UGameplayStatics::PlaySoundAtLocation(this, LoadedSound, SpawnLoc);
            }
        }
    }
}

