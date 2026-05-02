// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/UI/VL_PlayerQuickSlot.h"
#include "Player/UI/VL_QuickSlotOne.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"

#include "Base/Component/VL_InventoryComponent.h"
#include "Base/Data/VL_CommonStructs.h"
#include "Player/VL_PlayerController.h"
#include "Base/Data/VL_DataRows.h"

#include "Player/VL_PlayerState.h"
#include "GameFramework/PlayerController.h"

void UVL_PlayerQuickSlot::NativeConstruct()
{
    Super::NativeConstruct();

    QuickSlotArray.Empty();

    if (!HB_Slots || !QuickSlotOneClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayerQuickSlot::NativeConstruct HB_Slots or QuickSlotOneClass is nullptr"));
        return;
    }

    HB_Slots->ClearChildren();

    //SlotWidget <- Slot 으로 쓰면 UWidget이랑 오류남.
    for (int32 i = 0; i < 9; ++i)
    {
        //OwningPlayer 설정
        APlayerController* PC = GetOwningPlayer();
        UVL_QuickSlotOne* SlotWidget = CreateWidget<UVL_QuickSlotOne>(PC, QuickSlotOneClass);

        if (!SlotWidget)
        {
            UE_LOG(LogTemp, Error, TEXT("PlayerQuickSlot: SlotWidget Create Failed"));
            continue;
        }

        // 2 내부 인덱스는 0~8로 저장. 표시는 위젯 슬롯에서 +1
        SlotWidget->SetSlotIndex(i);

        // 3 클릭 이벤트를 부모로 연결 (인덱스 같이 넘어옴)
        SlotWidget->OnQuickSlotClicked.AddDynamic(this, &UVL_PlayerQuickSlot::OnSlotClicked);

        // UI에 붙이기
        if (UHorizontalBoxSlot* HSlot = HB_Slots->AddChildToHorizontalBox(SlotWidget))
        {
            HSlot->SetPadding(FMargin(5.f, 0.f));
            HSlot->SetVerticalAlignment(VAlign_Center);
        }

        // 5 배열에 보관
        QuickSlotArray.Add(SlotWidget);
    }

    CurrentSelectedSlot = 0;
    UpdateSelectionVisual();

    UE_LOG(LogTemp, Warning, TEXT("PlayerQuickSlot NativeConstruct | Widget=%s"),
        *GetName());

}

void UVL_PlayerQuickSlot::OnSlotClicked(int32 SlotIndex)
{
    SelectSlot(SlotIndex);
    UE_LOG(LogTemp, Warning, TEXT("슬롯 클릭: %d"), CurrentSelectedSlot);
}

void UVL_PlayerQuickSlot::SelectSlot(int32 NewIndex)
{
    CurrentSelectedSlot = FMath::Clamp(NewIndex, 0, 8);
    UpdateSelectionVisual();
}

void UVL_PlayerQuickSlot::SetInventoryComponent(UVL_InventoryComponent* InInventoryComponent)
{
    InventoryComponent = InInventoryComponent;
    UE_LOG(LogTemp, Warning, TEXT("QuickSlot InvComp Set: %s"),
        InventoryComponent ? TEXT("OK") : TEXT("NULL"));

    if (!InventoryComponent)
    {
        return;
    }

    APlayerController* PC = GetOwningPlayer();
    if (!PC)
    {
        return;
    }

    AVL_PlayerState* PS = PC->GetPlayerState<AVL_PlayerState>();
    if (!PS)
    {
        return;
    }

    PS->OnQuickSlotChanged.RemoveDynamic(this, &UVL_PlayerQuickSlot::HandleQuickSlotChanged);
    PS->OnQuickSlotChanged.AddUniqueDynamic(this, &UVL_PlayerQuickSlot::HandleQuickSlotChanged);

    UE_LOG(LogTemp, Warning, TEXT("QuickSlot Bound OnQuickSlotChanged | Widget=%s | Addr=%p"),
        *GetName(),
        this);

    RefreshQuickSlot();

}

void UVL_PlayerQuickSlot::RefreshQuickSlot()
{
    if (!InventoryComponent)
    {
        return;
    }

    const TArray<FInventorySlotData>& QuickSlots = InventoryComponent->GetQuickSlots();
    UE_LOG(LogTemp, Warning, TEXT("RefreshQuickSlot | QuickSlots Num = %d"), QuickSlots.Num());

    for (int32 i = 0; i < QuickSlotArray.Num(); ++i)
    {
        UE_LOG(LogTemp, Warning, TEXT("RefreshQuickSlot: i=%d IsEmpty=%d Quantity=%d"),
            i,
            QuickSlots.IsValidIndex(i) ? QuickSlots[i].IsEmpty() : -1,
            QuickSlots.IsValidIndex(i) ? QuickSlots[i].Quantity : -1);

        if (!QuickSlotArray[i]) continue;

        if (QuickSlots.IsValidIndex(i) && !QuickSlots[i].IsEmpty())
        {
            const FItemDataRow* ItemData = InventoryComponent->GetItemData(QuickSlots[i].ItemID);
            UTexture2D* ItemIcon = ItemData ? ItemData->Icon : nullptr;

            QuickSlotArray[i]->UpdateSlotDisplay(ItemIcon, QuickSlots[i].Quantity);
        }
        else
        {
            QuickSlotArray[i]->ClearSlot();
        }
    }

}

void UVL_PlayerQuickSlot::HandleQuickSlotChanged()
{
    UE_LOG(LogTemp, Warning, TEXT("SetInventoryComponent | Widget=%s | Addr=%p"),
        *GetName(),
        this);
    RefreshQuickSlot();
}

void UVL_PlayerQuickSlot::UpdateSelectionVisual()
{
    for (int32 i = 0; i < QuickSlotArray.Num(); ++i)
    {
        if (QuickSlotArray[i])
        {
            QuickSlotArray[i]->SetHighlight(i == CurrentSelectedSlot);
        }
    }
}
