// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_MoveTo.h"
#include "BTTask_MoveToDuration.generated.h"

/**
 * 
 */
UCLASS()
class VIALACTEA_API UBTTask_MoveToDuration : public UBTTask_MoveTo
{
	GENERATED_BODY()

public:
	UBTTask_MoveToDuration();

protected:
	// 태스크 시작 시 호출
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	// 매 프레임 실행 시간 체크를 위해 호출
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	// 태스크 중단(Abort) 시 처리 (중요: 이동 멈춤 처리)
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;


private:
	/* 이동을 지속할 최대 시간 */
	UPROPERTY(EditAnywhere, Category = "Node", meta = (ClampMin = "0.0"))
	float MaxMoveDuration = 2.0f;

	/* 현재 누적된 시간 */
	float ElapsedTime = 0.0f;
};
