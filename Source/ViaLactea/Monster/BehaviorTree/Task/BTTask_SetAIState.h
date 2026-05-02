// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/AI/BehaviorTree/Tasks/VL_BTTaskNode.h"
#include "BTTask_SetAIState.generated.h"

UCLASS()
class VIALACTEA_API UBTTask_SetAIState : public UVL_BTTaskNode
{
    GENERATED_BODY()
public:
    UBTTask_SetAIState();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

    // 에디터에서 어떤 상태로 바꿀지 선택할 수 있도록 노출
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    uint8 NewState; // Enum의 uint8 값을 직접 입력하거나 매칭

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector StateKey;
};
