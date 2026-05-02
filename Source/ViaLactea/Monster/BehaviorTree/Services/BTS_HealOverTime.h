#pragma once

#include "CoreMinimal.h"
#include "Base/AI/BehaviorTree/Services/VL_BTService.h"
#include "Base/AI/AIState.h"
#include "BTS_HealOverTime.generated.h"

UCLASS()
class VIALACTEA_API UBTS_HealOverTime : public UVL_BTService
{
	GENERATED_BODY()

public:
	UBTS_HealOverTime();

protected:
	// 서비스가 활성화될 때 (노드 진입 시) 호출
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	// 주기적으로 호출 (Interval 설정에 따름)
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	// 서비스가 중단될 때 호출
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTS_CommonMemory); }

	void FinishServiceLogic(UBehaviorTreeComponent& OwnerComp, UBlackboardComponent* BB);
protected:
	// 거리 임계값 제곱 (헤더에 상수로 두면 더 좋음)
	const float MaxAbandonDistance = 1500.0f;
	const float MaxAbandonDistanceSqr = MaxAbandonDistance * MaxAbandonDistance;
private:
	/** 블랙보드 키 설정 */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector IsLockedKey;

	/** 설정 값 */
	UPROPERTY(EditAnywhere, Category = "Time")
	float HealAmountPerSecond = 10.0f;

	UPROPERTY(EditAnywhere, Category = "Time")
	float LockDuration = 2.0f;

	float ElapsedTime = 0.f;
};
