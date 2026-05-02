#include "Monster/Boss/Gimmick/VL_LaserProjectile.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

#include "Player/MainCharacterBase.h"
#include "Monster/Boss/VL_Boss1.h"
#include "Base/Data/VFX/VL_VFXDataAsset.h"
#include "GameFramework/GameStateBase.h"

#include "CustomLog/CustomLog.h"

AVL_LaserProjectile::AVL_LaserProjectile()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    AActor::SetReplicateMovement(false);

    // 2. 루트 컴포넌트 설정
    // 레이저의 시작점 역할을 합니다.
    DefaultRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultRoot"));
    SetRootComponent(DefaultRoot);

    // 3. 비주얼 컴포넌트 (나이아가라)
    LaserVisualComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("LaserVisualComp"));
    LaserVisualComp->SetupAttachment(RootComponent);
    // 레이저는 소환 직후 InitializeLaser에서 켜줄 것이므로 초기엔 비활성화 가능
    LaserVisualComp->bAutoActivate = false;


     // 4. 초기화 변수 설정
    CurrentElapsedTime = 0.0f;
    MaxDuration = 2.0f;
    DamageAmount = 20.f;
    bIsLaserActive = false;

    // 레이저 기본 두께 (생성자 단계에서 기본값 부여)
    LaserRadius = 30.0f;
    bShowDebugLine = false; // 기본적으론 끔

    // 멀티히트 대상 목록 초기화 (메모리 미리 예약)
    HitActors.Reserve(10);
}

// Called when the game starts or when spawned
void AVL_LaserProjectile::BeginPlay()
{
	Super::BeginPlay();
	
}

void AVL_LaserProjectile::InitializeLaser(const UVL_VFXDataAsset* InDataAsset, FGameplayTag InTag, AActor* InTargetActor, float InDuration, float InSize)
{
    if (!HasAuthority()) return;

    VFXData.DataAsset = const_cast<UVL_VFXDataAsset*>(InDataAsset);
    VFXData.Tag = InTag;

    TargetActor = InTargetActor; // 
    MaxDuration = InDuration;

    //ServerStartTime = GetWorld()->GetTimeSeconds();

    if (AGameStateBase* GS = GetWorld()->GetGameState())
        ServerStartTime = GS->GetServerWorldTimeSeconds();
    else
        ServerStartTime = GetWorld()->GetTimeSeconds();

    LaserRadius = InSize;
    LaserMaxRange = 8000.f;

    CurrentElapsedTime = 0.0f;

    HitActors.Reset();

    if (GetOwner())
    {
        StartLocation = GetOwner()->GetRootComponent()->GetSocketLocation(TEXT("Muzzle_Laser"));
    }
    else
    {
        // Owner가 아직 없다면, 서버가 마지막으로 복제해준 본인 위치라도 사용
        StartLocation = GetActorLocation();
    }

    FVector MyLocation = GetActorLocation();

    // 바닥에서 솟구치는 효과를 위해 본인 위치 아래쪽을 시작점으로 설정
    InitialRotation = GetActorRotation();

    if (TargetActor)
    {
        FinalTargetLocation = TargetActor->GetActorLocation();
    }
    else
    {
        FinalTargetLocation = MyLocation + GetActorForwardVector() * 1000.f;
    }


    bIsLaserActive = true;

    if (GetNetMode() != NM_DedicatedServer)
    {
        OnRep_VFXData();
    }


}

// Called every frame
void AVL_LaserProjectile::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);


    if (!bIsLaserActive) return;

    float CurrentWorldTime =0.f;
    if (AGameStateBase* GS = GetWorld()->GetGameState())
    {
        CurrentWorldTime = GS->GetServerWorldTimeSeconds();
    }

    float SynchronizedElapsed = CurrentWorldTime - ServerStartTime;

   // float Alpha = FMath::Clamp(CurrentElapsedTime / MaxDuration, 0.2f, 1.0f);
    float Alpha = FMath::Clamp(SynchronizedElapsed / MaxDuration, 0.2f, 1.0f);

    //if (TargetActor)
    //{
    //    FinalTargetLocation = TargetActor->GetActorLocation();
    //}

    if (GetOwner())
    {
        
        AActor* BossActor = GetOwner();
        if (BossActor)
        {
            ACharacter* BossCharacter = Cast<ACharacter>(BossActor);
            if (BossCharacter)
            {
                StartLocation = BossCharacter->GetMesh()->GetSocketLocation(TEXT("Muzzle_Laser"));
            }
            else
            {
                // 캐릭터가 아니라면 Root 위치라도 가져옴
                StartLocation = BossActor->GetActorLocation();
            }
        }
        SetActorLocation(StartLocation);
    }

    FRotator TargetRotation = (FinalTargetLocation - StartLocation).Rotation();
    FQuat CurrentQuat = FQuat::Slerp(FQuat(InitialRotation), FQuat(TargetRotation), Alpha);
    SetActorRotation(CurrentQuat.Rotator());


    FVector StartPos = GetActorLocation();
    FVector ForwardDir = GetActorForwardVector();
    FVector MaxEndPos = StartPos + (ForwardDir * LaserMaxRange);

    // 2. LineTrace로 장애물(땅 등) 확인
    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetOwner()); // 보스 자신은 무시
    Params.AddIgnoredActor(this);      // 레이저 액터 자신도 무시

    // ECC_Visibility 또는 프로젝트에서 설정한 Ground 채널을 사용하세요.
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartPos,
        MaxEndPos,
        ECC_Visibility,
        Params
    );
    float CurrentLength = LaserMaxRange;
    // 3. 거리 계산 (Hit 여부에 따라)
    float FinalDistance = bHit ? HitResult.Distance : LaserMaxRange;

    if (LaserVisualComp)
    {
        // 나이아가라 에디터에 설정된 파라미터 이름에 맞추기
        FVector2D LaserSize(LaserRadius, FinalDistance);
        LaserVisualComp->SetVariableVec2(TEXT("User.LaserSize"), LaserSize);

        FVector CollisionData(FinalDistance, 0.0f, 0.0f);
        LaserVisualComp->SetVariableVec3(TEXT("User.EndSize"), CollisionData);
    }
    
    // 5. 데미지 판정 (서버에서만 수행)
    if (HasAuthority())
    {
        FVector CurrentBeamEnd = StartLocation + (GetActorForwardVector() * FinalDistance);

        ProcessDamage(StartLocation, CurrentBeamEnd);
        // 기간 만료 시 파괴
        if (SynchronizedElapsed >= MaxDuration)
        {
            DeactivateLaser();
        }
    }
}

void AVL_LaserProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps); // Super 필수 호출

    DOREPLIFETIME(AVL_LaserProjectile, VFXData);
    DOREPLIFETIME(AVL_LaserProjectile, InitialRotation);
    DOREPLIFETIME(AVL_LaserProjectile, FinalTargetLocation);
    DOREPLIFETIME(AVL_LaserProjectile, bIsLaserActive);


    DOREPLIFETIME(AVL_LaserProjectile, ServerStartTime);
    DOREPLIFETIME(AVL_LaserProjectile, MaxDuration);
}

void AVL_LaserProjectile::OnRep_VFXData()
{
    if (!VFXData.DataAsset || !LaserVisualComp) return;

    const FVLEffectInfo* Info = VFXData.DataAsset->GetEffectInfoByTag(VFXData.Tag);
    if (!Info) return;

    UNiagaraSystem* NS = Info->NiagaraSystem.Get();
    if (!NS) NS = Info->NiagaraSystem.LoadSynchronous(); // 없으면 채우기

    if (NS)
    {
        // NS가 '기존에 있었든', '방금 로드했든' 상관없이 동일하게 처리
        LaserVisualComp->SetAsset(NS);
        LaserVisualComp->Activate(true);
    }
}

void AVL_LaserProjectile::UpdateLaserTransform(float DeltaTime)
{
}
void AVL_LaserProjectile::ProcessDamage(FVector BeamStart, FVector BeamEnd)
{
    if (!HasAuthority()) return;
    // 1. 여러 개의 히트 결과를 담을 배열
    TArray<FHitResult> HitResults;

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    Params.AddIgnoredActor(GetOwner());

    // 레이저의 두께 설정
    FCollisionShape SphereShape = FCollisionShape::MakeSphere(LaserRadius);

    // 2. SweepMultiByChannel 사용 (경로상의 모든 대상 검출)
    bool bHit = GetWorld()->SweepMultiByChannel(
        HitResults,
        BeamStart,
        BeamEnd,
        FQuat::Identity,
        ECC_Pawn, // 또는 프로젝트에서 설정한 적군 전용 채널 (예: ECC_GameTraceChannel1)
        SphereShape,
        Params
    );

    if (bHit)
    {
        for (const FHitResult& Hit : HitResults)
        {
            AActor* HitActor = Hit.GetActor();

            // 3. 중복 타격 방지 로직
            // HitActors는 헤더에 정의된 TArray<AActor*> 입니다.
            if (Cast<AMainCharacterBase>(HitActor) && !HitActors.Contains(HitActor))
            {
                //CUSTOM_LOG("%s 맞음", *HitActor->GetName());
                UGameplayStatics::ApplyDamage(
                    HitActor,
                    DamageAmount,
                    GetInstigatorController(),
                    GetOwner(),
                    UDamageType::StaticClass()
                );

                // 이번 레이저 발사에서 이미 맞은 목록에 추가
                HitActors.Add(HitActor);

                // (선택 사항) 피격 이펙트가 필요하다면 여기서 스폰
                // UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactVFX, Hit.ImpactPoint);
            }
        }
    }
}

void AVL_LaserProjectile::DeactivateLaser()
{
    bIsLaserActive = false;

    if (LaserVisualComp)
    {
        // Deactivate는 이미 생성된 파티클은 유지하되, 새로운 파티클 생성을 멈춤
        LaserVisualComp->Deactivate();
    }
    Destroy();
}


