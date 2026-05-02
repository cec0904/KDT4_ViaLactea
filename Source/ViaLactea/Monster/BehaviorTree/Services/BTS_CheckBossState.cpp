#include "Monster/BehaviorTree/Services/BTS_CheckBossState.h"
#include "../../Boss/VL_Boss1.h"
#include "Base/Data/Character/VL_BossMonsterDataAsset.h"

#include "Base/Component/VL_AggroComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

#include "CustomLog/CustomLog.h"

namespace BBKeys
{
    static const FName AIState = TEXT("AIState");
    static const FName BossPhase = TEXT("BossPhase");
    static const FName TargetActor = TEXT("TargetActor");
    static const FName BIsLocked = TEXT("BIsLocked");
    static const FName CurrentHP = TEXT("CurrentHP");
    static const FName AIBossPattern = TEXT("AIBossPattern");
}

UBTS_CheckBossState::UBTS_CheckBossState()
{
    NodeName = TEXT("Check Boss State Service");
    Interval = 0.3f;
    bCreateNodeInstance = true;

    bCallTickOnSearchStart = true;
}
void UBTS_CheckBossState::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    Super::OnBecomeRelevant(OwnerComp, NodeMemory);

    FBTS_CommonMemory* MyMemory = reinterpret_cast<FBTS_CommonMemory*>(NodeMemory);

    AAIController* AIC = OwnerComp.GetAIOwner();
    if (AIC)
    {
        MyMemory->CachedAIC = AIC;
        MyMemory->CachedBB = OwnerComp.GetBlackboardComponent();
        if (AVL_Boss1* Boss = Cast<AVL_Boss1>(AIC->GetPawn()))
        {
            MyMemory->CachedBoss = Boss;
            MyMemory->CachedMonsterData = Boss->GetMonsterDataAsset();
        }
    }
}

void UBTS_CheckBossState::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    FBTS_CommonMemory* MyMemory = reinterpret_cast<FBTS_CommonMemory*>(NodeMemory);

    if (!MyMemory->CachedAIC.IsValid()) MyMemory->CachedAIC = OwnerComp.GetAIOwner();
    if (!MyMemory->CachedBB.IsValid())  MyMemory->CachedBB = OwnerComp.GetBlackboardComponent();

    if (!MyMemory->CachedBoss.IsValid() && MyMemory->CachedAIC.IsValid())
    {
        if (AVL_Boss1* Boss = Cast<AVL_Boss1>(MyMemory->CachedAIC->GetPawn()))
        {
            MyMemory->CachedBoss = Boss;
            MyMemory->CachedMonsterData = Boss->GetMonsterDataAsset(); // 보스 찾을 때 데이터도 같이 갱신
        }
    }

    // --- 변수 할당 및 유효성 검사 ---
    AAIController* AIC = MyMemory->CachedAIC.Get();
    AVL_Boss1* AIChar = MyMemory->CachedBoss.Get();
    UBlackboardComponent* BB = MyMemory->CachedBB.Get();
    UVL_MonsterDataAsset* Data = MyMemory->CachedMonsterData.Get();

    if (!AIC || !AIChar || !BB || !Data || !AIChar->HasAuthority())
    {
        return;
    }
    
    EAIState CurrentAIState = (EAIState)BB->GetValueAsEnum(BBKeys::AIState);
    EAIBossPattern BossPattern = (EAIBossPattern)BB->GetValueAsEnum(BBKeys::AIBossPattern);

    // 죽었을 때는 아무것도 안 함
    if (CurrentAIState == EAIState::Dead) return;

    // 그로기 중이거나 페이즈 전환 중일 때도 타겟을 바꾸지 않도록 함

    if (BossPattern == EAIBossPattern::None && CurrentAIState != EAIState::Groggy)
    {
        if (UVL_AggroComponent* AggroComp = AIChar->GetAggroComponent())
        {
            
            float DetectionRange = Data->DetectionRange;
            AActor* CurrentTarget = Cast<AActor>(BB->GetValueAsObject(BBKeys::TargetActor));
            AActor* BestTarget = AggroComp->GetHighestAggroTarget(DetectionRange, CurrentTarget);

            if (BestTarget)
            {
                if (CurrentAIState != EAIState::Combat)
                {
                    BB->SetValueAsEnum(TEXT("AIState"), (uint8)EAIState::Combat);
                    // 필요 시 여기서 보스의 '발견 포효' RPC 호출 가능
                }

                BB->SetValueAsObject(BBKeys::TargetActor, BestTarget);
                BB->SetValueAsEnum(BBKeys::AIState, (uint8)EAIState::Combat);
                AIC->SetFocus(BestTarget);
            }
            else if (CurrentTarget != nullptr)
            {
                BB->SetValueAsObject(BBKeys::TargetActor, nullptr);
                BB->SetValueAsEnum(BBKeys::AIState, (uint8)EAIState::Idle);
                AIC->ClearFocus(EAIFocusPriority::Gameplay);
            }
        }
    }

    if (AIChar->GetIsGroggy() == false)
    {
        float RecoveryAmount = Data->GroggyRecoveryRate * DeltaSeconds;
        AIChar->DecreaseGroggyGauge(RecoveryAmount);
  
    }
}