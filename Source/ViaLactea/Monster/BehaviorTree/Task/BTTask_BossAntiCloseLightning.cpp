#include "Monster/BehaviorTree/Task/BTTask_BossAntiCloseLightning.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "Monster/Boss/VL_Boss1.h"
#include "Base/Data/Character/VL_BossMonsterDataAsset.h"
#include "Base/Data/VFX/VL_VFXDataAsset.h"
#include "CustomLog/CustomLog.h"

UBTTask_BossAntiCloseLightning::UBTTask_BossAntiCloseLightning()
{
	NodeName = "Boss Anti-Close Lightning";
    bCreateNodeInstance = true;
    INIT_TASK_NODE_NOTIFY_FLAGS();

}

EBTNodeResult::Type UBTTask_BossAntiCloseLightning::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    // 1. 보스 캐릭터 및 데이터 에셋 유효성 검사
    CachedOwnerComp = &OwnerComp;

    AAIController* AIC = OwnerComp.GetAIOwner();

    if (!AIC) return EBTNodeResult::Failed;

    AVL_Boss1* Boss = Cast<AVL_Boss1>(AIC->GetPawn());
    if (!Boss || !Boss->GetBossDataAsset()) return EBTNodeResult::Failed;

    const UVL_BossMonsterDataAsset* BossData = Boss->GetBossDataAsset();

    const FBossPatternData* PData = BossData->GetPatternDataByTag(LightningTag);
    
    if (!PData)
    {
        //CUSTOM_LOG("패턴 데이터 없음");
        return EBTNodeResult::Failed;
    }
    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!NavSystem) return EBTNodeResult::Failed;

    FVector Origin = Boss->GetActorLocation();
    SpawnLocations.Empty();


    for (int32 i = 0; i < PData->SpawnCount; ++i)
    {
        FNavLocation GroundLocation;
        bool bFoundValidLocation = false;

        float LightningMaxRange = PData->MaxRange;
        float LightningMinRange = LightningMaxRange / 3;
        // 1. 내비메시 위에서 유효한 '바닥' 좌표를 구합니다.
        for (int32 Attempt = 0; Attempt < 10; ++Attempt)
        {
            if (NavSystem->GetRandomReachablePointInRadius(Origin, LightningMaxRange, GroundLocation))
            {
                float DistSq = FVector::DistSquared(Origin, GroundLocation.Location);
                float MinRadiusSq = FMath::Square(LightningMinRange);
                if (DistSq >= MinRadiusSq)
                {
                    SpawnLocations.Add(GroundLocation.Location);
                    break; // 유효한 지점 발견!
                }
            }
        }
    }
    if (SpawnLocations.Num() <= 0)
    {
        //CUSTOM_LOG("유효지점없음");
        return EBTNodeResult::Failed;
    }
    Boss->OnPatternStarted(PData->PatternType);

    UAnimMontage* MontageToPlay = PData->PatternMontage.LoadSynchronous();

    if (MontageToPlay)
    {
        Boss->Multicast_PlayBossMontage(MontageToPlay);

        UAnimInstance* AnimInstance = Boss->GetMesh()->GetAnimInstance();
        if (AnimInstance)
        {
            FOnMontageEnded EndDelegate;
            EndDelegate.BindUObject(this, &UBTTask_BossAntiCloseLightning::OnMontageEnded);
            AnimInstance->Montage_SetEndDelegate(EndDelegate, MontageToPlay);

            return EBTNodeResult::InProgress;
        }

    }
    return EBTNodeResult::Failed;
}

void UBTTask_BossAntiCloseLightning::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
    if (!CachedOwnerComp.IsValid()) return;


    if (AAIController* AIC = OwnerComp.GetAIOwner())
    {
        if (AVL_Boss1* Boss = Cast<AVL_Boss1>(AIC->GetPawn()))
        {
            if (UAnimInstance* AnimInstance = Boss->GetMesh()->GetAnimInstance())
            {
                AnimInstance->OnMontageEnded.RemoveDynamic(this, &UBTTask_BossAntiCloseLightning::OnMontageEnded);
            }
        }

        if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
        {
            ResetBossBlackboard(BB);
        }
    }
    Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);

}

void UBTTask_BossAntiCloseLightning::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{

    EBTNodeResult::Type Result = bInterrupted ? EBTNodeResult::Failed : EBTNodeResult::Succeeded;

    if (!bInterrupted)
    {
        if (AAIController* AIC = CachedOwnerComp->GetAIOwner())
        {
            if (AVL_Boss1* Boss = Cast<AVL_Boss1>(AIC->GetPawn()))
            {
                for (const FVector& SpawnLoc : SpawnLocations)
                {
                    Boss->SpawnLightning(LightningTag, SpawnLoc);
                }
            }
        }
    }

    // 태스크 지연 종료 보고
    FinishLatentTask(*CachedOwnerComp, Result);
    
}
