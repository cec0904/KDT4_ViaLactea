// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/AI/BehaviorTree/Tasks/VL_BTTaskNode.h"
#include "BTTask_SetLocation.generated.h"

/**
 * 
 */
UCLASS()
class VIALACTEA_API UBTTask_SetLocation : public UVL_BTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_SetLocation();

	// --- BT Task Overrides ---
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override; // override 명시
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

protected:
	// --- Delegate Callbacks ---
	UFUNCTION()
	void OnBossLanded(const FHitResult& Hit);

	UFUNCTION()
	void OnSetLocationMontageEnded(UAnimMontage* Montage, bool bInterrupted);

private:
	// --- Utility ---
	bool CalculateJumpVelocity(FVector Start, FVector End, FVector& OutVelocity, float& Time);


private:
	// --- Cached Data (Latent Task용) ---
	// TObjectPtr은 가비지 컬렉션으로부터 안전하게 보호해줍니다.
	UPROPERTY()
	TObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;

	UPROPERTY()
	TObjectPtr<class AVL_Boss1> CachedBoss; // 구체적인 클래스 타입으로 캐싱하면 캐스팅 비용 절감 가능

	UPROPERTY()
	class UAnimMontage* CachedMontage;

	float JumpTime= 0.f;
};
