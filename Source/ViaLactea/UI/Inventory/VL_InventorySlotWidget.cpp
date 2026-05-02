// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/VL_InventorySlotWidget.h"
#include "Base/Component/VL_InventoryComponent.h"
#include "UI/Inventory/VL_ItemDragDropOperation.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Player/VL_PlayerController.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "Base/Data/VL_DataRows.h"

#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

void UVL_InventorySlotWidget::InitSlot(UVL_InventoryComponent* InInventoryComponent, EVLSlotType InSlotType, int32 InSlotIndex)
{
    InventoryComponent = InInventoryComponent;
    SlotType = InSlotType;
    SlotIndex = InSlotIndex;
}

void UVL_InventorySlotWidget::SetSlotData(const FInventorySlotData& InData)
{
    SlotData = InData;

    //아이콘, 수량 텍스트 갱신

    if (!IMG_ItemIcon || !TXT_Quantity)
    {
        return;
    }

    if (SlotData.IsEmpty())
    {
        TXT_Quantity->SetText(FText::GetEmpty());
        IMG_ItemIcon->SetBrushFromTexture(nullptr);
        IMG_ItemIcon->SetVisibility(ESlateVisibility::Hidden);
        return;
    }

    //수량 표시
    if (SlotData.Quantity > 1)
    {
        TXT_Quantity->SetText(FText::AsNumber(SlotData.Quantity));
    }
    else
    {
        TXT_Quantity->SetText(FText::GetEmpty());
    }

    //아이콘 표시
    if (ItemDataTable)
    {
        static const FString ContextString(TEXT("InventorySlotWidget_SetSlotData"));

        if (const FItemDataRow* ItemRow = ItemDataTable->FindRow<FItemDataRow>(SlotData.ItemID, ContextString))
        {
            IMG_ItemIcon->SetBrushFromTexture(ItemRow->Icon);
            IMG_ItemIcon->SetVisibility(ESlateVisibility::Visible);
        }
        else
        {
            IMG_ItemIcon->SetBrushFromTexture(nullptr);
            IMG_ItemIcon->SetVisibility(ESlateVisibility::Hidden);
        }
    }
    else
    {
        IMG_ItemIcon->SetBrushFromTexture(nullptr);
        IMG_ItemIcon->SetVisibility(ESlateVisibility::Hidden);
    }

}

FReply UVL_InventorySlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    APlayerController* PC = GetOwningPlayer();
    if (AVL_PlayerController* VLPC = Cast<AVL_PlayerController>(PC))
    {
        VLPC->SetSelectedInventorySlot(SlotType, SlotIndex);
    }

    if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton) && !SlotData.IsEmpty())
    {
        return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
    }

    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UVL_InventorySlotWidget::NativeOnDragDetected(
    const FGeometry& InGeometry,
    const FPointerEvent& InMouseEvent,
    UDragDropOperation*& OutOperation)
{
    Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

    if (!InventoryComponent || SlotData.IsEmpty())
    {
        return;
    }

    //아이템정보를담을 상자 생성
    UVL_ItemDragDropOperation* DragOp = NewObject<UVL_ItemDragDropOperation>();
    if (!DragOp)
    {
        return;
    }
    //드래그 정보가 어떤것인지 저장 (아이템, 슬롯, 슬롯 번호)
    DragOp->DraggedItem = SlotData;
    DragOp->FromSlotType = SlotType;
    DragOp->FromSlotIndex = SlotIndex;

    // 여기서 커서가 들고 있는 아이템으로 저장 = 마우스가 아이템을 들고 있는 상태
    InventoryComponent->SetHeldItem(SlotData);

    // 아이템을 실제로 집는 데 성공한 순간 사운드 재생
    PlayUISound(PickItemSound);

    //이 정보를 UMG시스템에 전달.
    OutOperation = DragOp;

    bIsBeingDragged = true;
    SetRenderOpacity(0.3f);
}

bool UVL_InventorySlotWidget::NativeOnDrop(
    const FGeometry& InGeometry,
    const FDragDropEvent& InDragDropEvent,
    UDragDropOperation* InOperation)
{
    Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

    UE_LOG(LogTemp, Warning, TEXT("NativeOnDrop Start"));

    if (!InventoryComponent || !InOperation)
    {
        UE_LOG(LogTemp, Warning, TEXT("NativeOnDrop Fail: InventoryComponent null"));
        return false;
    }

    //드래그 정보 가져오기
    UVL_ItemDragDropOperation* DragOp = Cast<UVL_ItemDragDropOperation>(InOperation);
    if (!DragOp)
    {
        UE_LOG(LogTemp, Warning, TEXT("NativeOnDrop Fail: DragOp Cast Failed"));
        return false;
    }

    UE_LOG(LogTemp, Warning, TEXT("NativeOnDrop Start"));
    UE_LOG(LogTemp, Warning, TEXT("From: %d %d / To: %d %d"),
        (int32)DragOp->FromSlotType, DragOp->FromSlotIndex,
        (int32)SlotType, SlotIndex);

    //같은 슬롯인지 확인
    if (DragOp->FromSlotType == SlotType && DragOp->FromSlotIndex == SlotIndex)
    {
        //같은 슬롯이면 드롭 처리 안함
        InventoryComponent->ClearHeldItem();
        UE_LOG(LogTemp, Warning, TEXT("NativeOnDrop Fail: Same Slot"));
        return false;
    }


    ////InventoryComponent에 구현할 핵심 함수
    //bool bSuccess = InventoryComponent->MoveOrSwapSlot(
    //    DragOp->FromSlotType,
    //    DragOp->FromSlotIndex,
    //    SlotType,
    //    SlotIndex
    //);

    //UE_LOG(LogTemp, Warning, TEXT("MoveOrSwapSlot Result = %d"), bSuccess);

    //if (bSuccess)
    //{
    //    InventoryComponent->ClearHeldItem();
    //}

    //bIsBeingDragged = false;
    //SetRenderOpacity(1.0f);

    //return bSuccess;

    APlayerController* PC = GetOwningPlayer();
    AVL_PlayerController* VLPC = Cast<AVL_PlayerController>(PC);
    if (!VLPC)
    {
        UE_LOG(LogTemp, Warning, TEXT("NativeOnDrop Fail: PlayerController Cast Failed"));
        return false;
    }

    VLPC->ServerMoveOrSwapSlot(
        DragOp->FromSlotType,
        DragOp->FromSlotIndex,
        SlotType,
        SlotIndex
    );

    // HeldItem은 UI용 임시 표시니까 클라에서 바로 지워도 됨
    InventoryComponent->ClearHeldItem();

    bIsBeingDragged = false;
    SetRenderOpacity(1.0f);

    return true;
}

void UVL_InventorySlotWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    Super::NativeOnDragCancelled(InDragDropEvent, InOperation);

    if (!InventoryComponent)
    {
        return;
    }

    InventoryComponent->ClearHeldItem();

    bIsBeingDragged = false;
    SetRenderOpacity(1.0f);

    UE_LOG(LogTemp, Warning, TEXT("NativeOnDragCancelled: HeldItem Cleared"));
}
