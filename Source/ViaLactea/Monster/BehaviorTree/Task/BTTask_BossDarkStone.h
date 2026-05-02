// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/AI/BehaviorTree/Tasks/VL_BTTaskNode.h"
#include "BTTask_BossDarkStone.generated.h"

/**
 * 
 */
UCLASS()
class VIALACTEA_API UBTTask_BossDarkStone : public UVL_BTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_BossDarkStone();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

    virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

protected:
    // 몽타주 종료 콜백
    void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);


protected:
    UPROPERTY(EditAnywhere, Category = "Pattern")
    FGameplayTag DarkStoneTag;

    // 운석이 떨어질 높이
    UPROPERTY(EditAnywhere, Category = "Pattern")
    float SpawnHeight = 1000.f;

private:
    UPROPERTY()
    TObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;
};
