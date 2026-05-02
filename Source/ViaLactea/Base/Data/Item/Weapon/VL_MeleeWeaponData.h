// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/Data/Item/Weapon/VL_WeaponDataAsset.h"
#include "VL_MeleeWeaponData.generated.h"

/**
 * 
 */
UCLASS()
class VIALACTEA_API UVL_MeleeWeaponData : public UVL_WeaponDataAsset
{
	GENERATED_BODY()
public:
    UPROPERTY(EditDefaultsOnly, Category = "Combat|Melee")
    float AttackRadius = 50.f; // 타격 판정 반경

    UPROPERTY(EditDefaultsOnly, Category = "Combat|Melee")
    float HitStopDuration = 0.f; // 역경직(타격 시 일시 정지) 시간
};
