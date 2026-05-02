// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/Data/Action/VL_ActionDataAsset.h"
#include "VL_SkillDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class VIALACTEA_API UVL_SkillDataAsset : public UVL_ActionDataAsset
{
	GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Combat")
    float DamageMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, Category = "Combat")
    float ManaCost = 20.f;

    UPROPERTY(EditAnywhere, Category = "Combat")
    float Cooldown = 1.f;
	
};
