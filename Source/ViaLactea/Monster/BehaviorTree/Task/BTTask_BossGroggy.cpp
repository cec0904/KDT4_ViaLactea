#include "Monster/BehaviorTree/Task/BTTask_BossGroggy.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Base/Data/Character/VL_MonsterDataAsset.h"
// 본인 프로젝트 경로에 맞게 헤더를 인클루드 하세요.
#include "../../Boss/VL_Boss1.h"
#include "CustomLog/CustomLog.h"

UBTTask_BossGroggy::UBTTask_BossGroggy()
{
    NodeName = TEXT("Boss Groggy Task");
    INIT_TASK_NODE_NOTIFY_FLAGS();

    bNotifyTick = false;
}

EBTNodeResult::Type UBTTask_BossGroggy::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIC = OwnerComp.GetAIOwner();
    if (!AIC) return EBTNodeResult::Failed;

    AVL_AICharacterBase* Boss = Cast<AVL_AICharacterBase>(AIC->GetPawn());
    if (!Boss) return EBTNodeResult::Failed;


    if (Boss)
    {
        float GroggyTime = Boss->GetMonsterDataAsset()->GroggyDuration;

        // 캐릭터에게 모든 로직을 일임합니다.
        Boss->StartGroggyDuration(GroggyTime);

        // 태스크는 "진행 중" 상태로 남습니다.
        // 이후 캐릭터가 AIState를 Combat으로 바꾸면, 
        // 데코레이터(Observer Aborts Self)에 의해 태스크가 자동으로 종료됩니다.
        return EBTNodeResult::InProgress;
    }

    return EBTNodeResult::Failed;
}

void UBTTask_BossGroggy::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
    if (UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
    {
        ResetBossBlackboard(BB);
    }
}

EBTNodeResult::Type UBTTask_BossGroggy::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIC = OwnerComp.GetAIOwner();
    if (AVL_AICharacterBase* Boss = Cast<AVL_AICharacterBase>(AIC->GetPawn()))
    {
        // 태스크가 취소될 때 (예: 보스 사망 등) 캐릭터의 그로기 타이머와 상태를 강제로 정리
        Boss->EndGroggyState();
    }

    return Super::AbortTask(OwnerComp, NodeMemory);
}