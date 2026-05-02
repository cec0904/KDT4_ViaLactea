#include "Monster/BehaviorTree/Task/BTTask_BossLaser.h"

#include "AIController.h"

#include "BehaviorTree/BlackboardComponent.h"

#include "../../Boss/VL_Boss1.h"
#include "Base/Data/Character/VL_BossMonsterDataAsset.h"
#include "Base/Data/VFX/VL_VFXDataAsset.h"

UBTTask_BossLaser::UBTTask_BossLaser()
{
    NodeName = TEXT("Boss Laser Attack");
    INIT_TASK_NODE_NOTIFY_FLAGS();
    bNotifyTick = false;
}

EBTNodeResult::Type UBTTask_BossLaser::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    CachedOwnerComp = &OwnerComp;
    AAIController* AIC = OwnerComp.GetAIOwner();
    if (!AIC) return EBTNodeResult::Failed;

    AVL_Boss1* Boss = Cast<AVL_Boss1>(AIC->GetPawn());
    if (!Boss || !Boss->GetBossDataAsset()) return EBTNodeResult::Failed;

    // 1. 데이터 에셋에서 레이저 패턴 데이터 가져오기
    const UVL_BossMonsterDataAsset* BossData = Boss->GetBossDataAsset();
    const FBossPatternData* PData = BossData->GetPatternDataByTag(LaserTag);
    if (!PData) return EBTNodeResult::Failed;

    // 2. 타겟 위치 파악 (블랙보드에서 타겟 액터나 위치를 가져옴)
    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB) return EBTNodeResult::Failed;
    AActor* TargetActor = Cast<AActor>(BB->GetValueAsObject(TEXT("TargetActor")));

    if (TargetActor)
    {
        Boss->SpawnLaser(LaserTag, TargetActor);
    }


    // 4. 패턴 시작 알림 및 애니메이션 재생
    Boss->OnPatternStarted(PData->PatternType);

    GetWorld()->GetTimerManager().ClearTimer(LaserTimerHandle);

    TWeakObjectPtr<UBehaviorTreeComponent> OwnerCompPtr(&OwnerComp);

    // &OwnerComp를 캡처하여 람다 내부에서 태스크 종료를 알림
    GetWorld()->GetTimerManager().SetTimer(LaserTimerHandle, [this, OwnerCompPtr]()
        {
            if (OwnerCompPtr.IsValid())
            {
                FinishLatentTask(*OwnerCompPtr, EBTNodeResult::Succeeded);
            }

        }, PData->Duration, false);

    return EBTNodeResult::InProgress;
}

EBTNodeResult::Type UBTTask_BossLaser::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    // 태스크가 중단될 때 타이머를 제거하여 FinishLatentTask가 뒤늦게 호출되는 것을 방지
    GetWorld()->GetTimerManager().ClearTimer(LaserTimerHandle);


    return Super::AbortTask(OwnerComp, NodeMemory);
}

void UBTTask_BossLaser::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
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

//void UBTTask_BossLaser::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
//{
//    if (CachedOwnerComp)
//    {
//        // 공격 종료 후 블랙보드 상태 초기화
//        UBlackboardComponent* BB = CachedOwnerComp->GetBlackboardComponent();
//        if (BB)
//        {
//            BB->SetValueAsEnum(TEXT("AIBossPattern"), (uint8)EAIBossPattern::None);
//        }
//
//        EBTNodeResult::Type Result = bInterrupted ? EBTNodeResult::Failed : EBTNodeResult::Succeeded;
//
//        // 태스크 지연 종료 보고
//        FinishLatentTask(*CachedOwnerComp, Result);
//    }
//}
