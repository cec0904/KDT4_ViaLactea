#include "Monster/BehaviorTree/Task/BTTask_AntiCloseBoom.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "../../Boss/VL_Boss1.h"
#include "Base/Data/Character/VL_BossMonsterDataAsset.h"
#include "Base/Data/VFX/VL_VFXDataAsset.h"
#include "Monster/Boss/Animation/BossAnimInstanceBase.h"

UBTTask_AntiCloseBoom::UBTTask_AntiCloseBoom()
{
    NodeName = TEXT("Anti Close Boom");
    // 중요: 델리게이트와 캐싱된 데이터를 안전하게 관리하기 위해 인스턴스 생성 활성화
    bCreateNodeInstance = true;
    INIT_TASK_NODE_NOTIFY_FLAGS();

}

EBTNodeResult::Type UBTTask_AntiCloseBoom::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    CachedOwnerComp = &OwnerComp;

    AAIController* AIC = OwnerComp.GetAIOwner();
    if (!AIC) return EBTNodeResult::Failed;

    AVL_Boss1* Boss = Cast<AVL_Boss1>(AIC->GetPawn());
    if (!Boss || !Boss->GetBossDataAsset()) return EBTNodeResult::Failed;

    // 1. 데이터 에셋에서 패턴 정보 가져오기
    const FBossPatternData* PData = Boss->GetBossDataAsset()->GetPatternDataByTag(LoopingVFXTag);
    if (!PData) return EBTNodeResult::Failed;

    // 2. 폭발 액터 스폰 (보스의 함수 호출)
    // 보스 위치에 스폰하거나 필요시 소켓 위치 등을 계산하여 전달
    Boss->SpawnExplosive(LoopingVFXTag, ExplosionVFXTag, Boss->GetActorLocation());

    // 3. 패턴 시작 상태 알림 (애니메이션/로직 상태 동기화)
    Boss->OnPatternStarted(PData->PatternType);

    // 4. 몽타주 재생 및 종료 델리게이트 바인딩
    UAnimMontage* MontageToPlay = PData->PatternMontage.LoadSynchronous();
    if (MontageToPlay)
    {
        Boss->Multicast_PlayBossMontage(MontageToPlay);

        UAnimInstance* AnimInstance = Boss->GetMesh()->GetAnimInstance();
        if (AnimInstance)
        {
            if (UVL_CharacterAnimInstanceBase* BossAnimInst = Cast<UVL_CharacterAnimInstanceBase>(AnimInstance))
            {
                BossAnimInst->bFullBody = true;

                FOnMontageEnded EndDelegate;
                EndDelegate.BindUObject(this, &UBTTask_AntiCloseBoom::OnMontageEnded);
                BossAnimInst->Montage_SetEndDelegate(EndDelegate, MontageToPlay);
            }

            return EBTNodeResult::InProgress;
        }
    }

    // 몽타주 재생 실패 시 Fallback 처리
    OwnerComp.GetBlackboardComponent()->SetValueAsEnum(TEXT("AIBossPattern"), (uint8)EAIBossPattern::None);
    return EBTNodeResult::Succeeded;
}

void UBTTask_AntiCloseBoom::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
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

void UBTTask_AntiCloseBoom::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (CachedOwnerComp)
    {
        // 블랙보드 패턴 상태 초기화

        // 몽타주가 끝났으므로 태스크 종료 보고
        // bInterrupted가 true면 (피격 등으로 끊김) Failed, 아니면 Succeeded
        FinishLatentTask(*CachedOwnerComp, bInterrupted ? EBTNodeResult::Failed : EBTNodeResult::Succeeded);
    }
}