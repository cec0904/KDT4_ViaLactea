// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/AI/BehaviorTree/Services/VL_BTService.h"
#include "BTS_SetMovementMode.generated.h"

/**
 * 
 */
UCLASS()
class VIALACTEA_API UBTS_SetMovementMode : public UVL_BTService
{
	GENERATED_BODY()
public:
	UBTS_SetMovementMode();

protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector BIsLockedKey;
};
