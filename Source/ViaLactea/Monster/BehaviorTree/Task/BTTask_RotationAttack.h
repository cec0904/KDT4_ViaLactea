// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/AI/BehaviorTree/Tasks/VL_BTTaskNode.h"
#include "BTTask_RotationAttack.generated.h"

/**
 * 
 */
UCLASS()
class VIALACTEA_API UBTTask_RotationAttack : public UVL_BTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_RotationAttack();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

private:
	UPROPERTY(EditAnywhere, Category = "Effects")
	UAnimMontage* RotationAttackMontage;

	UPROPERTY()
	UBehaviorTreeComponent* CachedOwnerComp;

	
};
