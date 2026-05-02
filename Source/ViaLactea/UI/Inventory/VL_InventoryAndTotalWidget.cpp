// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/VL_InventoryAndTotalWidget.h"

#include "Player/VL_PlayerState.h"
#include "Base/Component/VL_InventoryComponent.h"

#include "UI/Inventory/VL_InventoryMainWidget.h"
#include "Player/UI/VL_PlayerQuickSlot.h"

#include "GameFramework/PlayerController.h"

void UVL_InventoryAndTotalWidget::NativeConstruct()
{
    Super::NativeConstruct();

    APlayerController* PC = GetOwningPlayer();
    if (!PC) return;

    CachedPlayerState = PC->GetPlayerState<AVL_PlayerState>();
    if (!CachedPlayerState) return;

    CachedInventoryComponent = CachedPlayerState->GetInventoryComponent();
    if (!CachedInventoryComponent) return;

    CachedPlayerState->OnInventoryChanged.AddUniqueDynamic(this, &UVL_InventoryAndTotalWidget::RefreshAll);
    CachedPlayerState->OnQuickSlotChanged.AddUniqueDynamic(this, &UVL_InventoryAndTotalWidget::RefreshAll);

    RefreshAll();
}

void UVL_InventoryAndTotalWidget::RefreshAll()
{
    if (WBP_InventoryMain)
    {
        WBP_InventoryMain->SetInventoryComponent(CachedInventoryComponent);
        WBP_InventoryMain->RefreshInventory();
    }

    if (VLWBP_PlayerQuickSlot)
    {
        VLWBP_PlayerQuickSlot->SetInventoryComponent(CachedInventoryComponent);
    }
}