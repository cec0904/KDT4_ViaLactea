// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/UI/VL_UMGBase.h"

#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"

#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

#include "VL_PlayerInfoUMG.generated.h"


/**
 *
 */

class AMainCharacterBase;
class UVL_StatComponent;

UCLASS()
class VIALACTEA_API UVL_PlayerInfoUMG : public UVL_UMGBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UImage* IMG_HPFill;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|HP")
	TObjectPtr<UMaterialInterface> HPFillMaterial = nullptr;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> MID_HPBar = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UImage* IMG_SPFill;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|Stamina")
	TObjectPtr<UMaterialInterface> SPFillMaterial = nullptr;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> MID_SPBar = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UImage* IMG_HPDelayFill;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|HP")
	TObjectPtr<UMaterialInterface> HPDelayFillMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|HP")
	float HPDelayStartTime = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|HP")
	float HPDelayLerpSpeed = 2.5f;

public:
	UFUNCTION(BlueprintCallable)
	void RebindAndRefresh();

private:

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> MID_HPDelayBar = nullptr;

	UPROPERTY()
	float TargetHPPercent = 1.0f;

	UPROPERTY()
	float DisplayedDelayHPPercent = 1.0f;

	FTimerHandle HPDelayTimerHandle;

	UPROPERTY()
	bool bStartDelay = false;

public:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	virtual void NativeConstruct() override;

	UPROPERTY()
	TObjectPtr<AMainCharacterBase> CachedCharacter = nullptr;

	UPROPERTY()
	TObjectPtr<UVL_StatComponent> CachedStatComponent = nullptr;

	UFUNCTION()
	void HandleHPChanged(float NewHP, float MaxHP);

	UFUNCTION()
	void HandleStaminaChanged(float NewStamina, float MaxStamina);

	void BindToStatComponent();
	void RefreshAll();

	void InitializeHPMaterial();
	void InitializeStaminaMaterial();

protected:
	void InitializeHPDelayMaterial();
	void StartHPDelayBar();
	void UpdateHPDelayBar(float DeltaTime);
};
