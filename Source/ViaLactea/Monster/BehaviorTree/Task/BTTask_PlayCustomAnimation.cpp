#include "Monster/BehaviorTree/Task/BTTask_PlayCustomAnimation.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "AIController.h"
#include "Base/Character/VL_AICharacterBase.h"
#include "Animation/AnimInstance.h"

#include "Base/AI/AIState.h"

#include "CustomLog/CustomLog.h"

UBTTask_PlayCustomAnimation::UBTTask_PlayCustomAnimation()
{
    NodeName = "Play Custom Animation";
    bNotifyTick = false;
    bCreateNodeInstance = true;
    INIT_TASK_NODE_NOTIFY_FLAGS();

}

EBTNodeResult::Type UBTTask_PlayCustomAnimation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIC = OwnerComp.GetAIOwner();
    if (!AIC) return EBTNodeResult::Failed;

    AVL_AICharacterBase* AIChar = Cast<AVL_AICharacterBase>(AIC->GetPawn());
    if (!AIChar || !MontageToPlay) return EBTNodeResult::Failed;

    UAnimInstance* AnimInst = AIChar->GetMesh()->GetAnimInstance();
    if (AnimInst)
    {
        float Duration = AnimInst->Montage_Play(MontageToPlay);

        if (Duration > 0.f)
        {
            TWeakObjectPtr<UBehaviorTreeComponent> OwnerCompPtr(&OwnerComp);

            // 3. 더 안전한 델리게이트 바인딩
            FOnMontageEnded EndDelegate;
            EndDelegate.BindLambda([this, OwnerCompPtr](UAnimMontage* Montage, bool bInterrupted)
                {
                    if (OwnerCompPtr.IsValid())
                    {
                        // 중단(Interrupted)되었을 때는 AbortTask에서 처리하므로, 
                        // 정상 종료 시에만 Succeeded를 호출
                        if (!bInterrupted)
                        {
                            FinishLatentTask(*OwnerCompPtr, EBTNodeResult::Succeeded);
                        }
                    }
                });

            AnimInst->Montage_SetEndDelegate(EndDelegate, MontageToPlay);
            return EBTNodeResult::InProgress;
        }
    }
    //CUSTOM_LOG("애니메이션 실패");
    return EBTNodeResult::Failed;
}

EBTNodeResult::Type UBTTask_PlayCustomAnimation::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIC = OwnerComp.GetAIOwner();
    if (AIC)
    {
        ACharacter* Char = Cast<ACharacter>(AIC->GetPawn());
        if (!Char) return Super::AbortTask(OwnerComp, NodeMemory);

        AIC->GetBlackboardComponent()->SetValueAsEnum(TEXT("AIState"), (uint8)EAIState::IsReturning);
        Char->StopAnimMontage(MontageToPlay);
    }
    FinishLatentTask(OwnerComp, EBTNodeResult::Aborted);
    // 부모의 AbortTask를 호출하여 태스크가 정식으로 Aborted 상태가 되도록 함
    return EBTNodeResult::Aborted;
}


void UBTTask_PlayCustomAnimation::OnAnimationFinished(UBehaviorTreeComponent* OwnerComp)
{
    if (OwnerComp)
    {
        FinishLatentTask(*OwnerComp, EBTNodeResult::Succeeded);
    }
}