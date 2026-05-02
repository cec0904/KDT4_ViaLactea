#pragma once

#include "CoreMinimal.h"
#include "Base/AI/BehaviorTree/Tasks/VL_BTTaskNode.h"
#include "BTTask_Backstep.generated.h"


UCLASS()
class VIALACTEA_API UBTTask_Backstep : public UVL_BTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_Backstep();

	// --- BT Task Overrides ---
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

protected:

	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UPROPERTY(EditAnywhere, Category = "Effects")
	UAnimMontage* BackstepMontage;


	float DefaultGravity = 0.f;
	float DefaultFriction = 0.f;
	float DefaultBraking = 0.f;

	FTimerHandle LaunchTimerHandle;

private:
	FVector TargetLocation;
	/*float MoveSpeed = 800.0f;*/

private:
	UPROPERTY()
	UBehaviorTreeComponent* CachedOwnerComp;
};