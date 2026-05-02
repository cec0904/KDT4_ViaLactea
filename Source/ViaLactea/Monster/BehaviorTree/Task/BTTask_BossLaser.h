// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/AI/BehaviorTree/Tasks/VL_BTTaskNode.h"
#include "BTTask_BossLaser.generated.h"

/**
 * 
 */
UCLASS()
class VIALACTEA_API UBTTask_BossLaser : public UVL_BTTaskNode
{
	GENERATED_BODY()
public:
    UBTTask_BossLaser();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;


protected:
    // 몽타주 종료 시 호출될 델리게이트
    //void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    FTimerHandle LaserTimerHandle;

    UPROPERTY(EditAnywhere, Category = "Design")
    FGameplayTag LaserTag;

private:
    UPROPERTY()
    UBehaviorTreeComponent* CachedOwnerComp;
};
