#include "VL_MeteorProjectile.h"

#include "Base/Data/VFX/VL_VFXDataAsset.h"
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/MainCharacterBase.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraFunctionLibrary.h"
//#include "DrawDebugHelpers.h"  // 디버그용 헤더

#include "CustomLog/CustomLog.h"

AVL_MeteorProjectile::AVL_MeteorProjectile()
{
    PrimaryActorTick.bCanEverTick = false;

    bReplicates = true;
    SetReplicateMovement(true);

    // 1. 충돌체 설정 (루트)
    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
    CollisionComp->SetCollisionProfileName(TEXT("Projectile"));

    // 바닥 충돌 범위가 크므로 이펙트와 싱크 맞추기 위해서 100을 올려줌.
    CollisionComp->SetNotifyRigidBodyCollision(true);

    RootComponent = CollisionComp;

    NiagaraComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComp"));
    NiagaraComp->SetupAttachment(RootComponent);
    NiagaraComp->bAutoActivate = false;
   // NiagaraComp->SetVariableBool(FName("User.bIsExploded"), true);

    // 3. 발사체 이동 컴포넌트
    ProjectileMovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMove"));
    ProjectileMovementComp->UpdatedComponent = CollisionComp;
    ProjectileMovementComp->InitialSpeed = 0.f;
    ProjectileMovementComp->ProjectileGravityScale = 0.0f;
}



void AVL_MeteorProjectile::BeginPlay()
{
    Super::BeginPlay();
    //CollisionComp->SetHiddenInGame(false);
    // CollisionComp->IgnoreActorWhenMoving(this, true);

    // 궤적 충돌 감지 시작 (서버에서만)
    if (HasAuthority())
    {
        CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &AVL_MeteorProjectile::OnMeteorOverlap);

        // 바닥(월드 정적 물체)에 닿았을 때 폭발하기 위해 Hit 이벤트 바인딩
        CollisionComp->OnComponentHit.AddDynamic(this, &AVL_MeteorProjectile::OnMeteorHit);
    }

    this->SetLifeSpan(10.0f);
}


void AVL_MeteorProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AVL_MeteorProjectile, VFXData);
    DOREPLIFETIME(AVL_MeteorProjectile, SphereSize);
    DOREPLIFETIME(AVL_MeteorProjectile, IsVFXMoving);
}

void AVL_MeteorProjectile::InitializeProjectile(const UVL_VFXDataAsset* InDataAsset, FGameplayTag InTag, float InDamage, float InRadius, FVector InVelocity, FVector InSpawnOffset, bool InIsVFXMoving)
{
    if (!HasAuthority()) return;

    // 구조체 데이터 채우기
    VFXData.DataAsset = const_cast<UVL_VFXDataAsset*>(InDataAsset);
    VFXData.Tag = InTag;

    this->DamageAmount = InDamage;
    this->SphereSize = InRadius;

    if (CollisionComp)
    {
        CollisionComp->SetSphereRadius(SphereSize);
    }
    if (InIsVFXMoving)
    {
        IsVFXMoving = InIsVFXMoving;
        ProjectileMovementComp->Velocity = FVector(-300.f, 0.f, -500.f);
    }
    else
    {
        ProjectileMovementComp->Velocity = InVelocity;
    }
    ProjectileMovementComp->UpdateComponentVelocity();

    // 서버에서는 OnRep이 자동으로 호출되지 않으므로 시각 효과를 위해 직접 호출
    OnRep_VFXData();

    // 강제 네트워크 업데이트 (데이터가 즉시 복제되도록 유도)
    ForceNetUpdate();

}

// 1. 궤적 충돌 (공중에서 지나가다 맞았을 때)
void AVL_MeteorProjectile::OnMeteorOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!HasAuthority()) return;

    if (!OtherActor || OtherActor == this || OtherActor == GetInstigator())
    {
        return;
    }

    if (AMainCharacterBase* Player = Cast<AMainCharacterBase>(OtherActor))
    {
        
        // 이미 이 운석에 맞은 적이 없다면
        if (!OverlappedActors.Contains(Player))
        {
            UGameplayStatics::ApplyDamage(OtherActor, DamageAmount, GetInstigatorController(), this, nullptr);
            OverlappedActors.Add(Player);

            FVector ImpactLocation = GetActorLocation();

            // 3. 멀티캐스트로 이펙트 재생
            const FVLEffectInfo* Info = VFXData.DataAsset->GetEffectInfoByTag(VFXData.Tag);
            if (Info && !Info->ImpactNiagara.IsNull())
            {
                Multicast_PlayImpactVFX(ImpactLocation);
            }

            DeleteProjectile();

        }
    }
}

// 2. 바닥 충돌 (바닥에 닿는 순간)
void AVL_MeteorProjectile::OnMeteorHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!HasAuthority() || bHasExploded) return;

    if (OtherActor == nullptr || OtherActor == this || OtherActor->IsA(AVL_MeteorProjectile::StaticClass()))
    {
        return;
    }


    // 2. 폭발 범위(100.f) 내 데미지 판정
    TArray<AActor*> IgnoredActors;
    IgnoredActors.Add(this);
    IgnoredActors.Add(GetOwner());


    // 100.0f 반지름으로 원형 데미지 적용
    TArray<AActor*> NearbyMeteors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AVL_MeteorProjectile::StaticClass(), NearbyMeteors);
    IgnoredActors.Append(NearbyMeteors);

    UGameplayStatics::ApplyRadialDamage(
        this,
        DamageAmount, // 폭발은 궤적 데미지보다 강하게 설정(예: 2배)
        Hit.ImpactPoint,
        SphereSize,              // 100.f 검사 범위
        UDamageType::StaticClass(),
        IgnoredActors,
        this,
        GetInstigatorController(),
        true                 // 가려진 대상 제외 여부
    );

    //// [디버그용] 폭발 범위 시각화 (인게임에서 확인 가능)
    //DrawDebugSphere(GetWorld(), Hit.ImpactPoint, SphereSize, 12, FColor::Yellow, false, 2.0f, 0, 2.0f);

    const FVLEffectInfo* Info = VFXData.DataAsset->GetEffectInfoByTag(VFXData.Tag);

    // [조건 처리] ImpactNiagara가 실제로 할당되어 있을 때만 멀티캐스트 호출
    if (Info && !Info->ImpactNiagara.IsNull())
    {
        Multicast_PlayImpactVFX(Hit.ImpactPoint);
    }

    // 나이아가라 시스템에 이미 충돌 이펙트가 처리 되어 있어서 정리
    DeleteProjectile();

}

void AVL_MeteorProjectile::OnRep_VFXData()
{
    if (!VFXData.IsValid())
    {
        CUSTOM_LOG("vfx 데이터가 유효하지 않음");
    }

    const FVLEffectInfo* Info = VFXData.DataAsset->GetEffectInfoByTag(VFXData.Tag);
    if (!Info) return;

    if (IsVFXMoving)
    {
        NiagaraComp->SetRelativeLocation(FVector(0.f, 0.f, -SphereSize / 2));
    }
    // 이펙트 및 사운드 재생 로직
    if (UNiagaraSystem* NS = Info->NiagaraSystem.Get()) 
    {
        NiagaraComp->SetAsset(NS);
        NiagaraComp->Activate();
    }
    else
    {
        UNiagaraSystem* LoadedNS = Info->NiagaraSystem.LoadSynchronous();
        NiagaraComp->SetAsset(LoadedNS);
        NiagaraComp->Activate();
    }

    // 2. 사운드 재생
    if (USoundBase* Sound = Info->Sound.Get())
    {
        UGameplayStatics::SpawnSoundAttached(Sound, RootComponent);
    }
    else
    {
        Sound = Info->Sound.LoadSynchronous();
        UGameplayStatics::SpawnSoundAttached(Sound, RootComponent);
    }
}

void AVL_MeteorProjectile::Multicast_PlayImpactVFX_Implementation(FVector HitLocation)
{

    // 1. 기존 날아가는 VFX 끄기
    if (NiagaraComp)
    {
        NiagaraComp->Deactivate();
    }

    if (!VFXData.IsValid()) return;
    const FVLEffectInfo* Info = VFXData.DataAsset->GetEffectInfoByTag(VFXData.Tag);
    if (!Info) return;

    if (!Info->ImpactNiagara.IsNull())
    {
        UNiagaraSystem* NS = Info->ImpactNiagara.Get();
        if (!NS) NS = Info->ImpactNiagara.LoadSynchronous();

        if (NS)
        {
            UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), NS, HitLocation);
        }
    }

    if (!Info->ImpactSound.IsNull())
    {
        USoundBase* Sound = Info->ImpactSound.Get();
        if (!Sound) Sound = Info->ImpactSound.LoadSynchronous();

        if (Sound)
        {
            UGameplayStatics::PlaySoundAtLocation(this, Sound, HitLocation);
        }
    }

}

void AVL_MeteorProjectile::DeleteProjectile()
{
    if (!HasAuthority()) return;

    bHasExploded = true;

    CollisionComp->SetSimulatePhysics(false);
    CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ProjectileMovementComp->StopMovementImmediately();

    this->SetLifeSpan(3.0f);
}

//
//AVL_MeteorProjectile::AVL_MeteorProjectile()
//{
//    PrimaryActorTick.bCanEverTick = false;
//
//    bReplicates = true;
//    SetReplicateMovement(true);
//
//
//    // 1. 충돌체 설정 (루트)
//    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
//    CollisionComp->InitSphereRadius(50.0f); // 운석 크기에 맞춰 조절
//    CollisionComp->SetCollisionProfileName(TEXT("Projectile"));
//
//    // 바닥 충돌 범위가 크므로 이펙트와 싱크 맞추기 위해서 100을 올려줌.
//    //CollisionComp->SetRelativeLocation(FVector(0, 0, -SphereSize));
//    CollisionComp->SetNotifyRigidBodyCollision(true);
//
//    RootComponent = CollisionComp;
//
//    // 2. 나이아가라 컴포넌트 (루트에 부착)
//    NiagaraComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComp"));
//    NiagaraComp->SetupAttachment(RootComponent);
//    NiagaraComp->SetRelativeLocation(FVector(0, 0, -SphereSize));
//    NiagaraComp->bAutoActivate = false;
//
//
//    // 3. 발사체 이동 컴포넌트
//    ProjectileMovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMove"));
//    ProjectileMovementComp->UpdatedComponent = CollisionComp;
//    ProjectileMovementComp->InitialSpeed = 0.f;
//    ProjectileMovementComp->ProjectileGravityScale = 0.0f;
//}
//
//
//
//void AVL_MeteorProjectile::BeginPlay()
//{
//    Super::BeginPlay();
//
//    // CollisionComp->IgnoreActorWhenMoving(this, true);
//
//    // 궤적 충돌 감지 시작 (서버에서만)
//    if (HasAuthority())
//    {
//        CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &AVL_MeteorProjectile::OnMeteorOverlap);
//
//        // 바닥(월드 정적 물체)에 닿았을 때 폭발하기 위해 Hit 이벤트 바인딩
//        CollisionComp->OnComponentHit.AddDynamic(this, &AVL_MeteorProjectile::OnMeteorHit);
//    }
//}
//
////void AVL_MeteorProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
////{
////    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
////    DOREPLIFETIME(AVL_MeteorProjectile, VFXDataAsset);
////    DOREPLIFETIME(AVL_MeteorProjectile, EffectTag);
////}
//
//void AVL_MeteorProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
//{
//    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
//
//    // 반드시 구조체 변수명을 등록해야 클라이언트로 데이터가 날아갑니다.
//    DOREPLIFETIME(AVL_MeteorProjectile, VFXData);
//}
//
//void AVL_MeteorProjectile::InitializeProjectile(const UVL_VFXDataAsset* InDataAsset, FGameplayTag InTag, float InDamage)
//{
//    if (!HasAuthority()) return;
//
//    // 구조체 데이터 채우기
//    VFXData.DataAsset = const_cast<UVL_VFXDataAsset*>(InDataAsset);
//    VFXData.Tag = InTag;
//
//    this->DamageAmount = InDamage;
//
//    // 속도 설정
//    ProjectileMovementComp->Velocity = FVector(-500.f, 0.f, -500.f);
//    ProjectileMovementComp->UpdateComponentVelocity(); // 물리 속도 즉시 갱신
//
//    // 서버에서는 OnRep이 자동으로 호출되지 않으므로 시각 효과를 위해 직접 호출
//    OnRep_VFXData();
//
//    // 강제 네트워크 업데이트 (데이터가 즉시 복제되도록 유도)
//    ForceNetUpdate();
//
//    // 디버그 타이머 (생략 가능)
//    // ...
//}
//
//// 1. 궤적 충돌 (공중에서 지나가다 맞았을 때)
//void AVL_MeteorProjectile::OnMeteorOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
//{
//    if (!HasAuthority()) return;
//
//    if (!OtherActor || OtherActor == this || OtherActor == GetInstigator())
//    {
//        return;
//    }
//
//    if (AMainCharacterBase* Player = Cast<AMainCharacterBase>(OtherActor))
//    {
//
//        // 이미 이 운석에 맞은 적이 없다면
//        if (!OverlappedActors.Contains(Player))
//        {
//            UGameplayStatics::ApplyDamage(OtherActor, DamageAmount, GetInstigatorController(), this, nullptr);
//            OverlappedActors.Add(Player);
//
//            DeleteProjectile();
//
//        }
//    }
//}
//
//// 2. 바닥 충돌 (바닥에 닿는 순간)
//void AVL_MeteorProjectile::OnMeteorHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
//{
//    if (!HasAuthority()) return;
//
//    if (OtherActor == nullptr || OtherActor == this || OtherActor->IsA(AVL_MeteorProjectile::StaticClass()))
//    {
//        return;
//    }
//
//    if (USoundBase* SoftSound = ExplosionSound.LoadSynchronous())
//    {
//
//        // Hit.ImpactPoint는
//        UGameplayStatics::PlaySoundAtLocation(this, SoftSound, Hit.ImpactPoint);
//    }
//
//    // 2. 폭발 범위(100.f) 내 데미지 판정
//    TArray<AActor*> IgnoredActors;
//    IgnoredActors.Add(this);
//    IgnoredActors.Add(GetOwner());
//
//
//    // 100.0f 반지름으로 원형 데미지 적용
//    TArray<AActor*> NearbyMeteors;
//    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AVL_MeteorProjectile::StaticClass(), NearbyMeteors);
//    IgnoredActors.Append(NearbyMeteors);
//
//    UGameplayStatics::ApplyRadialDamage(
//        this,
//        DamageAmount, // 폭발은 궤적 데미지보다 강하게 설정(예: 2배)
//        Hit.ImpactPoint,
//        SphereSize,              // 100.f 검사 범위
//        UDamageType::StaticClass(),
//        IgnoredActors,
//        this,
//        GetInstigatorController(),
//        true                 // 가려진 대상 제외 여부
//    );
//
//    // [디버그용] 폭발 범위 시각화 (인게임에서 확인 가능)
//    DrawDebugSphere(GetWorld(), Hit.ImpactPoint, SphereSize, 12, FColor::Yellow, false, 2.0f, 0, 2.0f);
//
//    // 나이아가라 시스템에 이미 충돌 이펙트가 처리 되어 있어서 정리
//    DeleteProjectile();
//
//}
//
//void AVL_MeteorProjectile::OnRep_VFXData()
//{
//    if (!VFXData.IsValid())
//    {
//        CUSTOM_LOG("vfx 데이터가 유효하지 않음");
//    }
//
//    const FVLEffectInfo* Info = VFXData.DataAsset->GetEffectInfoByTag(VFXData.Tag);
//    //const FVLEffectInfo* Info = VFXDataAsset->GetEffectInfoByTag(EffectTag);
//    if (!Info) return;
//
//    // 이펙트 및 사운드 재생 로직
//    if (UNiagaraSystem* NS = Info->NiagaraSystem.Get()) // LoadSynchronous 대신 Get 사용
//    {
//        NiagaraComp->SetAsset(NS);
//       // NiagaraComp->SetRelativeScale3D(Info->Scale); // 데이터 에셋의 스케일 반영
//        NiagaraComp->Activate();
//    }
//    else
//    {
//        // 혹시라도 로드가 안 되어 있을 경우를 대비한 방어 코드 (선택 사항)
//        // 만약 보스에서 프리로딩이 확실하다면 이 부분은 실행되지 않습니다.
//        UNiagaraSystem* LoadedNS = Info->NiagaraSystem.LoadSynchronous();
//        NiagaraComp->SetAsset(LoadedNS);
//        NiagaraComp->Activate();
//    }
//
//    // 2. 사운드 재생
//    if (USoundBase* Sound = Info->Sound.Get())
//    {
//        UGameplayStatics::SpawnSoundAttached(Sound, RootComponent);
//    }
//    else
//    {
//        Sound = Info->Sound.LoadSynchronous();
//        UGameplayStatics::SpawnSoundAttached(Sound, RootComponent);
//    }
//}
//
//void AVL_MeteorProjectile::DeleteProjectile()
//{
//    if (GetLifeSpan() > 0.0f) return;
//
//    CollisionComp->SetSimulatePhysics(false);
//    CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
//    ProjectileMovementComp->StopMovementImmediately();
//
//    this->SetLifeSpan(4.0f);
//}
