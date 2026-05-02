// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "VL_BossGimmickInterface.generated.h"


UINTERFACE(MinimalAPI)
class UVL_BossGimmickInterface : public UInterface
{
	GENERATED_BODY()

};

/**
 * 
 */
class VIALACTEA_API IVL_BossGimmickInterface
{
	GENERATED_BODY()

public:
    // 기믹이 현재 돌진 가능한 상태인지 확인
    UFUNCTION(BlueprintNativeEvent, Category = "Gimmick")
    bool IsTargetable() const;

    // 기믹의 돌진 목표 지점 반환
    UFUNCTION(BlueprintNativeEvent, Category = "Gimmick")
    FVector GetGimmickTargetLocation() const;


};
