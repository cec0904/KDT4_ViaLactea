// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/AI/BehaviorTree/Tasks/VL_BTTaskNode.h"
#include "BTTask_SpawnMinions.generated.h"

UCLASS()
class VIALACTEA_API UBTTask_SpawnMinions : public UVL_BTTaskNode
{
	GENERATED_BODY()
public:
    UBTTask_SpawnMinions();

protected:
    // 태스크 실행 시 호출되는 핵심 함수
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

    virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

private:
    // 에디터에서 어떤 소환 타입을 사용할지 선택할 수 있게 노출
    UPROPERTY(EditAnywhere, Category = "Spawn", meta = (AllowPrivateAccess = "true"))
    FGameplayTag SummonTag;
};
