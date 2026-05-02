#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "VL_BTTaskNode.generated.h"

/**
 * 
 */
UCLASS()
class VIALACTEA_API UVL_BTTaskNode : public UBTTaskNode
{
	GENERATED_BODY()
	//INIT_TASK_NODE_NOTIFY_FLAGS();
public:
	UVL_BTTaskNode();
protected:
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

	// 자식 클래스에서 이 함수를 호출하기만 하면 됩니다.
	void ResetBossBlackboard(UBlackboardComponent* BB);

};
