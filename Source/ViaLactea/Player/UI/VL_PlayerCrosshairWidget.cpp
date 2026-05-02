// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/UI/VL_PlayerCrosshairWidget.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"

void UVL_PlayerCrosshairWidget::NativeConstruct()
{
	Super::NativeConstruct();
	UpdateCrosshairVisual();
}

void UVL_PlayerCrosshairWidget::SetAimState(bool bInIsAiming)
{
	if (bIsAiming == bInIsAiming)
	{
		return;
	}

	bIsAiming = bInIsAiming;

	if (!bIsAiming)
	{
		ChargeAlpha = 0.f;
		bShowChargeRing = false;
	}

	UpdateCrosshairVisual();
}

void UVL_PlayerCrosshairWidget::SetChargeAlpha(float InChargeAlpha)
{
	ChargeAlpha = FMath::Clamp(InChargeAlpha, 0.f, 1.f);
	UpdateCrosshairVisual();
}

void UVL_PlayerCrosshairWidget::SetChargeRingVisible(bool bInVisible)
{
	if (bShowChargeRing == bInVisible)
	{
		return;
	}

	bShowChargeRing = bInVisible;
	UpdateCrosshairVisual();
}

void UVL_PlayerCrosshairWidget::ResetCrosshair()
{
	bIsAiming = false;
	bShowChargeRing = false;
	ChargeAlpha = 0.f;
	UpdateCrosshairVisual();
}

void UVL_PlayerCrosshairWidget::UpdateCrosshairVisual()
{
	if (Image_CenterDot)
	{
		Image_CenterDot->SetColorAndOpacity(bIsAiming ? AimDotColor : NormalDotColor);
	}

	if (Image_ChargeRing)
	{
		Image_ChargeRing->SetVisibility(
			bShowChargeRing ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden
		);
	}

	const float RingSize = FMath::Lerp(ChargeRingMaxSize, ChargeRingMinSize, ChargeAlpha);

	if (SizeBox_ChargeRing)
	{
		SizeBox_ChargeRing->SetWidthOverride(RingSize);
		SizeBox_ChargeRing->SetHeightOverride(RingSize);
	}
}
