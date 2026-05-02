#include "Monster/BehaviorTree/Task/BTTask_MoveToDuration.h"
#include "AIController.h"

UBTTask_MoveToDuration::UBTTask_MoveToDuration()
{
	NodeName = "Move To (Duration)";
	// TickTask를 사용하기 위해 true 설정
	bNotifyTick = true;
	INIT_TASK_NODE_NOTIFY_FLAGS();

}

EBTNodeResult::Type UBTTask_MoveToDuration::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 1. 경과 시간 초기화
	ElapsedTime = 0.0f;

	// 2. 부모 클래스(BTTask_MoveTo)의 이동 로직 실행
	// 이 시점에 AI는 블랙보드 키를 향해 경로 탐색 및 이동을 시작합니다.
	EBTNodeResult::Type NodeResult = Super::ExecuteTask(OwnerComp, NodeMemory);

	// 만약 이미 도착했거나 실패했다면 즉시 결과 반환
	if (NodeResult != EBTNodeResult::InProgress)
	{
		return NodeResult;
	}

	return EBTNodeResult::InProgress;
}

void UBTTask_MoveToDuration::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	ElapsedTime += DeltaSeconds;

	// 3. 설정한 시간을 초과했는지 체크
	if (ElapsedTime >= MaxMoveDuration)
	{
		AAIController* MyController = OwnerComp.GetAIOwner();
		if (MyController)
		{
			// 이동 강제 중지
			MyController->StopMovement();
		}

		// 성공으로 태스크 종료 (다음 노드로 넘어감)
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

EBTNodeResult::Type UBTTask_MoveToDuration::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* MyController = OwnerComp.GetAIOwner();
	if (MyController)
	{
		MyController->StopMovement();
	}

	return Super::AbortTask(OwnerComp, NodeMemory);
}
