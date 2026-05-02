#include "Monster/BehaviorTree/Task/BTTask_Groggy.h"
#include "Base/Character/VL_AICharacterBase.h"
#include "Base/Data/Character/VL_MonsterDataAsset.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"



UBTTask_Groggy::UBTTask_Groggy()
{
    NodeName = "Play Groggy";
    bNotifyTick = false;
    bCreateNodeInstance = true;
    INIT_TASK_NODE_NOTIFY_FLAGS();

}

EBTNodeResult::Type UBTTask_Groggy::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AVL_AICharacterBase* AIChar = Cast<AVL_AICharacterBase>(OwnerComp.GetAIOwner()->GetPawn());
    if (!AIChar) return EBTNodeResult::Failed;

    // 1. 멀티캐스트 호출 (모든 클라이언트에서 애니메이션 재생)
    AIChar->Multicast_PlayGroggy();

    // 2. 서버(태스크)에서도 종료 시점을 알기 위해 델리게이트 바인딩
    UAnimInstance* AnimInst = AIChar->GetMesh()->GetAnimInstance();
    UAnimMontage* GroggyAnim = AIChar->GetMonsterDataAsset()->MonsterGroggyMontage.LoadSynchronous();

    if (AnimInst && GroggyAnim)
    {
        TWeakObjectPtr<UBehaviorTreeComponent> OwnerCompPtr(&OwnerComp);

        FOnMontageEnded EndDelegate;
        EndDelegate.BindLambda([this, OwnerCompPtr, AIChar](UAnimMontage* Montage, bool bInterrupted)
            {
                if (!OwnerCompPtr.IsValid() || !AIChar)
                {
                    return;
                }
                // 3. 애니메이션 종료 후 복구 로직 (기존 OnGroggyMontageEnded 내용)
                AIChar->SetGroggy(false);
                AIChar->EnableMovement(); // 속도 복구 등

                UBlackboardComponent* BB = OwnerCompPtr->GetBlackboardComponent();
                if (BB)
                {
                    BB->SetValueAsEnum(TEXT("AIState"), (uint8)EAIState::Idle);
                    BB->SetValueAsEnum(TEXT("AIBossPattern"), (uint8)EAIBossPattern::None);
                    
                }

                FinishLatentTask(*OwnerCompPtr, EBTNodeResult::Succeeded);
            });

        AnimInst->Montage_SetEndDelegate(EndDelegate, GroggyAnim);
        return EBTNodeResult::InProgress;
    }

    return EBTNodeResult::Failed;
}

EBTNodeResult::Type UBTTask_Groggy::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIC = OwnerComp.GetAIOwner();
    if (AIC)
    {
        if (AVL_AICharacterBase* AIChar = Cast<AVL_AICharacterBase>(AIC->GetPawn()))
        {
            if (UAnimInstance* AnimInst = AIChar->GetMesh()->GetAnimInstance())
            {
                FOnMontageEnded EmptyDel;
                AnimInst->Montage_SetEndDelegate(EmptyDel, AIChar->GetMonsterDataAsset()->MonsterGroggyMontage.LoadSynchronous());
            }

            // 애니메이션 중단 및 상태 초기화
            AIChar->StopAnimMontage();
            AIChar->SetGroggy(false);
        }
    }

    return Super::AbortTask(OwnerComp, NodeMemory);
}
