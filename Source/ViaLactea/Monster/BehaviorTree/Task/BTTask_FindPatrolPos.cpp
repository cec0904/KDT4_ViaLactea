#include "BTTask_FindPatrolPos.h"

#include "Base/FrameWork/VL_AIControllerBase.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "CustomLog/CustomLog.h"

UBTTask_FindPatrolPos::UBTTask_FindPatrolPos()
{
	NodeName = TEXT("Find Random Patrol Pos");
	INIT_TASK_NODE_NOTIFY_FLAGS();

}

EBTNodeResult::Type UBTTask_FindPatrolPos::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	//Super::ExecuteTask(OwnerComp, NodeMemory);

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

	APawn* AIPawn = AIController->GetPawn();
	if (!AIPawn) return EBTNodeResult::Failed;

	UNavigationSystemV1* NaviSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NaviSystem) return EBTNodeResult::Failed;

	UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();
	if (!BBComp) return EBTNodeResult::Failed;

	FVector OriginPos = AIPawn->GetActorLocation();
	FNavLocation NextPos;

	for (int32 i = 0; i < 5; ++i)
	{
		if (NaviSystem->GetRandomPointInNavigableRadius(OriginPos, SearchRadius, NextPos))
		{
			OwnerComp.GetBlackboardComponent()->SetValueAsVector(PatrolPosKey.SelectedKeyName, NextPos.Location);

			return EBTNodeResult::Succeeded;
		}
	}
	return EBTNodeResult::Failed;
}