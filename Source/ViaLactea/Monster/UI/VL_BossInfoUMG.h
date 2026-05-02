// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/UI/VL_UMGBase.h"
#include "VL_BossInfoUMG.generated.h"

/**
 * 
 */

class UImage;
class UMaterialInstanceDynamic;
class UTextBlock;

UCLASS()
class VIALACTEA_API UVL_BossInfoUMG : public UVL_UMGBase
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

protected:
	UPROPERTY(meta = (BindWidget))
	UImage* IMG_BossHPBar = nullptr;

	UPROPERTY()
	UMaterialInstanceDynamic* BossHPMID = nullptr;

public:
	UFUNCTION(BlueprintCallable)
	void UpdateBossHP(float CurrentHP, float MaxHP);

	UFUNCTION(BlueprintCallable)
	void SetBossName(const FText& Name);

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TXT_BossName = nullptr;
};
