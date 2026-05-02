// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_MoveToNavMeshEdge.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "Base/AI/AIState.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"

UBTTask_MoveToNavMeshEdge::UBTTask_MoveToNavMeshEdge()
{
    NodeName = TEXT("MoveTo NavMesh Edge Pos");
    INIT_TASK_NODE_NOTIFY_FLAGS();

}

EBTNodeResult::Type UBTTask_MoveToNavMeshEdge::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIC = OwnerComp.GetAIOwner();
    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());

    if (!AIC || !BB || !NavSys || !AIC->GetPawn()) return EBTNodeResult::Failed;

    APawn* AIPawn = AIC->GetPawn();
    FVector AILoc = AIPawn->GetActorLocation();

    FVector HomePos = BB->GetValueAsVector(TEXT("HomePos"));

    FVector DirToHome = (HomePos - AILoc);
    DirToHome.Z = 0.f;

    // 만약 위치가 거의 같다면 (Zero Vector 방지)
    DirToHome = DirToHome.GetSafeNormal();
    if (DirToHome.IsNearlyZero())
    {
        DirToHome = AIPawn->GetActorForwardVector();
    }

    FNavLocation EdgeLocation;
    bool bFoundEdge = false;

    for (int32 i = 0; i < 5; ++i)
    {
        float SearchDist = 300.f + (i * 300.f);
        FVector TestPoint = AILoc + (DirToHome * SearchDist);

        if (NavSys->ProjectPointToNavigation(TestPoint, EdgeLocation, FVector(300.f, 300.f, 500.f)))
        {
            bFoundEdge = true;
            break;
        }
    }

    // 4. 찾았다면 그 지점으로 이동 명령
    if (bFoundEdge)
    {
        // 이미 네비메쉬 위에 있다면 성공 반환
        if (FVector::DistSquared(AILoc, EdgeLocation.Location) < 2500.f) // 50cm 이내
        {
            return EBTNodeResult::Succeeded;
        }

        EPathFollowingRequestResult::Type RequestResult = AIC->MoveToLocation(EdgeLocation.Location, 50.f);

        if (RequestResult != EPathFollowingRequestResult::Failed)
        {
            return EBTNodeResult::Succeeded;
        }
    }

    AIPawn->SetActorLocation(HomePos);
    return EBTNodeResult::Failed;
}

void UBTTask_MoveToNavMeshEdge::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
    if (AAIController* AIC = OwnerComp.GetAIOwner())
    {
        if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
        {
            ResetBossBlackboard(BB);
            BB->SetValueAsEnum(TEXT("AIState"), (uint8)EAIState::Combat);

        }
    }
    Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}
