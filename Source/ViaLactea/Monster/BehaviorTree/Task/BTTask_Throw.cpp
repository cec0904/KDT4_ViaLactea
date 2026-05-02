#include "Monster/BehaviorTree/Task/BTTask_Throw.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "Monster/Boss/VL_Boss1.h"
#include "Base/Data/Character/VL_BossMonsterDataAsset.h"

#include "CustomLog/CustomLog.h"

UBTTask_Throw::UBTTask_Throw()
{
	NodeName = "Throw Pattern Task";
	bCreateNodeInstance = true;
	INIT_TASK_NODE_NOTIFY_FLAGS();
}

EBTNodeResult::Type UBTTask_Throw::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AVL_Boss1* Boss = Cast<AVL_Boss1>(OwnerComp.GetAIOwner()->GetPawn());
    if (!Boss) return EBTNodeResult::Failed;

    UVL_BossMonsterDataAsset* Data = Boss->GetBossDataAsset();
    if (!Data) return EBTNodeResult::Failed;

    const FBossPatternData* PData = Data->GetPatternDataByTag(ThrowTag);
    if (!PData) return EBTNodeResult::Failed;

    if (!PData->PatternMontage.IsNull())
    {
        UAnimMontage* Montage = PData->PatternMontage.LoadSynchronous();

        if (Montage)
        {
            MyPlayingMontage = Montage;
            UAnimInstance* AnimInstance = Boss->GetMesh()->GetAnimInstance();
            if (AnimInstance)
            {
                CachedOwnerComp = &OwnerComp;

                AnimInstance->OnMontageEnded.RemoveDynamic(this, &UBTTask_Throw::OnMontageEnded);
                AnimInstance->OnMontageEnded.AddDynamic(this, &UBTTask_Throw::OnMontageEnded);

                Boss->OnPatternStarted(PData->PatternType);

                // 3. 몽타주 재생
                Boss->Multicast_PlayBossMontage(Montage);

                // 태스크를 완료하지 않고 '진행 중' 상태로 둠
                return EBTNodeResult::InProgress;
            }
        }
    }
    return EBTNodeResult::Failed;
}

void UBTTask_Throw::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (Montage != MyPlayingMontage)
    {
        return;
    }

    if (!CachedOwnerComp.IsValid()) return;

    // 2. 델리게이트 즉시 해제 (중복 호출 및 메모리 누수 방지)
    if (AAIController* MyController = CachedOwnerComp->GetAIOwner())
    {
        if (ACharacter* Boss = Cast<ACharacter>(MyController->GetPawn()))
        {
            if (UAnimInstance* AnimInstance = Boss->GetMesh()->GetAnimInstance())
            {
                AnimInstance->OnMontageEnded.RemoveDynamic(this, &UBTTask_Throw::OnMontageEnded);
            }
        }
    }
    
    EBTNodeResult::Type Result = bInterrupted ? EBTNodeResult::Aborted : EBTNodeResult::Succeeded;
    //CUSTOM_LOG(" 몽타주 종료 델리게이트 반환  Result : %d", Result);
    // 이 함수가 불려야 BT의 노란 불이 꺼집니다.
    FinishLatentTask(*CachedOwnerComp, Result);
}

EBTNodeResult::Type UBTTask_Throw::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    // 태스크가 중단될 때도 델리게이트를 안전하게 해제해야 함
    AVL_Boss1* Boss = Cast<AVL_Boss1>(OwnerComp.GetAIOwner()->GetPawn());
    if (Boss)
    {
        // 1. 델리게이트 먼저 해제 (중복 호출 방지)
        if (UAnimInstance* AnimInstance = Boss->GetMesh()->GetAnimInstance())
        {
            AnimInstance->OnMontageEnded.RemoveDynamic(this, &UBTTask_Throw::OnMontageEnded);
        }
        // 2. 몽타주 즉시 중단 (0.2초 정도 블렌드 아웃을 주면 부드럽게 멈춥니다)
        Boss->StopBossMontage(0.2f);

        //무기본 켜주기
        TArray<FName> Bones = { FName(TEXT("weapon_l")), FName(TEXT("weapon_r")) };
        
        Boss->Multicast_HideBones(Bones, false);
    }

    return EBTNodeResult::Aborted;
}

void UBTTask_Throw::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
    //CUSTOM_LOG("태스크 종료. 결과: %d", (int32)TaskResult);

    if (AAIController* AIC = OwnerComp.GetAIOwner())
    {
        UBlackboardComponent* BB = AIC->GetBlackboardComponent();
        if (BB)
        {
            ResetBossBlackboard(BB);
        }
    }
    Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}