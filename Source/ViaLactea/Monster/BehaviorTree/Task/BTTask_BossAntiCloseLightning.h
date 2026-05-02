// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/AI/BehaviorTree/Tasks/VL_BTTaskNode.h"
#include "BTTask_BossAntiCloseLightning.generated.h"

/**
 * 
 */
UCLASS()
class VIALACTEA_API UBTTask_BossAntiCloseLightning : public UVL_BTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_BossAntiCloseLightning();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult);

    UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

protected:
	UPROPERTY(EditAnywhere, Category = "Pattern")
	FGameplayTag LightningTag;

private:
	TWeakObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;
	TArray<FVector> SpawnLocations;
};
