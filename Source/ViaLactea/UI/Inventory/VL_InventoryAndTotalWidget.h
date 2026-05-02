// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/UI/VL_UMGBase.h"
#include "VL_InventoryAndTotalWidget.generated.h"

/**
 * 
 */
class UVL_InventoryMainWidget;
class UVL_PlayerQuickSlot;
class AVL_PlayerState;
class UVL_InventoryComponent;

UCLASS()
class VIALACTEA_API UVL_InventoryAndTotalWidget : public UVL_UMGBase
{
	GENERATED_BODY()
	
public:

protected:

    virtual void NativeConstruct() override;

    UPROPERTY(meta = (BindWidget))
    class UVL_InventoryMainWidget* WBP_InventoryMain;

    UPROPERTY(meta = (BindWidget))
    class UVL_PlayerQuickSlot* VLWBP_PlayerQuickSlot;

    UPROPERTY()
    class AVL_PlayerState* CachedPlayerState;

    UPROPERTY()
    class UVL_InventoryComponent* CachedInventoryComponent;

    UFUNCTION()
    void RefreshAll();

};
