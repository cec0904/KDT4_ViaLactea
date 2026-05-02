// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/UI/VL_UMGBase.h"
#include "VL_PlayerCrosshairWidget.generated.h"

/**
 * 
 */

class UImage;
class USizeBox;

UCLASS()
class VIALACTEA_API UVL_PlayerCrosshairWidget : public UVL_UMGBase
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Crosshair")
	void SetAimState(bool bInIsAiming);

	UFUNCTION(BlueprintCallable, Category = "Crosshair")
	void SetChargeAlpha(float InChargeAlpha);

	UFUNCTION(BlueprintCallable, Category = "Crosshair")
	void SetChargeRingVisible(bool bInVisible);

	UFUNCTION(BlueprintCallable, Category = "Crosshair")
	void ResetCrosshair();

protected:
	virtual void NativeConstruct() override;

	void UpdateCrosshairVisual();

protected:
	UPROPERTY(meta = (BindWidget))
	UImage* Image_CenterDot;

	UPROPERTY(meta = (BindWidget))
	UImage* Image_ChargeRing;

	UPROPERTY(meta = (BindWidgetOptional))
	USizeBox* SizeBox_ChargeRing;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crosshair")
	bool bIsAiming = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crosshair")
	bool bShowChargeRing = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crosshair")
	float ChargeAlpha = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crosshair")
	FLinearColor NormalDotColor = FLinearColor(0.85f, 0.85f, 0.85f, 1.f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crosshair")
	FLinearColor AimDotColor = FLinearColor(1.f, 0.15f, 0.15f, 1.f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crosshair")
	float ChargeRingMaxSize = 80.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crosshair")
	float ChargeRingMinSize = 20.f;
};
