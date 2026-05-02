#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "VFXData.generated.h"

class UVL_VFXDataAsset;

USTRUCT(BlueprintType)
struct FVFXData
{
	GENERATED_BODY()
public:
	UPROPERTY()
	UVL_VFXDataAsset* DataAsset = nullptr;

	UPROPERTY()
	FGameplayTag Tag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CollisionRadius = 50.0f; // 충돌 범위

	// 데이터가 유효한지 확인하는 도우미 함수
	bool IsValid() const { return DataAsset != nullptr && Tag.IsValid(); }
};