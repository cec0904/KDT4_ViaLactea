// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/AI/BehaviorTree/Services/VL_BTService.h"
#include "Base/AI/AIState.h"
#include "BTS_CheckState.generated.h"

class AAIController;
class ACharacter;

UCLASS()
class VIALACTEA_API UBTS_CheckState : public UVL_BTService
{
	GENERATED_BODY()
public:
	UBTS_CheckState();

	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTS_CommonMemory); }

protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	// 캐싱을 위한 변수들


};
