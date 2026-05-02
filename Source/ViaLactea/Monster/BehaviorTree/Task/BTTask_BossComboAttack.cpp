#include "Monster/BehaviorTree/Task/BTTask_BossComboAttack.h"
#include "AIController.h"
#include "Monster/Boss/VL_Boss1.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "CustomLog/CustomLog.h"

UBTTask_BossComboAttack::UBTTask_BossComboAttack()
{
	NodeName = TEXT("Boss Combo Attack");
	// TickTask를 사용하기 위해 true로 설정
	bNotifyTick = false;
    bCreateNodeInstance = true;
    bNotifyTaskFinished = true;
    INIT_TASK_NODE_NOTIFY_FLAGS();

}

EBTNodeResult::Type UBTTask_BossComboAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!AIC) return EBTNodeResult::Failed;

	AVL_Boss1* Boss = Cast<AVL_Boss1>(AIC->GetPawn());
	if (!Boss) return EBTNodeResult::Failed;

    Boss->OnPatternStarted(EAIBossPattern::ComboAttack);

    UBlackboardComponent* BB = AIC->GetBlackboardComponent();
    if (BB)
    {
        // 콤보 시작 시 타겟 변경을 막기 위해 Lock을 겁니다.
        BB->SetValueAsBool(TEXT("BIsLocked"), true);
    }

    bWasAborted = false;
    CachedOwnerComp = &OwnerComp;

	Boss->OnComboEnded.RemoveAll(this); // 중복 바인딩 방지
	Boss->OnComboEnded.AddUObject(this, &UBTTask_BossComboAttack::OnAttackFinished);

	// 1. 보스의 콤보 시작 함수 호출
	Boss->StartCombo();

	// 2. 공격이 시작되었으므로 태스크를 '진행 중' 상태로 반환
	return EBTNodeResult::InProgress;
}

void UBTTask_BossComboAttack::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
    // 2. 보스 객체 관련 정리
    if (AAIController* AIC = OwnerComp.GetAIOwner())
    {
        if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
        {
            ResetBossBlackboard(BB);
            BB->SetValueAsBool(TEXT("BIsLocked"), false);
        }

        if (AVL_Boss1* Boss = Cast<AVL_Boss1>(AIC->GetPawn()))
        {
            // 델리게이트 해제 (메모리 누수 및 중복 실행 방지)
            Boss->OnComboEnded.RemoveAll(this);
            if (TaskResult != EBTNodeResult::Succeeded && Boss->GetIsAttacking())
            {
                Boss->EndCombo();


            }
        }
    }
    Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

void UBTTask_BossComboAttack::OnAttackFinished()
{
    if (CachedOwnerComp.IsValid())
    {
        FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
    }
}

EBTNodeResult::Type UBTTask_BossComboAttack::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{

    AAIController* AIC = OwnerComp.GetAIOwner();
    if (AIC)
    {
        if (AVL_Boss1* Boss = Cast<AVL_Boss1>(AIC->GetPawn()))
        {
            // 공격 중 중단되었다면 보스의 상태를 강제로 초기화 (몽타주 중단 등)
            Boss->EndCombo();
            Boss->OnComboEnded.RemoveAll(this);
        }
    }

    return Super::AbortTask(OwnerComp, NodeMemory);
}