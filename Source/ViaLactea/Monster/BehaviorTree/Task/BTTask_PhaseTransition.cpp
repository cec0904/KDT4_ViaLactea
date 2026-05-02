#include "Monster/BehaviorTree/Task/BTTask_PhaseTransition.h"
#include "AIController.h"
#include "../../Boss/VL_Boss1.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "CustomLog/CustomLog.h"
UBTTask_PhaseTransition::UBTTask_PhaseTransition()
{
    NodeName = TEXT("Boss Phase Change");
    bNotifyTick = false;

    bCreateNodeInstance = true;
    INIT_TASK_NODE_NOTIFY_FLAGS();

}

EBTNodeResult::Type UBTTask_PhaseTransition::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    CachedOwnerComp = &OwnerComp;

    AAIController* AIC = OwnerComp.GetAIOwner();
    AVL_Boss1* Boss = (AIC) ? Cast<AVL_Boss1>(AIC->GetPawn()) : nullptr;

    if (Boss)
    {
        EBossPhase NextPhase = (EBossPhase)AIC->GetBlackboardComponent()->GetValueAsEnum(TEXT("BossPhase"));
        if (NextPhase != EBossPhase::None)
        {
            // 바인딩 (이전 잔재가 있을 수 있으니 확실히 지우고 다시 등록)
            Boss->OnPhaseChangeFinished.RemoveDynamic(this, &UBTTask_PhaseTransition::FinishedChanging);
            Boss->OnPhaseChangeFinished.AddDynamic(this, &UBTTask_PhaseTransition::FinishedChanging);

            Boss->SetCurrentPhase(NextPhase);
            return EBTNodeResult::InProgress;
        }
    }

    // 실패 시 로직
    if (UBlackboardComponent* BB = CachedOwnerComp->GetBlackboardComponent())
    {
        ResetBossBlackboard(BB);
    }
    return EBTNodeResult::Failed;
}

void UBTTask_PhaseTransition::FinishedChanging()
{
    if (!CachedOwnerComp.IsValid()) return;

    AAIController* AIC = CachedOwnerComp->GetAIOwner();
    if (!AIC) return;

    if (AVL_Boss1* Boss = Cast<AVL_Boss1>(AIC->GetPawn()))
    {
        Boss->OnPhaseChangeFinished.RemoveDynamic(this, &UBTTask_PhaseTransition::FinishedChanging);
    }

    // 2. 블랙보드 업데이트
    if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
    {
        ResetBossBlackboard(BB);
    }

    // 3. 태스크 정상 종료
    FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
}

// 4. [추가] 페이즈 전환 도중 강제 중단되었을 때의 방어 로직
EBTNodeResult::Type UBTTask_PhaseTransition::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIC = OwnerComp.GetAIOwner();
    if (AIC)
    {
        if (AVL_Boss1* Boss = Cast<AVL_Boss1>(AIC->GetPawn()))
        {
            // 중단될 때도 무조건 델리게이트를 해제하여 좀비 호출을 막습니다.
            Boss->OnPhaseChangeFinished.RemoveDynamic(this, &UBTTask_PhaseTransition::FinishedChanging);
        }
    }

    return EBTNodeResult::Aborted;
}

void UBTTask_PhaseTransition::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
    if (AAIController* AIC = OwnerComp.GetAIOwner())
    {
        if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
        {
            ResetBossBlackboard(BB);
        }
    }
    Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}


