// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "VL_CooldownComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class VIALACTEA_API UVL_CooldownComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVL_CooldownComponent();

protected:
	// 태그별 마지막 사용 시간을 저장하는 장부 (런타임 데이터)
	// ui 연동시 tmap은 복사 불가능
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cooldown|State")
	TMap<FGameplayTag, float> CooldownHistory;

public:
	//  데이터 에셋 연동형: 쿨타임 수치를 데이터 에셋에서 자동으로 가져와서 체크
	UFUNCTION(BlueprintCallable, Category = "Cooldown")
	bool IsReady(FGameplayTag Tag) const;

	/** * 2. 수동 입력형: 특정 쿨타임 수치를 직접 넣어서 체크 (범용 액터용)
	 */
	UFUNCTION(BlueprintCallable, Category = "Cooldown")
	bool IsReadyManual(FGameplayTag Tag, float Duration) const;

	/** * 쿨타임 시작 (현재 시간을 기록)
	 */
	UFUNCTION(BlueprintCallable, Category = "Cooldown")
	void StartCooldown(FGameplayTag Tag);

	/** * 특정 쿨타임 강제 초기화
	 */
	UFUNCTION(BlueprintCallable, Category = "Cooldown")
	void ResetCooldown(FGameplayTag Tag);

public:
	float GetCurrentTime() const;

};
