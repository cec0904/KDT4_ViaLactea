#include "Monster/BehaviorTree/Services/BTS_SetMovementMode.h"
#include "Monster/Boss/VL_Boss1.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"


UBTS_SetMovementMode::UBTS_SetMovementMode()
{
	NodeName = TEXT("Set Movement Mode (Rush)");

	bNotifyTick = false;
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;

	//bCreateNodeInstance = true;

}

void UBTS_SetMovementMode::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    Super::OnBecomeRelevant(OwnerComp, NodeMemory);

    AAIController* AIC = OwnerComp.GetAIOwner();
    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();

    if (AIC && BB)
    {
        if (AVL_Boss1* Boss = Cast<AVL_Boss1>(AIC->GetPawn()))
        {
            // 1. 보스 특수 모드 활성화
            Boss->SetRushMode(true);

            // 2. 블랙보드 값 업데이트 (Bool 타입일 경우)
            BB->SetValueAsBool(BIsLockedKey.SelectedKeyName, true);
        }
    }
}

void UBTS_SetMovementMode::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    Super::OnCeaseRelevant(OwnerComp, NodeMemory);

    AAIController* AIC = OwnerComp.GetAIOwner();
    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();

    if (AIC && BB)
    {
        if (AVL_Boss1* Boss = Cast<AVL_Boss1>(AIC->GetPawn()))
        {
            // 1. 보스 특수 모드 해제
            Boss->SetRushMode(false);

            // 2. 블랙보드 값 업데이트
            BB->SetValueAsBool(BIsLockedKey.SelectedKeyName, false);
        }
    }
}