// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/UI/VL_UMGBase.h"
#include "VL_InventoryMainWidget.generated.h"

class AVL_PlayerState;
class UVL_InventoryComponent;
class UWidgetSwitcher;
class UUserWidget;
class UVL_PlayerQuickSlot;

/**
 *
 */
UCLASS()
class VIALACTEA_API UVL_InventoryMainWidget : public UVL_UMGBase
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

public:
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void InitializeFromPlayerState();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void RefreshInventory();

    UFUNCTION(BlueprintCallable)
    void SetInventoryComponent(class UVL_InventoryComponent* InInventoryComponent);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Inventory")
    TObjectPtr<AVL_PlayerState> CachedPlayerState;

    UPROPERTY(BlueprintReadOnly, Category = "Inventory")
    TObjectPtr<UVL_InventoryComponent> CachedInventoryComponent;

    UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
    TObjectPtr<UUserWidget> WBP_InventoryGrid;

};
