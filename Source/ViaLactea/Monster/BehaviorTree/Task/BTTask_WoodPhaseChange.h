// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/AI/BehaviorTree/Tasks/VL_BTTaskNode.h"
#include "BTTask_WoodPhaseChange.generated.h"

/**
 * 
 */
UCLASS()
class VIALACTEA_API UBTTask_WoodPhaseChange : public UVL_BTTaskNode
{
	GENERATED_BODY()

public:
    UBTTask_WoodPhaseChange();

protected:
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

    UFUNCTION()
    void FinishedChanging();

protected:
    UPROPERTY()
    UBehaviorTreeComponent* CachedOwnerComp;
};
