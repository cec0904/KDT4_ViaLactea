#include "BTTask_SpawnMinions.h"
#include "AIController.h"
#include "../../Boss/VL_Boss1.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Base/Data/Character/VL_BossMonsterDataAsset.h"



UBTTask_SpawnMinions::UBTTask_SpawnMinions()
{
	NodeName = TEXT("Spawn Minions");


}

EBTNodeResult::Type UBTTask_SpawnMinions::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController) return EBTNodeResult::Failed;

    // 제어 중인 폰을 보스 클래스로 캐스팅
    AVL_Boss1* Boss = Cast<AVL_Boss1>(AIController->GetPawn());
    if (!Boss)
    {
        return EBTNodeResult::Failed;
    }
    const UVL_BossMonsterDataAsset* BossData = Boss->GetBossDataAsset();

    const FBossPatternData* PData = BossData->GetPatternDataByTag(SummonTag);
    if (!PData) return EBTNodeResult::Failed;

    Boss->OnPatternStarted(PData->PatternType);

    // 보스 클래스에 구현해둔 함수 호출
    Boss->SpawnMinions(SummonTag);


    OwnerComp.GetBlackboardComponent()->SetValueAsEnum(TEXT("AIBossPattern"), (uint8)EAIBossPattern::None);
    // 즉시 완료됨을 알림
    return EBTNodeResult::Succeeded;
}

void UBTTask_SpawnMinions::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
    //CUSTOM_LOG("태스크 종료. 결과: %d", (int32)TaskResult);

    if (AAIController* AIC = OwnerComp.GetAIOwner())
    {
        UBlackboardComponent* BB = AIC->GetBlackboardComponent();
        if (BB)
        {
            ResetBossBlackboard(BB);
        }
    }
    Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}