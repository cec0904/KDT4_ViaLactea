// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VL_StatComponent.h"
#include "VL_AggroComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VIALACTEA_API UVL_AggroComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UVL_AggroComponent();

	UFUNCTION(BlueprintCallable, Category = "Aggro")
	void RegisterInitialTarget(AActor* Target);

	// 데미지를 입을 때마다 호출하여 어그로 수치 증가
	UFUNCTION(BlueprintCallable, Category = "Aggro")
	void AddThreat(AActor* Attacker, float DamageAmount);

	// 타겟이 죽거나 범위를 벗어나면 목록에서 제거
	UFUNCTION(BlueprintCallable, Category = "Aggro")
	void RemoveThreat(AActor* Target);

	UFUNCTION(BlueprintCallable, Category = "Aggro")
	void ClearAllThreat();

	// 현재 가장 어그로 수치가 높은 타겟 반환 (거리, 데미지 등 종합 계산)
	UFUNCTION(BlueprintCallable, Category = "Aggro")
	AActor* GetHighestAggroTarget(float DetectionRange, AActor* CurrentTarget = nullptr) const;

	TMap<AActor*, float>* GetThreatMap();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// 타겟별 누적 어그로(데미지 기반) 저장
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Aggro|Data")
	TMap<AActor*, float> ThreatMap;

	// --- 기획 조정용 가중치 변수들 ---

	// 데미지 1당 올라가는 어그로 점수
	UPROPERTY(EditDefaultsOnly, Category = "Aggro|Weight")
	float DamageWeight = 1.0f;

	// 거리가 100 멀어질 때마다 깎이는 어그로 점수 (가까운 적 우선)
	UPROPERTY(EditDefaultsOnly, Category = "Aggro|Weight")
	float DistancePenaltyWeight = 5.0f;

	// 파티원 수에 따른 어그로 분산 계수 (필요시 사용)
	UPROPERTY(EditDefaultsOnly, Category = "Aggro|Weight")
	float PlayerCountScoreWeight = 100.0f;

	// 타겟 결정 시 적용할 무작위성 (0.0 ~ 1.0)
	// 0이면 무조건 점수 높은 인원, 1에 가까울수록 운에 따라 타겟이 자주 바뀜
	UPROPERTY(EditDefaultsOnly, Category = "Aggro|Weight", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RandomnessFactor = 0.5f;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
