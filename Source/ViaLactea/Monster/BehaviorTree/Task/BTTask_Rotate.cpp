#include "Monster/BehaviorTree/Task/BTTask_Rotate.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../../Boss/VL_Boss1.h"
#include "Base/Data/Character/VL_BossMonsterDataAsset.h"

#include "CustomLog/CustomLog.h"

UBTTask_Rotate::UBTTask_Rotate()
{
	NodeName = TEXT("Boss Rotate To Target");
	// TickTask를 사용하기 위해 true 설정
	bNotifyTick = false;
    bCreateNodeInstance = true;

    INIT_TASK_NODE_NOTIFY_FLAGS();

    //TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_Rotate, TargetActorKey), AActor::StaticClass());
}

EBTNodeResult::Type UBTTask_Rotate::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIC = OwnerComp.GetAIOwner();
    AVL_Boss1* Boss = Cast<AVL_Boss1>(AIC->GetPawn());
    if (!Boss || !AIC->GetFocusActor()) return EBTNodeResult::Failed;

    UVL_BossMonsterDataAsset* BossDataAsset = Boss->GetBossDataAsset();
    if (!BossDataAsset) return EBTNodeResult::Failed;

    // 1. 각도 계산 (DeltaYaw)
    FVector TargetLoc = AIC->GetFocusActor()->GetActorLocation();
    // 보스에서 타겟으로 향하는 방향 벡터 DirToTarget
    FVector DirToTarget = (TargetLoc - Boss->GetActorLocation()).GetSafeNormal();

    float DeltaYaw = FMath::FindDeltaAngleDegrees(Boss->GetActorRotation().Yaw, DirToTarget.Rotation().Yaw);

    // 데이터 에셋에서 적절한 몽타주 선택
    UAnimMontage* MontageToPlay = nullptr;

    if (DeltaYaw > 0.f)
    {
        // 양수이면 오른쪽으로 회전
        MontageToPlay = BossDataAsset->RightRotationMontage.LoadSynchronous();
    }
    else
    {

        // 음수이면 왼쪽으로 회전
        MontageToPlay = BossDataAsset->LeftRotationMontage.LoadSynchronous();
    }

    if (MontageToPlay)
    {
        CachedOwnerComp = &OwnerComp;
        CachedBoss = Boss;

        // 몽타주가 도는 동안 AI Controller가 강제로 몸을 돌리지 못하게 합니다.
      /*  Boss->GetCharacterMovement()->bUseControllerDesiredRotation = false;*/

        // 4. 몽타주 실행 및 종료 델리게이트 설정
        Boss->Multicast_PlayBossMontage(MontageToPlay);

        FOnMontageEnded EndDelegate;
        EndDelegate.BindUObject(this, &UBTTask_Rotate::OnRotateMontageEnded);
        Boss->GetMesh()->GetAnimInstance()->Montage_SetEndDelegate(EndDelegate, MontageToPlay);

        return EBTNodeResult::InProgress;
    }
    OnRotateMontageEnded(nullptr, true);
    return EBTNodeResult::Failed;
}

void UBTTask_Rotate::OnRotateMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (CachedOwnerComp)
    {
        FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
    }
}
void UBTTask_Rotate::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
    // 5. 공통 복구 로직 (성공/실패/중단 모든 경우)
    if (UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
    {
        BB->SetValueAsEnum(TEXT("AIBossPattern"), (uint8)EAIBossPattern::None);
        BB->SetValueAsEnum(TEXT("Reposition"), (uint8)EReposition::None);

        BB->SetValueAsBool(TEXT("BIsLocked"), true);
    }

    if (CachedBoss)
    {
    //    // AI에게 회전 제어권 복구
    //    CachedBoss->GetCharacterMovement()->bUseControllerDesiredRotation = true;

        if (UAnimInstance* AnimInst = CachedBoss->GetMesh()->GetAnimInstance())
        {
            FOnMontageEnded EmptyDelegate;
            AnimInst->Montage_SetEndDelegate(EmptyDelegate, nullptr);
        }
    }

    Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);

    CachedBoss = nullptr;
    CachedOwnerComp = nullptr;
}

EBTNodeResult::Type UBTTask_Rotate::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    // OnTaskFinished가 호출되므로 별도 복구 불필요
    return Super::AbortTask(OwnerComp, NodeMemory);
}