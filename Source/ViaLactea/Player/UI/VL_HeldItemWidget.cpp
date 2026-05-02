// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/UI/VL_HeldItemWidget.h"

#include "Base/Component/VL_InventoryComponent.h"
#include "Base/Data/VL_DataRows.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanelSlot.h"
#include "Engine/Texture2D.h"
#include "GameFramework/PlayerController.h"

#include "Framework/Application/SlateApplication.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/SlateBlueprintLibrary.h"

void UVL_HeldItemWidget::NativeConstruct()
{
    Super::NativeConstruct();

    SetVisibility(ESlateVisibility::HitTestInvisible);
    RefreshHeldItem();
}

void UVL_HeldItemWidget::NativeDestruct()
{
    if (InventoryComponent)
    {
        InventoryComponent->OnHeldItemChanged.RemoveDynamic(this, &UVL_HeldItemWidget::RefreshHeldItem);
    }

    Super::NativeDestruct();
}

void UVL_HeldItemWidget::SetInventoryComponent(UVL_InventoryComponent* NewInventoryComponent)
{
    if (InventoryComponent == NewInventoryComponent)
    {
        return;
    }

    if (InventoryComponent)
    {
        InventoryComponent->OnHeldItemChanged.RemoveDynamic(this, &UVL_HeldItemWidget::RefreshHeldItem);
    }

    InventoryComponent = NewInventoryComponent;

    if (InventoryComponent)
    {
        InventoryComponent->OnHeldItemChanged.AddDynamic(this, &UVL_HeldItemWidget::RefreshHeldItem);
    }

    RefreshHeldItem();
}

void UVL_HeldItemWidget::RefreshHeldItem()
{
    if (!InventoryComponent)
    {
        SetVisibility(ESlateVisibility::Collapsed);
        return;
    }

    if (!InventoryComponent->HasHeldItem())
    {
        SetVisibility(ESlateVisibility::Collapsed);
        return;
    }

    const FInventorySlotData& HeldItem = InventoryComponent->GetHeldItem();
    if (HeldItem.IsEmpty())
    {
        SetVisibility(ESlateVisibility::Collapsed);
        return;
    }

    const FItemDataRow* ItemData = InventoryComponent->GetItemData(HeldItem.ItemID);
    if (!ItemData)
    {
        SetVisibility(ESlateVisibility::Collapsed);
        return;
    }

    // 아이콘 먼저 세팅
    if (ItemIcon)
    {
        ItemIcon->SetBrushFromTexture(ItemData->Icon);
        ItemIcon->SetVisibility(ESlateVisibility::HitTestInvisible);
    }

    // 수량 텍스트 세팅
    if (QuantityText)
    {
        if (HeldItem.Quantity > 1)
        {
            QuantityText->SetText(FText::AsNumber(HeldItem.Quantity));
            QuantityText->SetVisibility(ESlateVisibility::HitTestInvisible);
        }
        else
        {
            QuantityText->SetText(FText::GetEmpty());
            QuantityText->SetVisibility(ESlateVisibility::Collapsed);
        }
    }

    // 보이기 전에 위치 먼저 갱신
    UpdatePositionToMouse();

    // 자식까지 전부 마우스/드롭 이벤트 안 먹게 함
    SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UVL_HeldItemWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    UpdatePositionToMouse();
}

void UVL_HeldItemWidget::UpdatePositionToMouse()
{
    if (GetVisibility() == ESlateVisibility::Collapsed)
    {
        return;
    }

    const FVector2D MousePos = UWidgetLayoutLibrary::GetMousePositionOnViewport(this);

    SetPositionInViewport(MousePos - FVector2D(36.f, 36.f), false);
}
