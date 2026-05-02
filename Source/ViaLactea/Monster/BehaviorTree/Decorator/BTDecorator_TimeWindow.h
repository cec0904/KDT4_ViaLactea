// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_TimeWindow.generated.h"

/**
 * 
 */
UCLASS()
class VIALACTEA_API UBTDecorator_TimeWindow : public UBTDecorator
{
	GENERATED_BODY()
public:
	UBTDecorator_TimeWindow();

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

public:

	// 시작 시간 (Blackboard)
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector StartTimeKey;

	// 지속 시간 (고정값)
	UPROPERTY(EditAnywhere, Category = "Condition")
	float Duration = 1.0f;

	// (선택) Blackboard에서 Duration 읽기
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector DurationKey;

	// DurationKey 쓸지 여부
	UPROPERTY(EditAnywhere, Category = "Condition")
	bool bUseBlackboardDuration = false;
};
