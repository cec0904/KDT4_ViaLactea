#include "Monster/BehaviorTree/Task/BTTask_Strafe.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "BehaviorTree/BTFunctionLibrary.h"
#include "Navigation/PathFollowingComponent.h"

#include "NavigationSystem.h"
#include "../../Boss/VL_Boss1.h"
#include "Base/Data/Character/VL_BossMonsterDataAsset.h"
#include "Base/AI/AIBossPattern.h"


#include "CustomLog/CustomLog.h"

UBTTask_Strafe::UBTTask_Strafe()
{
	NodeName = TEXT("Strafe");

	bNotifyTick = false;
	bCreateNodeInstance = true;
    INIT_TASK_NODE_NOTIFY_FLAGS();


	StrafeLength = 500.0f;
	StrafeSpeed = 200.0f;
}

EBTNodeResult::Type UBTTask_Strafe::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIC = OwnerComp.GetAIOwner();
    APawn* Pawn = AIC ? AIC->GetPawn() : nullptr;

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    AActor* Target = Cast<AActor>(BB ? BB->GetValueAsObject("TargetActor") : nullptr);

    if (!AIC || !Pawn || !Target || !BB)
    {
        return EBTNodeResult::Failed;
    }

    AVL_Boss1* Boss = Cast<AVL_Boss1>(Pawn);
    if (!Boss) return EBTNodeResult::Failed;

    Boss->GetCharacterMovement()->bUseControllerDesiredRotation = true;
    Boss->GetCharacterMovement()->MaxWalkSpeed = StrafeSpeed;


    // 1. 방향 계산
    auto CalculateDest = [&](bool bRight) -> FVector
        {
            FVector PawnLoc = Boss->GetActorLocation();
            FVector TargetLoc = Target->GetActorLocation();
            FVector ToTarget = (TargetLoc - PawnLoc).GetSafeNormal2D(); // 2D 평면 방향
            FVector SideVector = FVector::CrossProduct(ToTarget, FVector::UpVector);
            if (!bRight) SideVector *= -1.0f;

            float CurrentDist = FVector::Dist2D(PawnLoc, TargetLoc);
            FVector Adjustment = ToTarget * (CurrentDist > StrafeLength + 100.f ? 200.f : (CurrentDist < StrafeLength - 100.f ? -200.f : 0.f));

            return PawnLoc + (SideVector * StrafeLength) + Adjustment;
        };

    bStrafeRight = FMath::RandBool();
    FVector FinalDest = CalculateDest(bStrafeRight);

    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    FNavLocation NextNavLoc;
    FVector CheckExtent(100.f, 100.f, 500.f);

    if (!NavSys || !NavSys->ProjectPointToNavigation(FinalDest, NextNavLoc, CheckExtent))
    {
        // 반대 방향 재시도
        bStrafeRight = !bStrafeRight;
        FinalDest = CalculateDest(bStrafeRight);
        if (!NavSys->ProjectPointToNavigation(FinalDest, NextNavLoc, CheckExtent))
        {
            ResetMovementSettings(OwnerComp);
            return EBTNodeResult::Failed;
        }
    }

    // 3. 이동 시작
    EPathFollowingRequestResult::Type MoveResult = AIC->MoveToLocation(NextNavLoc.Location, 80.0f);

    if (MoveResult == EPathFollowingRequestResult::RequestSuccessful)
    {
        WaitForMessage(OwnerComp, TEXT("MoveFinished"));

        // 안전한 타이머 설정 (OwnerComp 직접 참조 대신 TimerHandle만 관리)
        float Duration = Boss->GetBossDataAsset()->RepositionData.StrafeDuration;
        GetWorld()->GetTimerManager().SetTimer(StrafeTimerHandle, [this, &OwnerComp]()
            {
                // 아직 태스크가 실행 중인지 확인하는 로직이 있으면 더 안전함
                if (OwnerComp.GetActiveNode() == this)
                {
                    FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded); // 시간 다 되면 성공으로 끝낼지 실패로 끝낼지 결정
                }
            }, Duration, false);

        return EBTNodeResult::InProgress;
    }

    return (MoveResult == EPathFollowingRequestResult::AlreadyAtGoal) ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;

}

void UBTTask_Strafe::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(StrafeTimerHandle);
    }

    ResetMovementSettings(OwnerComp);
}

EBTNodeResult::Type UBTTask_Strafe::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    ResetMovementSettings(OwnerComp);

    if (AAIController* AIC = OwnerComp.GetAIOwner())
    {
        AIC->StopMovement();
    }

	return EBTNodeResult::Aborted;
}

void UBTTask_Strafe::ResetMovementSettings(UBehaviorTreeComponent& OwnerComp)
{
    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (BB)
    {
        BB->SetValueAsEnum(TEXT("AIBossPattern"), (uint8)EAIBossPattern::None);
        BB->SetValueAsEnum(TEXT("Reposition"), (uint8)EReposition::None);
        BB->SetValueAsBool(TEXT("BIsLocked"), true);
    }

    if (AAIController* AIC = OwnerComp.GetAIOwner())
    {
        AIC->StopMovement();

        if (AVL_Boss1* Boss = Cast<AVL_Boss1>(AIC->GetPawn()))
        {
            if (Boss->GetBossDataAsset() && Boss->GetCharacterMovement())
            {
                Boss->GetCharacterMovement()->bUseControllerDesiredRotation = false;
                Boss->GetCharacterMovement()->MaxWalkSpeed = Boss->GetBossDataAsset()->BaseStats.MaxMoveSpeed;
            }
        }
    }
}


void UBTTask_Strafe::OnMessage(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, FName Message, int32 RequestID, bool bSuccess)
{
    Super::OnMessage(OwnerComp, NodeMemory, Message, RequestID, bSuccess);

    // AI의 이동이 끝났을 때 발생하는 메시지 이름 확인
    if (Message == TEXT("MoveFinished"))
    {
        FinishLatentTask(OwnerComp, bSuccess ? EBTNodeResult::Succeeded : EBTNodeResult::Failed);
    }
}