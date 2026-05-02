#include "Monster/Boss/Gimmick/VL_BossGimmickActor.h"
#include "Net/UnrealNetwork.h"

AVL_BossGimmickActor::AVL_BossGimmickActor()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true; // 멀티플레이

    RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
    RootComponent = RootComp;

    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    MeshComp->SetupAttachment(RootComp);

    MaxHealth = 100.0f;
    CurrentState = EGimmickState::Idle;
}

void AVL_BossGimmickActor::BeginPlay()
{
    Super::BeginPlay();
    CurrentHealth = MaxHealth;
}

void AVL_BossGimmickActor::SetGimmickState(EGimmickState NewState)
{
    if (CurrentState == NewState || CurrentState == EGimmickState::Destroyed) return;

    CurrentState = NewState;

    // 1. 델리게이트 (보스가 이 기믹을 주시하고 있다면 알아챔)
    OnStateChanged.Broadcast(CurrentState);

    // 2. 서버에서 시각적 연출 실행
    HandleStateVisuals(CurrentState);
}

// 상태 연출 함수 기본 로직 (블루프린트에서 오버라이드 가능)
void AVL_BossGimmickActor::HandleStateVisuals_Implementation(EGimmickState State)
{
    // 상태에 따라 머티리얼 색상을 바꾸는 로직을 여기에 작성하거나 Blueprint에 위임
}

//화살타격 
float AVL_BossGimmickActor::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    if (CurrentState == EGimmickState::Destroyed) return 0.0f;

    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    CurrentHealth -= ActualDamage;

    // 체력이 다 달았을 때 파괴 처리
    if (CurrentHealth <= 0.0f)
    {
        SetGimmickState(EGimmickState::Destroyed);
        OnGimmickDestroyed.Broadcast(this);

        // n초 후 Destroy()를 호출하거나, 풀링(Pooling)을 위해 숨기기 처리
    }

    return ActualDamage;
}

// 멀티플레이 동기화
void AVL_BossGimmickActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AVL_BossGimmickActor, CurrentState);
}


void AVL_BossGimmickActor::OnRep_CurrentState()
{
    HandleStateVisuals(CurrentState);
}

// 인터페이스 구현
bool AVL_BossGimmickActor::IsTargetable_Implementation() const
{
    // 파괴되지 않은 상태면 돌진 가능 (자식에서 로직 변경 가능)
    return CurrentState != EGimmickState::Destroyed;
}

FVector AVL_BossGimmickActor::GetGimmickTargetLocation_Implementation() const
{
    return GetActorLocation();
}