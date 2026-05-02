// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/AI/BehaviorTree/Tasks/VL_BTTaskNode.h"
#include "BTTask_BossAoE.generated.h"

class UAnimMontage;

UCLASS()
class VIALACTEA_API UBTTask_BossAoE : public UVL_BTTaskNode
{
	GENERATED_BODY()

public:
    UBTTask_BossAoE();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

    virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

    virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

protected:
    UFUNCTION()
    void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);


protected:
    UPROPERTY(EditAnywhere, Category = "Pattern")
    FGameplayTag AoETag;

private:
    UPROPERTY()
    UBehaviorTreeComponent* CachedOwnerComp;

    UAnimMontage* MontageToPlay = nullptr;
};
