#include "Monster/BehaviorTree/Task/BTTask_WoodPhaseChange.h"
#include "AIController.h"
#include "../../Boss/VL_Boss1.h"
#include "BehaviorTree/BlackboardComponent.h"


UBTTask_WoodPhaseChange::UBTTask_WoodPhaseChange()
{
    NodeName = TEXT("Wood Phase Change");
    bNotifyTick = false;

    bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_WoodPhaseChange::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{

    CachedOwnerComp = &OwnerComp;

    AAIController* AIC = OwnerComp.GetAIOwner();
    AVL_Boss1* Boss = (AIC) ? Cast<AVL_Boss1>(AIC->GetPawn()) : nullptr;

    if (Boss)
    {
        Boss->OnPhaseChangeFinished.RemoveDynamic(this, &UBTTask_WoodPhaseChange::FinishedChanging);
        Boss->OnPhaseChangeFinished.AddDynamic(this, &UBTTask_WoodPhaseChange::FinishedChanging);

        return EBTNodeResult::InProgress;
    }

    return EBTNodeResult::Failed;
}


void UBTTask_WoodPhaseChange::FinishedChanging()
{
    if (UBlackboardComponent* BB = CachedOwnerComp->GetBlackboardComponent())
    {
        BB->SetValueAsEnum(TEXT("AIBossPattern"), (uint8)EAIBossPattern::None);
        BB->SetValueAsEnum(TEXT("AIState"), (uint8)EAIState::Combat);
        
    }
    if (CachedOwnerComp)
    {
        FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
    }
}