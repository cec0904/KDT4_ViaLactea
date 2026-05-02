#include "BTS_CheckState.h"

#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "Base/Character/VL_AICharacterBase.h"
#include "Base/Data/Character/VL_MonsterDataAsset.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"


#include "CustomLog/CustomLog.h"

UBTS_CheckState::UBTS_CheckState()
{
    NodeName = TEXT("Check State Service");
    Interval = 0.3f;
    bCreateNodeInstance = true;
}

void UBTS_CheckState::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    Super::OnBecomeRelevant(OwnerComp, NodeMemory);
    
    FBTS_CommonMemory* MyMemory = reinterpret_cast<FBTS_CommonMemory*>(NodeMemory);

    AAIController* AIC = OwnerComp.GetAIOwner();
    if (AIC)
    {
        MyMemory->CachedAIC = AIC;
        MyMemory->CachedBB = OwnerComp.GetBlackboardComponent();
        MyMemory->CachedPawn = Cast<AVL_AICharacterBase>(AIC->GetPawn());
    }
}

void UBTS_CheckState::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);


    FBTS_CommonMemory* MyMemory = reinterpret_cast<FBTS_CommonMemory*>(NodeMemory);
    
    // 1. AIC 지연 초기화
    if (!MyMemory->CachedAIC.IsValid())
    {
        MyMemory->CachedAIC = OwnerComp.GetAIOwner();
    }

    // 2. BB 지연 초기화 (추가)
    if (!MyMemory->CachedBB.IsValid())
    {
        MyMemory->CachedBB = OwnerComp.GetBlackboardComponent();
    }

    // 3. AIChar 지연 초기화
    if (!MyMemory->CachedPawn.IsValid() && MyMemory->CachedAIC.IsValid())
    {
        MyMemory->CachedPawn = Cast<AVL_AICharacterBase>(MyMemory->CachedAIC->GetPawn());
    }

    // --- 변수 할당 및 유효성 검사 ---
    AAIController* AIC = MyMemory->CachedAIC.Get();
    AVL_AICharacterBase* AIChar = MyMemory->CachedPawn.Get();
    UBlackboardComponent* BB = MyMemory->CachedBB.Get();

    if (!AIC || !AIChar || !BB || !AIChar->HasAuthority())
    {
        return;
    }

    // 데이터 에셋 캐싱
    const UVL_MonsterDataAsset* MonsterData = AIChar->GetMonsterDataAsset();
    if (!MonsterData)
    {
        return;
    }

    EAIState CurrentEnumState = (EAIState)BB->GetValueAsEnum(TEXT("AIState"));

    if (BB->GetValueAsBool(TEXT("bIsLocked")))
    {
        return; // 현재 상태(Alert/Heal)를 무조건 유지함
    }


    AActor* CurrentTarget = Cast<AActor>(BB->GetValueAsObject(TEXT("TargetActor")));

    FVector MonsterLocation = AIChar->GetActorLocation();
    
    // --- 1. NavMesh 유효성 검사 추가 ---
    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (NavSys)
    {
        float HalfHeight = AIChar->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
        FVector FootLoc = MonsterLocation - FVector(0.f, 0.f, HalfHeight);

        FNavLocation OutLocation;
        if (!NavSys->ProjectPointToNavigation(FootLoc, OutLocation, FVector(500.f, 500.f, 300.f)))
        {
            BB->SetValueAsEnum(TEXT("AIState"), (uint8)EAIState::Returnto);
            BB->SetValueAsObject(TEXT("TargetActor"), nullptr);
            return;
        }
    }
    // --- 1. 타겟 후보 탐색 ---
    AActor* BestTargetCandidate = nullptr;
    float MinDistance = MAX_FLT;

    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        if (APlayerController* PC = It->Get())
        {
            if (APawn* PlayerPawn = PC->GetPawn())
            {
                float Dist = FVector::Dist(MonsterLocation, PlayerPawn->GetActorLocation());
                if (Dist < MinDistance)
                {
                    MinDistance = Dist;
                    BestTargetCandidate = PlayerPawn;
                }
            }
        }
    }

    // --- 2. 타겟 확정 (Stickiness 적용) ---
    // 거리가 멀더라도 일단 비교 로직을 태우기 위해 후보를 먼저 세팅


    AActor* NewFinalTarget = CurrentTarget;
    if (CurrentTarget && IsValid(CurrentTarget))
    {
        float DistToCurrent = AIChar->GetDistanceTo(CurrentTarget);

        if (DistToCurrent > MonsterData->LoseTargetRange)
        {
            NewFinalTarget = nullptr; // 유실 거리를 벗어나면 포기
        }
        else
        {
            MinDistance = DistToCurrent;
            
            if (BestTargetCandidate && BestTargetCandidate != CurrentTarget)
            {
                float DistToBest = FVector::Dist(MonsterLocation, BestTargetCandidate->GetActorLocation());

                if (DistToBest < DistToCurrent - MonsterData->StickinessBuffer)
                {
                    FVector Dir = (BestTargetCandidate->GetActorLocation() - MonsterLocation).GetSafeNormal();
                    float Angle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(AIChar->GetActorForwardVector(), Dir)));

                    if (Angle <= MonsterData->VisionAngle)
                    {
                        NewFinalTarget = BestTargetCandidate;
                        MinDistance = DistToBest;
                    }
                }
            }
        }
    }
    // B. 타겟이 없는 경우: 신규 감지 (감지 거리 + 시야각 모두 만족해야 함)
    else if (BestTargetCandidate)
    {
        if (MinDistance <= MonsterData->DetectionRange)
        {
            FVector Dir = (BestTargetCandidate->GetActorLocation() - MonsterLocation).GetSafeNormal();
            float Angle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(AIChar->GetActorForwardVector(), Dir)));

            if (Angle <= MonsterData->VisionAngle)
            {
                NewFinalTarget = BestTargetCandidate;
            }
        }
    }

    // --- 3. 현재 공격 중인지 확인 ---
    bool bIsAttacking = AIChar->GetIsAttacking();

    // --- 4. 상태 결정 로직 ---
    //EAIState NewState = EAIState::Idle;
    EAIState NewState = CurrentEnumState;
    
    
    if (NewFinalTarget) // 타겟이 감지 범위 내에 있음
    {
        bool bIsTargetOnNav = false;
        FNavLocation DummyLocation;
        FVector TargetFootLoc = NewFinalTarget->GetActorLocation();

        // 만약 타겟이 캐릭터라면 캡슐 절반 높이를 빼서 발바닥 좌표를 구함
        if (ACharacter* TargetChar = Cast<ACharacter>(NewFinalTarget))
        {
            float TargetHalfHeight = TargetChar->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
            TargetFootLoc.Z -= TargetHalfHeight;
        }

        if (NavSys)
        {
            FVector QueryExtent = FVector(20.f, 20.f, 200.f);

            bIsTargetOnNav = NavSys->ProjectPointToNavigation(TargetFootLoc, DummyLocation, QueryExtent);
        }

        BB->SetValueAsBool(TEXT("bIsTargetOnNav"), bIsTargetOnNav);

        // [보완] 공격 중이라면 플레이어 위치와 상관없이 일단 Combat 유지
        if (bIsAttacking)
        {
            NewState = EAIState::Combat;
        }
        else
        {
                // A. 시야에 있고, 내비메쉬 위에도 있는가? (완벽한 추적 조건)
            if (bIsTargetOnNav)
            {
                float RangeThreshold = (CurrentEnumState == EAIState::Combat) ? MonsterData->ExitAttackRange : MonsterData->AttackRange;

                NewState = (MinDistance <= RangeThreshold) ? EAIState::Combat : EAIState::Chase;

                //CUSTOM_LOG("내비매쉬 : %d, 감지거리: %f", NewState, MinDistance);
            }
            // B. 시야에는 있지만, 내비메쉬 밖(2cm 이상)인가? (사용자님의 의도)
            else
            {
                NewState = EAIState::Alert;
            }

        }
    }
    else // 감지거리에 타깃이 없어서 타깃이 nullptr인 상황 
    {
        if (bIsAttacking)
        {
            NewState = EAIState::Combat;
        }
        else if (CurrentEnumState == EAIState::Chase ||
            CurrentEnumState == EAIState::Combat ||
            CurrentEnumState == EAIState::Alert)
        {
            NewState = EAIState::Idle;
        }
    }

    // --- 5. 블랙보드 업데이트 ---
    // A. 상태 업데이트
    if (NewState != CurrentEnumState)
    {

        BB->SetValueAsEnum(TEXT("AIState"), (uint8)NewState);

        // [핵심 추가] 상태가 Chase로 변경될 때 무조건 속도를 정상화합니다.

        if (UCharacterMovementComponent* MoveComp = AIChar->GetCharacterMovement())
        {
            if (NewState == EAIState::Chase)
            {
                // 공격 시 0이 되었던 속도를 여기서 확실히 되살립니다.
                MoveComp->MaxWalkSpeed = MonsterData->BaseStats.MaxMoveSpeed;
            }
            // 1. 비전투 상태 (Idle, Patrol, ReturnTo 등)
            // 캐릭터가 이동하는 방향을 바라보며 자연스럽게 걷습니다.
            if (NewState == EAIState::Idle || NewState == EAIState::Returnto || NewState == EAIState::Patrol)
            {
                MoveComp->bOrientRotationToMovement = true;   // 이동 방향으로 회전
                MoveComp->bUseControllerDesiredRotation = false; // 컨트롤러 회전 무시

                AIC->ClearFocus(EAIFocusPriority::Gameplay);
            }
            // 2. 전투/경계 상태 (Alert, Chase, Combat)
            // 타겟을 바라본 채로 이동(공전/게걸음)합니다.
            else
            {
                MoveComp->bOrientRotationToMovement = false;  // 이동 방향 회전 끔
                MoveComp->bUseControllerDesiredRotation = true; // 컨트롤러(Focus) 회전 따라감

            }
        }
    }

    // B. 타겟 업데이트 (공격 중에는 타겟을 바꾸지 않음)
    if (!bIsAttacking)
    {
        if (NewFinalTarget != CurrentTarget)
        {
            BB->SetValueAsObject(TEXT("TargetActor"), NewFinalTarget);
        }

        if (NewFinalTarget && (NewState == EAIState::Chase || NewState == EAIState::Alert || NewState == EAIState::Combat))
        {
            AIC->SetFocus(NewFinalTarget);
        }
        else
        {
            AIC->ClearFocus(EAIFocusPriority::Gameplay);
        }
    }
}