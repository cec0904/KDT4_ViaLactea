// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/AI/BehaviorTree/Tasks/VL_BTTaskNode.h"
#include "GameplayTagContainer.h"
#include "BTTask_Throw.generated.h"

/**
 * 
 */
UCLASS()
class VIALACTEA_API UBTTask_Throw : public UVL_BTTaskNode
{
	GENERATED_BODY()

public:
    UBTTask_Throw();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    
    virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
protected:
    UFUNCTION()
    void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);
    
    virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;


protected:
    UPROPERTY(EditAnywhere, Category = "Pattern")
    FGameplayTag ThrowTag;

    UPROPERTY()
    UAnimMontage* MyPlayingMontage = nullptr;
private:
    UPROPERTY()
    TWeakObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;
};
