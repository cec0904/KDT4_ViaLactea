#include "Monster/BehaviorTree/Task/BTTask_Backstep.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "NavMesh/RecastNavMesh.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


#include "Monster/Boss/VL_Boss1.h"
#include "Base/Data/Character/VL_BossMonsterDataAsset.h"

#include "CustomLog/CustomLog.h"

UBTTask_Backstep::UBTTask_Backstep()
{
    //backstep 태스크
    NodeName = "Boss Backstep";
    bNotifyTick = false; 
    bCreateNodeInstance = true;
    INIT_TASK_NODE_NOTIFY_FLAGS();

}

EBTNodeResult::Type UBTTask_Backstep::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    APawn* ControllingPawn = OwnerComp.GetAIOwner()->GetPawn();
    AVL_Boss1* Character = Cast<AVL_Boss1>(ControllingPawn);

    if (!Character || !BackstepMontage) return EBTNodeResult::Failed;

    // 1. 기본 수치 저장 및 캐싱
    UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement();
    DefaultGravity = MoveComp->GravityScale;
    DefaultFriction = MoveComp->GroundFriction;
    DefaultBraking = MoveComp->BrakingDecelerationWalking;

    CachedOwnerComp = &OwnerComp;

    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (NavSys)
    {
        FNavLocation ProjectedLocation;
        if (NavSys->ProjectPointToNavigation(TargetLocation, ProjectedLocation, FVector(100.f, 100.f, 300.f)))
        {
            // 3. 델리게이트 바인딩
            UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
            if (AnimInstance)
            {
                AnimInstance->OnMontageEnded.AddUniqueDynamic(this, &UBTTask_Backstep::OnMontageEnded);
            }

            // 4. 애니메이션 실행
            Character->Multicast_PlayBossMontage(BackstepMontage);

            // 5. 타이머: 0.3초 후 Launch 실행
            TWeakObjectPtr<AVL_Boss1> WeakChar(Character);
            Character->GetWorldTimerManager().SetTimer(LaunchTimerHandle, [WeakChar]()
                {
                    if (WeakChar.IsValid())
                    {
                        UCharacterMovementComponent* MC = WeakChar->GetCharacterMovement();
                        MC->StopMovementImmediately();
                        MC->GravityScale = 0.5f;
                        MC->GroundFriction = 0.0f;
                        MC->BrakingDecelerationWalking = 0.0f;

                        FVector LaunchVelocity = -WeakChar->GetActorForwardVector() * 300.f + FVector::UpVector * 330.f;
                        WeakChar->LaunchCharacter(LaunchVelocity, true, true);
                    }
                }, 0.4f, false);

            return EBTNodeResult::InProgress;
        }
    }

    // 네비매쉬 투영에 실패했거나 이동할 수 없는 경우
    return EBTNodeResult::Failed;
}

void UBTTask_Backstep::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (Montage == BackstepMontage && CachedOwnerComp)
    {
        // FinishLatentTask가 호출되면 이후 자동으로 OnTaskFinished가 실행됩니다.
        FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
    }
}

EBTNodeResult::Type UBTTask_Backstep::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{

    return Super::AbortTask(OwnerComp, NodeMemory);
}

void UBTTask_Backstep::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
    APawn* ControllingPawn = OwnerComp.GetAIOwner()->GetPawn();
    AVL_Boss1* Character = Cast<AVL_Boss1>(ControllingPawn);

    // 1. 물리 상태 복구 (초기화)
    if (Character)
    {
        UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement();
        MoveComp->GravityScale = DefaultGravity;
        MoveComp->GroundFriction = DefaultFriction;
        MoveComp->BrakingDecelerationWalking = DefaultBraking;

        // 델리게이트 제거
        if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
        {
            AnimInstance->OnMontageEnded.RemoveDynamic(this, &UBTTask_Backstep::OnMontageEnded);
        }

        // 타이머 제거 (0.3초 전에 종료될 경우 대비)
        Character->GetWorldTimerManager().ClearTimer(LaunchTimerHandle);
    }

    // 2. 블랙보드 키 정리
    if (UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
    {
        BB->SetValueAsEnum(TEXT("AIBossPattern"), (uint8)EAIBossPattern::None);
        BB->SetValueAsEnum(TEXT("Reposition"), (uint8)EReposition::None);
    }
    Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}
