#pragma once

#include "CoreMinimal.h"
#include "Base/AI/BehaviorTree/Tasks/VL_BTTaskNode.h"
#include "BTTask_Rotate.generated.h"

UCLASS()
class VIALACTEA_API UBTTask_Rotate : public UVL_BTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_Rotate();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UFUNCTION()
	void OnRotateMontageEnded(UAnimMontage* Montage, bool bInterrupted);

private:
	UPROPERTY()
	TObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;

	UPROPERTY()
	TObjectPtr<class AVL_Boss1> CachedBoss;

	/** 타겟 액터 블랙보드 키 */
	//UPROPERTY(EditAnywhere, Category = "Blackboard")
	//FBlackboardKeySelector TargetActorKey;

};
