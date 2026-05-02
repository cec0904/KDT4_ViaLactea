// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/AI/BehaviorTree/Tasks/VL_BTTaskNode.h"
#include "BTTask_Strafe.generated.h"

/**
 * 
 */
UCLASS()
class VIALACTEA_API UBTTask_Strafe : public UVL_BTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_Strafe();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult);
	
	EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory);

	void ResetMovementSettings(UBehaviorTreeComponent& OwnerComp);

protected:
	// 이동 완료 메시지를 받기 위한 함수
	virtual void OnMessage(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, FName Message, int32 RequestID, bool bSuccess) override;

protected:
	// 타이머 핸들 저장
	FTimerHandle StrafeTimerHandle;

private:
	float StrafeSpeed;
	float StrafeLength;
	bool bStrafeRight;
};
