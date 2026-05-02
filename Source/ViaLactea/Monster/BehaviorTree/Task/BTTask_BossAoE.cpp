
#include "Monster/BehaviorTree/Task/BTTask_BossAoE.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "../../Boss/VL_Boss1.h"
#include "Base/Data/Character/VL_BossMonsterDataAsset.h"
#include "Base/Data/VFX/VL_VFXDataAsset.h"



#include "CustomLog/CustomLog.h"

UBTTask_BossAoE::UBTTask_BossAoE()
{
    NodeName = "Boss AoE Pattern Task";
    INIT_TASK_NODE_NOTIFY_FLAGS();
}

EBTNodeResult::Type UBTTask_BossAoE::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    // 1. 보스 캐릭터 및 데이터 에셋 유효성 검사
    CachedOwnerComp = &OwnerComp;

    AAIController* AIC = OwnerComp.GetAIOwner();

    if (!AIC) return EBTNodeResult::Failed;

    AVL_Boss1* Boss = Cast<AVL_Boss1>(AIC->GetPawn());
    if (!Boss || !Boss->GetBossDataAsset()) return EBTNodeResult::Failed;

    const UVL_BossMonsterDataAsset* BossData = Boss->GetBossDataAsset();
    const FBossPatternData* PData = BossData->GetPatternDataByTag(AoETag);
    if (!PData) return EBTNodeResult::Failed;

    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!NavSystem) return EBTNodeResult::Failed;

    FVector Origin = Boss->GetActorLocation();
    int32 SuccessCount = 0;

    float MinExcludeRadius = 10.f * BossData->MonsterScale;
    // 데이터 에셋에 정의된 SpawnCount 만큼 반복

    for (int32 i = 0; i < PData->SpawnCount; ++i)
    {
        FNavLocation GroundLocation;
        bool bFoundValidLocation = false;

        // 1. 내비메시 위에서 유효한 '바닥' 좌표를 구합니다.
        for (int32 Attempt = 0; Attempt < 10; ++Attempt)
        {
            if (NavSystem->GetRandomReachablePointInRadius(Origin, PData->MaxRange, GroundLocation))
            {
                float DistSq = FVector::DistSquared(Origin, GroundLocation.Location);
                float MinRadiusSq = FMath::Square(MinExcludeRadius);
                if (DistSq >= MinRadiusSq)
                {
                    bFoundValidLocation = true;
                    break; // 유효한 지점 발견!
                }

            }


        }
        if (bFoundValidLocation)
        {
            float SpawnHeight = 2500.0f;
            float SpawnXaxis = -1500.f;

            FVector SkyLocation = GroundLocation.Location + FVector(SpawnXaxis, 0.f, SpawnHeight);

            Boss->SpawnMeteor(AoETag, SkyLocation);
            SuccessCount++;
        }


    }
    if (SuccessCount <= 0) return EBTNodeResult::Failed;

    Boss->OnPatternStarted(PData->PatternType);

    MontageToPlay = PData->PatternMontage.LoadSynchronous();

    if (MontageToPlay)
    {
        Boss->Multicast_PlayBossMontage(MontageToPlay);
    }

    UAnimInstance* AnimInstance = Boss->GetMesh()->GetAnimInstance();
    if (AnimInstance)
    {
        FOnMontageEnded EndDelegate;
        EndDelegate.BindUObject(this, &UBTTask_BossAoE::OnMontageEnded);
        AnimInstance->Montage_SetEndDelegate(EndDelegate, MontageToPlay);

        return EBTNodeResult::InProgress;
    }
    return EBTNodeResult::Succeeded;
}

void UBTTask_BossAoE::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (CachedOwnerComp)
    {

        // 몽타주가 강제로 끊겼다면(bInterrupted) Failed를, 정상 종료되었다면 Succeeded를 반환합니다.
        EBTNodeResult::Type Result = bInterrupted ? EBTNodeResult::Failed : EBTNodeResult::Succeeded;

        // 태스크 지연 종료 보고
        FinishLatentTask(*CachedOwnerComp, Result);
    }
}

EBTNodeResult::Type UBTTask_BossAoE::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIC = OwnerComp.GetAIOwner();
    if (AIC)
    {
        if (AVL_AICharacterBase* Boss = Cast<AVL_AICharacterBase>(AIC->GetPawn()))
        {
            // 1. 델리게이트 해제 (중복 호출 방지)
            if (UAnimInstance* AnimInstance = Boss->GetMesh()->GetAnimInstance())
            {
                FOnMontageEnded EmptyDelegate; // 빈 델리게이트 변수 생성
                AnimInstance->Montage_SetEndDelegate(EmptyDelegate, MontageToPlay);
            }

            // 2. 몽타주 즉시 중단 (범용 함수 활용)
            Boss->StopAllMontage(0.2f);
        }
    }

    return Super::AbortTask(OwnerComp, NodeMemory);
}

void UBTTask_BossAoE::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
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

