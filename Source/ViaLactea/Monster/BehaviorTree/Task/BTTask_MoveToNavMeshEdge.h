// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/AI/BehaviorTree/Tasks/VL_BTTaskNode.h"
#include "BTTask_MoveToNavMeshEdge.generated.h"

/**
 * 
 */
UCLASS()
class VIALACTEA_API UBTTask_MoveToNavMeshEdge : public UVL_BTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_MoveToNavMeshEdge();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;


};
