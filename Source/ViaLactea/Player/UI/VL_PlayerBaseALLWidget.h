// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/UI/VL_UMGBase.h"
#include "VL_PlayerBaseALLWidget.generated.h"

/**
 *
 */
class UVL_PlayerCrosshairWidget;
class UVL_PlayerInfoUMG;
class UVL_PlayerQuickSlot;

UCLASS()
class VIALACTEA_API UVL_PlayerBaseALLWidget : public UVL_UMGBase
{
    GENERATED_BODY()
public:
    FORCEINLINE UVL_PlayerCrosshairWidget* GetPlayerCrosshairWidget() const { return PlayerCrosshairWidget; }
    FORCEINLINE UVL_PlayerQuickSlot* GetPlayerQuickSlotWidget() const { return VLWBP_PlayerQuickSlot; }

    FORCEINLINE UVL_PlayerInfoUMG* GetPlayerInfoWidget() const { return VLWBP_PlayerInfoall; }
protected:
    UPROPERTY(meta = (BindWidget))
    UVL_PlayerCrosshairWidget* PlayerCrosshairWidget;

    UPROPERTY(meta = (BindWidget))
    UVL_PlayerQuickSlot* VLWBP_PlayerQuickSlot;

    UPROPERTY(meta = (BindWidget))
    UVL_PlayerInfoUMG* VLWBP_PlayerInfoall;
};
