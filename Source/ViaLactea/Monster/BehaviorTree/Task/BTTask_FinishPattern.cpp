#include "Monster/BehaviorTree/Task/BTTask_FinishPattern.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Base/AI/AIBossPattern.h"

UBTTask_FinishPattern::UBTTask_FinishPattern()
{
	NodeName = TEXT("Finish Pattern");

	// 블랙보드 키 필터링 (Enum 타입만 선택 가능하도록)
	BossPatternKey.AddEnumFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_FinishPattern, BossPatternKey), StaticEnum<EAIBossPattern>());
	RepositionTypeKey.AddEnumFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_FinishPattern, RepositionTypeKey), StaticEnum<EReposition>());
	INIT_TASK_NODE_NOTIFY_FLAGS();

}


EBTNodeResult::Type UBTTask_FinishPattern::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return EBTNodeResult::Failed;

	// 1. 메인 패턴 상태를 None으로 초기화 (서비스가 다시 판단할 수 있게 함)
	BB->SetValueAsEnum(BossPatternKey.SelectedKeyName, (uint8)EAIBossPattern::None); // 0은 보통 None (EAIBossPattern::None)

	// 2. 기동 타입도 함께 초기화 
	BB->SetValueAsEnum(RepositionTypeKey.SelectedKeyName, (uint8)EReposition::None); // EReposition::None

	BB->SetValueAsBool(IsLockedKey.SelectedKeyName, false);

	return EBTNodeResult::Succeeded;
}