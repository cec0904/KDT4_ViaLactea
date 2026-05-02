#include "Monster/BehaviorTree/Task/BTTask_RotationAttack.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "../../Boss/VL_Boss1.h"
#include "Base/Data/Character/VL_BossMonsterDataAsset.h"
#include "AIController.h"

#include "CustomLog/CustomLog.h"


UBTTask_RotationAttack::UBTTask_RotationAttack()
{
    NodeName = "Rotation Attack";
    bNotifyTick = false;
    bCreateNodeInstance = true;

    INIT_TASK_NODE_NOTIFY_FLAGS();
}

EBTNodeResult::Type UBTTask_RotationAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
  
    AAIController* AIC = OwnerComp.GetAIOwner();
    AVL_Boss1* Boss = AIC ? Cast<AVL_Boss1>(AIC->GetPawn()) : nullptr;
    if (!Boss) return EBTNodeResult::Failed;

    UAnimInstance* AnimInstance = Boss->GetMesh()->GetAnimInstance();
    if (!AnimInstance) return EBTNodeResult::Failed;

    CachedOwnerComp = &OwnerComp;
    if (RotationAttackMontage)
    {
        // 1. 몽타주 길이는 RPC 호출 전에 에셋으로부터 직접 가져올 수 있습니다.
        float Duration = RotationAttackMontage->GetPlayLength();

        // 2. 멀티캐스트 RPC 호출 (반환값 없음, Fire & Forget)
        Boss->Multicast_PlayBossMontage(RotationAttackMontage);

        // 3. 델리게이트 바인딩 (이전 답변 내용 참조)
        AnimInstance->OnMontageEnded.RemoveDynamic(this, &UBTTask_RotationAttack::OnMontageEnded);
        AnimInstance->OnMontageEnded.AddDynamic(this, &UBTTask_RotationAttack::OnMontageEnded);

        // 길이를 이용한 추가 로직이 필요하다면 여기서 사용하세요.
        // 예: Duration이 0보다 크면 InProgress 반환
        return EBTNodeResult::InProgress;
    }
    return EBTNodeResult::Failed;
}

EBTNodeResult::Type UBTTask_RotationAttack::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIC = OwnerComp.GetAIOwner();
    AVL_Boss1* Boss = AIC ? Cast<AVL_Boss1>(AIC->GetPawn()) : nullptr;

    if (Boss)
    {
        // 1. 애니메이션 강제 중단 (원한다면)
        Boss->StopAnimMontage(RotationAttackMontage);

        // 2. [가장 중요] 델리게이트 제거
        if (UAnimInstance* AnimInstance = Boss->GetMesh()->GetAnimInstance())
        {
            AnimInstance->OnMontageEnded.RemoveDynamic(this, &UBTTask_RotationAttack::OnMontageEnded);
        }
    }

    // 3. 태스크 정리
    // 블랙보드 값 초기화는 상황에 따라 선택 (여기서는 None으로 초기화하는 게 안전)
    if (UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
    {
        ResetBossBlackboard(BB);
    }

    return Super::AbortTask(OwnerComp, NodeMemory);
}

// 별도 정의된 콜백 함수
void UBTTask_RotationAttack::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    // 현재 재생된 몽타주인지 확인
    if (Montage != RotationAttackMontage) return;

    if (CachedOwnerComp)
    {
        // 1. 델리게이트 제거 (메모리 및 로직 안전)
        AAIController* AIC = CachedOwnerComp->GetAIOwner();
        if (AIC)
        {
            ACharacter* Boss = Cast<ACharacter>(AIC->GetPawn());
            if (Boss && Boss->GetMesh())
            {
                UAnimInstance* AnimInstance = Boss->GetMesh()->GetAnimInstance();
                if (AnimInstance)
                {
                    AnimInstance->OnMontageEnded.RemoveDynamic(this, &UBTTask_RotationAttack::OnMontageEnded);
                }
            }
        }

        // 2. 블랙보드 값 초기화 및 태스크 종료
        if (UBlackboardComponent* BB = CachedOwnerComp->GetBlackboardComponent())
        {
            ResetBossBlackboard(BB);
        }
        FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
    }
}