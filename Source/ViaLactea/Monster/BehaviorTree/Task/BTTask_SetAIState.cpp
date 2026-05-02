#include "BTTask_SetAIState.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_SetAIState::UBTTask_SetAIState()
{
    NodeName = TEXT("Set AI State");
}

EBTNodeResult::Type UBTTask_SetAIState::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (BB)
    {
        // 선택한 블랙보드 키(AIState)에 새로운 값(NewState)을 설정
        BB->SetValueAsEnum(StateKey.SelectedKeyName, NewState);
        return EBTNodeResult::Succeeded;
    }

    return EBTNodeResult::Failed;
}
