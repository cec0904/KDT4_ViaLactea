// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/UI/VL_QuickSlotOne.h"

#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"

#include "UI/Inventory/VL_ItemDragDropOperation.h"
#include "Player/VL_PlayerState.h"
#include "Base/Component/VL_InventoryComponent.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Input/Reply.h"

#include "Player/VL_PlayerController.h"

void UVL_QuickSlotOne::NativeConstruct()
{
    Super::NativeConstruct();

    //UE_LOG(LogTemp, Warning, TEXT("[SlotOne] Construct SlotButton=%s SelectionVisual=%s SlotNumberText=%s"),
    //    SlotButton ? TEXT("OK") : TEXT("NULL"),
    //    SelectionVisual ? TEXT("OK") : TEXT("NULL"),
    //    SlotNumberText ? TEXT("OK") : TEXT("NULL"));

    //초반 상태 초기화
    ClearSlot();
    SetHighlight(false);

    //버튼클릭시
    if (SlotButton)
    {

        SlotButton->OnClicked.RemoveAll(this);
        SlotButton->OnClicked.AddDynamic(this, &UVL_QuickSlotOne::OnSlotClicked);
    }
}

FReply UVL_QuickSlotOne::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
    {
        APlayerController* PC = GetOwningPlayer();
        if (!PC)
        {
            return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
        }

        AVL_PlayerState* PS = PC->GetPlayerState<AVL_PlayerState>();
        if (!PS)
        {
            return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
        }

        UVL_InventoryComponent* InvComp = PS->GetInventoryComponent();
        if (!InvComp)
        {
            return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
        }

        const TArray<FInventorySlotData>& QuickSlots = InvComp->GetQuickSlots();
        if (QuickSlots.IsValidIndex(SlotIndex) && !QuickSlots[SlotIndex].IsEmpty())
        {
            return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
        }
    }

    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UVL_QuickSlotOne::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    Super::NativeOnDragCancelled(InDragDropEvent, InOperation);

    APlayerController* PC = GetOwningPlayer();
    if (!PC) return;

    AVL_PlayerState* PS = PC->GetPlayerState<AVL_PlayerState>();
    if (!PS) return;

    UVL_InventoryComponent* InvComp = PS->GetInventoryComponent();
    if (!InvComp) return;

    InvComp->ClearHeldItem();

    //UE_LOG(LogTemp, Warning, TEXT("QuickSlot Drag Cancelled -> HeldItem Cleared"));
}

void UVL_QuickSlotOne::OnSlotClicked()
{
    OnQuickSlotClicked.Broadcast(SlotIndex);

    //UE_LOG(LogTemp, Warning, TEXT("%d번 슬롯 클릭됨!"), SlotIndex);
}

void UVL_QuickSlotOne::SetSlotIndex(int32 InSlotIndex)
{
    //헤더에 저장한 인덱스 값 저장
    SlotIndex = InSlotIndex;

    if (SlotNumberText)
    {
        SlotNumberText->SetText(FText::AsNumber(InSlotIndex + 1));
    }
}

void UVL_QuickSlotOne::UpdateSlotDisplay(UTexture2D* NewIcon, int32 NewCount)
{
    if (NewIcon && ItemIcon)
    {
        ItemIcon->SetBrushFromTexture(NewIcon);
        ItemIcon->SetRenderOpacity(1.0f); //투명도
    }

    //아이템 갯수 텍스트
    if (ItemCountText)
    {
        //아이템 갯수가 1개보다 많을때만 숫자 표시
        if (NewCount > 1)
        {
            ItemCountText->SetText(FText::AsNumber(NewCount));
            ItemCountText->SetVisibility(ESlateVisibility::Visible);
        }
        else
        {
            ItemCountText->SetVisibility(ESlateVisibility::Collapsed); // 숨김
        }
    }
}

void UVL_QuickSlotOne::SetHighlight(bool bIsSelected)
{
    //
    if (SelectionVisual)
    {
        SelectionVisual->SetVisibility(bIsSelected ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    }
    else
    {
       // UE_LOG(LogTemp, Error, TEXT("SelectionVisual is NULL (BindWidget failed)"));
    }

    /*UE_LOG(LogTemp, Warning, TEXT("SetHighlight called | SlotIndex=%d | Selected=%d"),
        SlotIndex, bIsSelected);*/

}

void UVL_QuickSlotOne::ClearSlot()
{
    if (ItemIcon)
    {
        ItemIcon->SetRenderOpacity(0.0f);
    }
    if (ItemCountText)
    {
        ItemCountText->SetVisibility(ESlateVisibility::Collapsed);
    }
}

bool UVL_QuickSlotOne::NativeOnDrop(
    const FGeometry& InGeometry,
    const FDragDropEvent& InDragDropEvent,
    UDragDropOperation* InOperation)
{
    //UE_LOG(LogTemp, Warning, TEXT("[QuickSlotOne] NativeOnDrop Start | SlotIndex=%d"), SlotIndex);

    UVL_ItemDragDropOperation* DragOp = Cast<UVL_ItemDragDropOperation>(InOperation);
    if (!DragOp)
    {
        return false;
    }

    APlayerController* PC = GetOwningPlayer();
    if (!PC)
    {
        return false;
    }

    AVL_PlayerController* VLPC = Cast<AVL_PlayerController>(PC);
    if (!VLPC)
    {
        return false;
    }

    AVL_PlayerState* PS = PC->GetPlayerState<AVL_PlayerState>();
    if (!PS)
    {
        return false;
    }

    UVL_InventoryComponent* InvComp = PS->GetInventoryComponent();
    if (!InvComp)
    {
        return false;
    }

    if (DragOp->FromSlotType == EVLSlotType::QuickSlot &&
        DragOp->FromSlotIndex == SlotIndex)
    {
        InvComp->ClearHeldItem();
        //UE_LOG(LogTemp, Warning, TEXT("[QuickSlotOne] Drop ignored: Same Slot"));
        return false;
    }

    VLPC->ServerMoveOrSwapSlot(
        DragOp->FromSlotType,
        DragOp->FromSlotIndex,
        EVLSlotType::QuickSlot,
        SlotIndex
    );

    InvComp->ClearHeldItem();

    //UE_LOG(LogTemp, Warning, TEXT("[QuickSlotOne] Move Request Sent | From=%d %d To=QuickSlot %d"),
    //    (int32)DragOp->FromSlotType,
    //    DragOp->FromSlotIndex,
    //    SlotIndex);

    return true;
}

void UVL_QuickSlotOne::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
    Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

    APlayerController* PC = GetOwningPlayer();
    if (!PC) return;

    AVL_PlayerState* PS = PC->GetPlayerState<AVL_PlayerState>();
    if (!PS) return;

    UVL_InventoryComponent* InvComp = PS->GetInventoryComponent();
    if (!InvComp) return;

    const TArray<FInventorySlotData>& QuickSlots = InvComp->GetQuickSlots();
    if (!QuickSlots.IsValidIndex(SlotIndex) || QuickSlots[SlotIndex].IsEmpty()) return;

    UVL_ItemDragDropOperation* DragOp = NewObject<UVL_ItemDragDropOperation>();
    DragOp->FromSlotType = EVLSlotType::QuickSlot;
    DragOp->FromSlotIndex = SlotIndex;
    DragOp->DraggedItem = QuickSlots[SlotIndex];
    InvComp->SetHeldItem(QuickSlots[SlotIndex]);
    OutOperation = DragOp;

    //UE_LOG(LogTemp, Warning, TEXT("QuickSlot Drag Start: %d"), SlotIndex);
}
