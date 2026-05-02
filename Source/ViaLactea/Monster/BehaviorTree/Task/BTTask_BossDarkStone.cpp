#include "Monster/BehaviorTree/Task/BTTask_BossDarkStone.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "../../Boss/VL_Boss1.h"
#include "Base/Data/Character/VL_BossMonsterDataAsset.h"

UBTTask_BossDarkStone::UBTTask_BossDarkStone()
{
    NodeName = TEXT("Boss Dark Stone");
    INIT_TASK_NODE_NOTIFY_FLAGS();

}

EBTNodeResult::Type UBTTask_BossDarkStone::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    CachedOwnerComp = &OwnerComp;

    AAIController* AIC = OwnerComp.GetAIOwner();

    if (!AIC) return EBTNodeResult::Failed;
    ACharacter* Target = Cast<ACharacter>(AIC->GetBlackboardComponent()->GetValueAsObject(TEXT("TargetActor")));
    if (!Target)
    {
        OnMontageEnded(nullptr, true);
        return EBTNodeResult::Failed;
    }
    AVL_Boss1* Boss = Cast<AVL_Boss1>(AIC->GetPawn());

    if (!Boss || !Boss->GetBossDataAsset()) return EBTNodeResult::Failed;

    const UVL_BossMonsterDataAsset* BossData = Boss->GetBossDataAsset();
    const FBossPatternData* PData = BossData->GetPatternDataByTag(DarkStoneTag);
    if (!PData) return EBTNodeResult::Failed;

    Boss->OnPatternStarted(PData->PatternType);
    // 거대 운석 위치잡고 소환
    FVector BossLoc = Boss->GetActorLocation();
    FVector TargetLoc = Target->GetActorLocation();

    // 1. 타겟을 향한 방향 벡터 계산 (Z축은 무시하여 수평 방향만 추출)
    FVector DirectionToTarget = (TargetLoc - BossLoc);
    DirectionToTarget.Z = 0.f;
    DirectionToTarget = DirectionToTarget.GetSafeNormal(); // 정규화 (길이를 1로 만듦)

    // 2. 소환 위치 설정
    float ForwardOffset = 500.f; // 보스 앞쪽으로 얼마나 떨구고 싶은지 거리값
    SpawnHeight = 1500.f;  // 소환 높이

    // 보스 위치 + (방향 * 거리) + (높이)
    FVector SpawnLocation = BossLoc + FVector(0.f, 0.f, SpawnHeight);
    Boss->SpawnMeteor(DarkStoneTag, SpawnLocation);


    UAnimMontage* MontageToPlay = PData->PatternMontage.LoadSynchronous();

    if (MontageToPlay)
    {
        Boss->Multicast_PlayBossMontage(MontageToPlay);
    }
    UAnimInstance* AnimInstance = Boss->GetMesh()->GetAnimInstance();
    if (AnimInstance)
    {
        FOnMontageEnded EndDelegate;
        EndDelegate.BindUObject(this, &UBTTask_BossDarkStone::OnMontageEnded);
        AnimInstance->Montage_SetEndDelegate(EndDelegate, MontageToPlay);

        return EBTNodeResult::InProgress;
    }
    OwnerComp.GetBlackboardComponent()->SetValueAsEnum(TEXT("AIBossPattern"), (uint8)EAIBossPattern::None);
    return EBTNodeResult::Failed;
}

void UBTTask_BossDarkStone::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
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

void UBTTask_BossDarkStone::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (CachedOwnerComp)
    {
        AAIController* AIC = CachedOwnerComp->GetAIOwner();
        
        if (!AIC) return ;

        AVL_Boss1* Boss = Cast<AVL_Boss1>(AIC->GetPawn());
        if (!Boss || !Boss->GetBossDataAsset()) return ;

       
        //// 거대 운석 위치잡고 소환
        //FVector SpawnLocation = Boss->GetActorLocation();
        //SpawnLocation = SpawnLocation + FVector(0.f, 0.f, 2000.f);

        //Boss->SpawnMeteor(DarkStoneTag, SpawnLocation);

        // 몽타주가 강제로 끊겼다면(bInterrupted) Failed를, 정상 종료되었다면 Succeeded를 반환합니다.
        EBTNodeResult::Type Result = bInterrupted ? EBTNodeResult::Failed : EBTNodeResult::Succeeded;

        // 태스크 지연 종료 보고
        FinishLatentTask(*CachedOwnerComp, Result);
    }
}
