// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/AI/BehaviorTree/Tasks/VL_BTTaskNode.h"
#include "BTTask_PlayCustomAnimation.generated.h"

/**
 * 
 */
UCLASS()
class VIALACTEA_API UBTTask_PlayCustomAnimation : public UVL_BTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_PlayCustomAnimation();

	// 태스크 실행 시 호출
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
    // 애니메이션이 끝났을 때 호출될 함수
    void OnAnimationFinished(UBehaviorTreeComponent* OwnerComp);

    UPROPERTY(EditAnywhere, Category = "Animation")
    UAnimMontage* MontageToPlay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
	FBlackboardKeySelector StateKey;

};
