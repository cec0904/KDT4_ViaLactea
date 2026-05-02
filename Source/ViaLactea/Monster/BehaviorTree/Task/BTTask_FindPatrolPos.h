// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/AI/BehaviorTree/Tasks/VL_BTTaskNode.h"
#include "BTTask_FindPatrolPos.generated.h"

/**
 * 
 */
UCLASS()
class VIALACTEA_API UBTTask_FindPatrolPos : public UVL_BTTaskNode
{
	GENERATED_BODY()
public:
	// 생성자는  노드이름을 정하는 것
	UBTTask_FindPatrolPos();
	//테스크 실행 시 호출되는 함수
	// 여기에 구현
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* Nodememory);

protected:
	//에디터 테스크 노드에서도 수정할수 있게 변수로 받아두면 좋다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float SearchRadius = 1200.f;

	//어느 블랙보드에 데이터에 접근해서 사용할거니?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	FBlackboardKeySelector PatrolPosKey;

};
