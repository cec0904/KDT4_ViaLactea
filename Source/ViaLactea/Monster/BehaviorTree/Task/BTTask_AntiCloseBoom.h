// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/AI/BehaviorTree/Tasks/VL_BTTaskNode.h"
#include "BTTask_AntiCloseBoom.generated.h"

/**
 * 
 */
UCLASS()
class VIALACTEA_API UBTTask_AntiCloseBoom : public UVL_BTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_AntiCloseBoom();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;


protected:
	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UPROPERTY(EditAnywhere, Category = "VFX")
	FGameplayTag LoopingVFXTag; // 지속용 태그

	UPROPERTY(EditAnywhere, Category = "VFX")
	FGameplayTag ExplosionVFXTag; // 마지막 폭발용 태그

private:
	// 델리게이트 호출 시 참조할 컴포넌트 캐싱
	UPROPERTY()
	UBehaviorTreeComponent* CachedOwnerComp;
};
