// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/AI/BehaviorTree/Services/VL_BTService.h"
#include "Base/AI/AIBossPattern.h"
#include "BTS_BossDecisionMaker.generated.h"

class AVL_Boss1;
class UVL_BossMonsterDataAsset;
class UBlackboardComponent;

UCLASS()
class VIALACTEA_API UBTS_BossDecisionMaker : public UVL_BTService
{
	GENERATED_BODY()
public:
	UBTS_BossDecisionMaker();

protected:
    virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	// 서비스가 주기적으로 실행할 함수
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	bool TryPhaseTransition(UBlackboardComponent* BB);

	float GetRepositionDuration(EReposition Type) const;


protected:
	void UpdateCombatContext(AVL_Boss1* Boss, APawn* Target, UBlackboardComponent* BB);
	/** 상황에 맞는 패턴 카테고리(Family)를 결정합니다. */
	EPatternFamily DecidePatternFamily(const FBossCombatContext& Context) const;

	EPatternFamily WoodDecidePatternFamily(const FBossCombatContext& Context) const;


	void SelectOnePattern(EPatternFamily SelectedFamily, UBlackboardComponent* BB);

	void SelectPositioning(APawn* Target, UBlackboardComponent* BB);

	void CheckBossNavMeshLocation(UBlackboardComponent* BB);

	// 블랙보드 키 선택 (에디터에서 설정)
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector BossFamilyPatternKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector BossPatternKey;

	 UPROPERTY(EditAnywhere, Category = "Blackboard")
	 FBlackboardKeySelector PhaseKey;

	 UPROPERTY(EditAnywhere, Category = "Blackboard")
	 FBlackboardKeySelector RepositionTypeKey;

private:
	// 캐싱을 위한 변수들
	UPROPERTY()
	AVL_Boss1* CachedBoss;

	UPROPERTY()
	UVL_BossMonsterDataAsset* CachedDataAsset;

    // 각 패턴별 마지막 실행 시간을 기록 (인스턴스별 관리)
    TMap<EAIBossPattern, float> LastPatternTimeMap;

private:
	// 플레이어가 근접해 있는 누적 시간
	float CloseRangeTimer = 0.0f;

	// 데이터 에셋에 있으면 좋지만, 임시로 상수로 정의 가능
	const float CloseRangeThreshold = 10.0f; // 너무 오래 붙어서 공격을 유지한다면 보스가 탈출

private:
	float ContextUpdateTimer = 0.0f;
	const float ContextUpdateInterval = 0.5f; // 갱신 주기
	bool bFirstTickInNone = false;
	FBossCombatContext CachedContext; // 계산된 데이터를 저장해둘 변수
	float CurrentTime;
	float NavCheckTimer = 0.0f; // 내비매시 검사 주기
};
