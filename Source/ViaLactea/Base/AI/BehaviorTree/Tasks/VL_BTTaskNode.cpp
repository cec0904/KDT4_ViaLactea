#include "VL_BTTaskNode.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Base/AI/AIBossPattern.h"

UVL_BTTaskNode::UVL_BTTaskNode()
{
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UVL_BTTaskNode::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	return EBTNodeResult::Aborted;
}
void UVL_BTTaskNode::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

void UVL_BTTaskNode::ResetBossBlackboard(UBlackboardComponent* BB)
{
	if (!BB) return;

	BB->SetValueAsEnum(TEXT("AIBossPattern"), (uint8)EAIBossPattern::None);
	BB->SetValueAsEnum(TEXT("PatternFamily"), (uint8)EPatternFamily::None);
}
