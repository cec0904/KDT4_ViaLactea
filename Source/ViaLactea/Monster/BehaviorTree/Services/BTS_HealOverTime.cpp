// Fill out your copyright notice in the Description page of Project Settings.


#include "BTS_HealOverTime.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Base/Character/VL_AICharacterBase.h"
#include "Base/Data/Character/VL_BossMonsterDataAsset.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CustomLog/CustomLog.h"

UBTS_HealOverTime::UBTS_HealOverTime()
{
    NodeName = TEXT("Heal and Lock State");
    // 서비스 주기를 0.1초 정도로 설정하여 부드럽게 회복

    bNotifyBecomeRelevant = true;
    bNotifyCeaseRelevant = true;

    bCreateNodeInstance = true;

    Interval = 0.3f;
    RandomDeviation = 0.08f;

    // 블랙보드 키 필터링 (Bool 타입만 선택 가능하게)
    IsLockedKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTS_HealOverTime, IsLockedKey));
}

void UBTS_HealOverTime::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{

    Super::OnBecomeRelevant(OwnerComp, NodeMemory);

    FBTS_CommonMemory* MyMemory = reinterpret_cast<FBTS_CommonMemory*>(NodeMemory);


    ElapsedTime = 0.0f;

    // 각 AI별 전용 메모리에 캐싱
    MyMemory->CachedAIC = OwnerComp.GetAIOwner();
    MyMemory->CachedBB = OwnerComp.GetBlackboardComponent();
    if (MyMemory->CachedAIC.IsValid())
    {
        MyMemory->CachedPawn = Cast<AVL_AICharacterBase>(MyMemory->CachedAIC->GetPawn());
    }

    // 초기 로직 실행
    if (UBlackboardComponent* BB = MyMemory->CachedBB.Get()) // Get()으로 포인터 추출
    {
        BB->SetValueAsBool(IsLockedKey.SelectedKeyName, true);

        if (AAIController* AIC = MyMemory->CachedAIC.Get())
        {
            FVector HomeLocation = BB->GetValueAsVector(TEXT("HomePos"));

            if (ACharacter* MyCharacter = Cast<ACharacter>(MyMemory->CachedPawn.Get()))
            {
                // 2. 캐릭터 무브먼트 컴포넌트 유효성 검사 후 속도 변경
                if (UCharacterMovementComponent* MoveComp = MyCharacter->GetCharacterMovement())
                {
                    MoveComp->MaxWalkSpeed = 300.f;
                }
            }
            AIC->MoveToLocation(HomeLocation, 50.0f);
        }
    }
}

void UBTS_HealOverTime::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    FBTS_CommonMemory* MyMemory = reinterpret_cast<FBTS_CommonMemory*>(NodeMemory);
    AAIController* AIC = MyMemory->CachedAIC.Get();
    UBlackboardComponent* BB = MyMemory->CachedBB.Get();
    AVL_AICharacterBase* AIChar = MyMemory->CachedPawn.Get();

    if (!AIC || !BB || !AIChar || !AIChar->HasAuthority()) return;

    // --- 1. 집(HomePos)에 도착했는지 체크 (성공 조건) ---
    FVector HomeLocation = BB->GetValueAsVector(TEXT("HomePos"));
    float DistToHomeSqr = FVector::DistSquared(AIChar->GetActorLocation(), HomeLocation);
    const float AcceptanceRadiusSqr = FMath::Square(55.0f); // 50cm보다 살짝 여유있게

    if (DistToHomeSqr <= AcceptanceRadiusSqr)
    {
        FinishServiceLogic(OwnerComp, BB);
        return;
    }

    // --- 2. 타겟 거리 체크 (실패/포기 조건) ---
    AActor* TargetActor = Cast<AActor>(BB->GetValueAsObject(TEXT("TargetActor")));
    if (TargetActor)
    {
        float DistToTargetSqr = FVector::DistSquared(AIChar->GetActorLocation(), TargetActor->GetActorLocation());
        if (DistToTargetSqr > MaxAbandonDistanceSqr)
        {
            BB->SetValueAsObject(TEXT("TargetActor"), nullptr); // 타겟 상실
            FinishServiceLogic(OwnerComp, BB);
            return;
        }
    }

    // --- 3. 힐 및 시간 초과 체크 ---

    if (!IsLockedKey.SelectedKeyName.IsNone())
    {
        // 체력회복 10 고정
        AIChar->ApplyHealing();

        ElapsedTime += DeltaSeconds;

        //CUSTOM_LOG("잠금 시간 진행중: %.2f / %.2f", MyMemory->ElapsedTime, LockDuration);
        if (ElapsedTime >= LockDuration)
        {
            //CUSTOM_LOG("잠금 시간 끝------------- %.2f", MyMemory->ElapsedTime);
            FinishServiceLogic(OwnerComp, BB);

        }
    }
}

// 중복 코드를 줄이기 위한 헬퍼 함수
void UBTS_HealOverTime::FinishServiceLogic(UBehaviorTreeComponent& OwnerComp, UBlackboardComponent* BB)
{


    if (BB)
    {
        BB->SetValueAsBool(IsLockedKey.SelectedKeyName, false);
        BB->SetValueAsEnum(TEXT("AIState"), (uint8)EAIState::Patrol);

        // 블랙보드 값을 바꿔서 데코레이터가 Abort를 일으키도록 유도
    }

}

void UBTS_HealOverTime::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    FBTS_CommonMemory* MyMemory = reinterpret_cast<FBTS_CommonMemory*>(NodeMemory);

    if (APawn* MyPawn = MyMemory->CachedPawn.Get())
    {
        if (AVL_AICharacterBase* Monster = Cast<AVL_AICharacterBase>(MyPawn))
        {
            if (UCharacterMovementComponent* MoveComp = Monster->GetCharacterMovement())
            {
                MoveComp->MaxWalkSpeed = Monster->GetMonsterDataAsset()->BaseStats.MaxMoveSpeed;
            }
        }
    }
    UBlackboardComponent* BB = MyMemory->CachedBB.Get();
    if (BB)
    {
        // 서비스가 완전히 종료될 때 확실히 잠금 해제
        BB->SetValueAsBool(IsLockedKey.SelectedKeyName, false);
        BB->SetValueAsObject(TEXT("TargetActor"), nullptr);
        BB->SetValueAsEnum(TEXT("AIState"), (uint8)EAIState::Patrol);
    }
    Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}


