// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "VL_AnimSkillSet.generated.h"

/**
 * 
 */
UCLASS()
class VIALACTEA_API UVL_AnimSkillSet : public UDataAsset
{
	GENERATED_BODY()
public:
    // 스킬 태그와 몽타주 매핑
    // 예: "Skill.Active.Q" -> Montage_SpinAttack
    // 예: "Skill.Active.E" -> Montage_PowerSmash
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    TMap<FGameplayTag, TSoftObjectPtr<UAnimMontage>> SkillMontages;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    UAnimMontage* GetMontageByTag(FGameplayTag Tag) const;

};
