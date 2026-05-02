// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/Data/Item/Weapon/VL_WeaponDataAsset.h"
#include "VL_RangeWeaponData.generated.h"

/**
 * 
 */
UCLASS()
class VIALACTEA_API UVL_RangeWeaponData : public UVL_WeaponDataAsset
{
	GENERATED_BODY()

public:
    // 발사할 화살/투사체 클래스 (BP_Arrow 등)
    UPROPERTY(EditDefaultsOnly, Category = "Combat|Range")
    TSubclassOf<class AActor> ProjectileClass;

    UPROPERTY(EditDefaultsOnly, Category = "Combat|Range")
    float MaxDrawTime = 1.5f; // 활 시위를 끝까지 당기는 시간


};
