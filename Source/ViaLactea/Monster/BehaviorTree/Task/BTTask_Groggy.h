// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/AI/BehaviorTree/Tasks/VL_BTTaskNode.h"
#include "BTTask_Groggy.generated.h"

/**
 * 
 */
UCLASS()
class VIALACTEA_API UBTTask_Groggy : public UVL_BTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_Groggy();

	// 태스크 실행 시 호출
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
