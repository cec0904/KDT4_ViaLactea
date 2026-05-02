
#pragma once

#include "CoreMinimal.h"
#include "Base/AI/BehaviorTree/Tasks/VL_BTTaskNode.h"
#include "AITypes.h"
#include "Navigation/PathFollowingComponent.h"
#include "BTTask_BossRush.generated.h"

class AVL_Boss1;
class UVL_BossMonsterDataAsset;
class UBehaviorTreeComponent;
struct FBossPatternData;

UCLASS()
class VIALACTEA_API UBTTask_BossRush : public UVL_BTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_BossRush();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	void MyCleanup(UBehaviorTreeComponent& OwnerComp);

	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;



protected:
	// 비헤이비어 트리 에디터에서 어떤 AoE 패턴을 쓸지 선택 (예: Boss.Pattern.AoE.Fire)
	UPROPERTY(EditAnywhere, Category = "Pattern")
	FGameplayTag RushTag;

private:
	bool FindRandomActorLocation2D(UBehaviorTreeComponent& OwnerComp, FVector& OutLocation);

	void StartRushMovement(UBehaviorTreeComponent& OwnerComp, const FBossPatternData* PData);

	TWeakObjectPtr<AVL_Boss1> CachedBoss;

	TWeakObjectPtr<UVL_BossMonsterDataAsset> CachedBossData;

	FAIRequestID MoveRequestID;

	float OriginalWalkSpeed;

	FTimerHandle RushTimerHandle;
	FTimerHandle TelegraphTimerHandle;

private:
	UPROPERTY()
	UBehaviorTreeComponent* CachedOwnerComp;
};
