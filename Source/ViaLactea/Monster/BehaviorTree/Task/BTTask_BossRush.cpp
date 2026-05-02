#include "Monster/BehaviorTree/Task/BTTask_BossRush.h"
#include "AIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "Engine/OverlapResult.h"
#include "NavigationSystem.h"

#include "../../Boss/VL_Boss1.h"
#include "Base/Data/Character/VL_BossMonsterDataAsset.h"

#include "CustomLog/CustomLog.h"

UBTTask_BossRush::UBTTask_BossRush()
{
	NodeName = TEXT("Boss Rush Task");

	INIT_TASK_NODE_NOTIFY_FLAGS();
	
}

EBTNodeResult::Type UBTTask_BossRush::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!AIC) return EBTNodeResult::Failed;

	CachedBoss = Cast<AVL_Boss1>(AIC->GetPawn());
	if (!CachedBoss.IsValid()) return EBTNodeResult::Failed;

	CachedBossData = CachedBoss->GetBossDataAsset();
	if (!CachedBossData.IsValid()) return EBTNodeResult::Failed;

	const FBossPatternData* PData = CachedBossData->GetPatternDataByTag(RushTag);
	if (!PData) return EBTNodeResult::Failed;
	

	CachedBoss->OnPatternStarted(PData->PatternType);

	StartRushMovement(OwnerComp, PData);

	/*TWeakObjectPtr<UBehaviorTreeComponent> WeakOwner(&OwnerComp);*/
	//GetWorld()->GetTimerManager().SetTimer(TelegraphTimerHandle, [this, WeakOwner, PData]()
	//	{
	//		if (auto* Owner = WeakOwner.Get())
	//		{
	//			StartRushMovement(*Owner, PData);
	//		}
	//	}, PData->Duration, false);

	return EBTNodeResult::InProgress;
}

bool UBTTask_BossRush::FindRandomActorLocation2D(UBehaviorTreeComponent& OwnerComp, FVector& OutLocation)
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return false;

	AActor* Target = Cast<AActor>(BB->GetValueAsObject(TEXT("TargetActor")));
	if (!Target) return false;

	FVector TargetLoc = Target->GetActorLocation();
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (NavSys)
	{
		FNavLocation ProjectedLocation;
		// 타겟 위치 주변에서 가장 가까운 바닥(NavMesh) 좌표를 찾습니다.
		if (NavSys->ProjectPointToNavigation(TargetLoc, ProjectedLocation, FVector(100.f, 100.f, 400.f)))
		{
			OutLocation = ProjectedLocation.Location;
		
			return true;
		}
	}
	OutLocation = TargetLoc;

	return true;
}

void UBTTask_BossRush::StartRushMovement(UBehaviorTreeComponent& OwnerComp, const FBossPatternData* PData)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!AIC || !CachedBoss.IsValid()) return;

	// 4. 위치 찾기 및 이동 시작
	FVector TargetLocation;
	if (FindRandomActorLocation2D(OwnerComp, TargetLocation))
	{
		CachedBoss->SetRushMode(true);
		auto MoveResult = AIC->MoveToLocation(TargetLocation, 200.0f);
		if (MoveResult == EPathFollowingRequestResult::Failed || MoveResult == EPathFollowingRequestResult::AlreadyAtGoal)
		{
			MyCleanup(OwnerComp);
			FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
			return;
		}

		// 5. 돌진 지속 시간 타이머
		GetWorld()->GetTimerManager().SetTimer(RushTimerHandle, [this, WeakOwner = TWeakObjectPtr<UBehaviorTreeComponent>(&OwnerComp)]()
			{
				if (auto* Owner = WeakOwner.Get())
				{
					MyCleanup(*Owner);
					FinishLatentTask(*Owner, EBTNodeResult::Succeeded);
				}
			}, PData->Duration, false);
	}
	else
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
	}
}

void UBTTask_BossRush::MyCleanup(UBehaviorTreeComponent& OwnerComp)
{
	if (UWorld* World = OwnerComp.GetWorld())
	{
		World->GetTimerManager().ClearTimer(RushTimerHandle);
		World->GetTimerManager().ClearTimer(TelegraphTimerHandle);
	}

	if (CachedBoss.IsValid())
	{

		CachedBoss->SetRushMode(false);
	}

	if (AAIController* AIC = OwnerComp.GetAIOwner())
	{
		AIC->StopMovement();
	}
	if (UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
	{
		BB->SetValueAsEnum(TEXT("AIBossPattern"), (uint8)EAIBossPattern::None);
	}
}

EBTNodeResult::Type UBTTask_BossRush::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 1. 진행 중인 모든 작업 중단 및 자원 정리
	MyCleanup(OwnerComp);

	// 2. 즉시 중단되었다면 Aborted 반환
	return EBTNodeResult::Aborted;
}


void UBTTask_BossRush::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	MyCleanup(OwnerComp);

	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}