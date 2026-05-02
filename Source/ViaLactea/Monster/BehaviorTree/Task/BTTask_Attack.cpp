#include "BTTask_Attack.h"
#include "Base/FrameWork/VL_AIControllerBase.h"
//#include "Base/Character/VL_AICharacterBase.h"
#include "Monster/Boss/VL_Boss1.h"
#include "Base/Data/Character/VL_BossMonsterDataAsset.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Base/AI/AIState.h"

#include "CustomLog/CustomLog.h"

UBTTask_Attack::UBTTask_Attack()
{
    NodeName = TEXT("Attack"); // 비헤이비어 트리 노드에 표시될 이름
    INIT_TASK_NODE_NOTIFY_FLAGS();
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AVL_AICharacterBase* AIChar = Cast<AVL_AICharacterBase>(OwnerComp.GetAIOwner()->GetPawn());
    if (!AIChar) return EBTNodeResult::Failed;
    

    AIChar->OnAttackFinished.RemoveAll(this);

    TWeakObjectPtr<UBehaviorTreeComponent> WeakOwnerComp(&OwnerComp);

    AIChar->OnAttackFinished.AddUObject(this, &UBTTask_Attack::OnAttackFinishedCallback, WeakOwnerComp);

    if (AIChar->Attack()) 
    {
        return EBTNodeResult::InProgress;
    }
    //else
    //{
    //    //CUSTOM_LOG("공격 실패");
    //}
    AIChar->OnAttackFinished.RemoveAll(this);
    return EBTNodeResult::Failed;
}

void UBTTask_Attack::OnAttackFinishedCallback(TWeakObjectPtr<UBehaviorTreeComponent> OwnerCompPtr)
{
    if (UBehaviorTreeComponent* OwnerComp = OwnerCompPtr.Get())
    {
        // 노드가 현재 실행 중인지 확인
        if (OwnerComp->GetActiveInstanceIdx() != INDEX_NONE)
        {
            AAIController* AIC = OwnerComp->GetAIOwner();
            
            if (!AIC ) return;
            if (AVL_AICharacterBase* AIChar = Cast<AVL_AICharacterBase>(AIC->GetPawn()))
            {
                // 모든 몬스터(보스 포함)가 공통으로 가져야 할 정리 로직
                AIChar->OnAttackFinished.RemoveAll(this);

                // 3. [특수] 그중에서 '보스'일 때만 수행해야 하는 작업 (블랙보드 키 처리 등)
            }

            FinishLatentTask(*OwnerComp, EBTNodeResult::Succeeded);
        }
    }
}

EBTNodeResult::Type UBTTask_Attack::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    //CUSTOM_LOG("공격 태스크 중단됨");

    if (AAIController* AIC = OwnerComp.GetAIOwner())
    {
        if (AVL_AICharacterBase* AIChar = Cast<AVL_AICharacterBase>(AIC->GetPawn()))
        {
            // 델리게이트 해제 및 애니메이션 중단
            AIChar->OnAttackFinished.RemoveAll(this);
            AIChar->StopAttack();
        }
    }

    // Abort 처리가 즉시 끝난다면 Aborted를 반환합니다.
    return EBTNodeResult::Aborted;
}

void UBTTask_Attack::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
    if (AAIController* AIC = OwnerComp.GetAIOwner())
    {
        if (AVL_AICharacterBase* AIChar = Cast<AVL_AICharacterBase>(AIC->GetPawn()))
        {
            AIChar->OnAttackFinished.RemoveAll(this);

            UBlackboardComponent* BB = AIC->GetBlackboardComponent();

            const FName BossTag = FName(TEXT("Boss1"));

            if (AIChar->ActorHasTag(BossTag))
            {
                // 보스만 가지고 있는 전용 블랙보드 키 "AIBossPattern" 초기화
                ResetBossBlackboard(BB);
            }
            else
            {
                BB->SetValueAsBool(TEXT("bIsLocked"), false);

            }
        }
    }

    Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}