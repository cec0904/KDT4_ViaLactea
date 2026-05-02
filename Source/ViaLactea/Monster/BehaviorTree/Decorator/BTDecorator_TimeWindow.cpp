// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/BehaviorTree/Decorator/BTDecorator_TimeWindow.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

UBTDecorator_TimeWindow::UBTDecorator_TimeWindow()
{
	NodeName = "Time Window Check";

	// 핵심: 조건 깨지면 자기 자신 Abort
	FlowAbortMode = EBTFlowAbortMode::Self;
}

bool UBTDecorator_TimeWindow::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return false;

	// 시작 시간
	const float StartTime = BB->GetValueAsFloat(StartTimeKey.SelectedKeyName);

	// 현재 시간
	const float CurrentTime = OwnerComp.GetWorld()->GetTimeSeconds();

	// Duration 결정
	float FinalDuration = Duration;

	if (bUseBlackboardDuration)
	{
		FinalDuration = BB->GetValueAsFloat(DurationKey.SelectedKeyName);
	}

	// 경과 시간
	const float Elapsed = CurrentTime - StartTime;
	// duration 시간보다 작은 시간에 들어오면 ture 
	return Elapsed < FinalDuration;
}
