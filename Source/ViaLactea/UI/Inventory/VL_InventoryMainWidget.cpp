// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/VL_InventoryMainWidget.h"

#include "Player/VL_PlayerState.h"
#include "Base/Component/VL_InventoryComponent.h"

#include "GameFramework/PlayerController.h"
#include "Player/UI/VL_PlayerQuickSlot.h"
#include "Player/VL_PlayerController.h"

void UVL_InventoryMainWidget::NativeConstruct()
{
    Super::NativeConstruct();

    InitializeFromPlayerState();
    RefreshInventory();
}

void UVL_InventoryMainWidget::InitializeFromPlayerState()
{
    APlayerController* PC = GetOwningPlayer();
    if (!PC)
    {
        UE_LOG(LogTemp, Warning, TEXT("InventoryMainWidget: OwningPlayer is null"));
        return;
    }

    CachedPlayerState = PC->GetPlayerState<AVL_PlayerState>();
    if (!CachedPlayerState)
    {
        UE_LOG(LogTemp, Warning, TEXT("InventoryMainWidget: PlayerState is null"));
        return;
    }

    CachedInventoryComponent = CachedPlayerState->GetInventoryComponent();
    if (!CachedInventoryComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("InventoryMainWidget: InventoryComponent is null"));
        return;
    }



    CachedPlayerState->OnInventoryChanged.AddUniqueDynamic(this, &UVL_InventoryMainWidget::RefreshInventory);
    UE_LOG(LogTemp, Warning, TEXT("InventoryMainWidget: Bind OnInventoryChanged Success"));
    UE_LOG(LogTemp, Warning, TEXT("InventoryMainWidget: InitializeFromPlayerState Success"));
}

void UVL_InventoryMainWidget::RefreshInventory()
{
    UE_LOG(LogTemp, Warning, TEXT("InventoryMainWidget: RefreshInventory Start"));

    if (!CachedInventoryComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("InventoryMainWidget: CachedInventoryComponent is NULL"));
        return;
    }

    if (!WBP_InventoryGrid)
    {
        UE_LOG(LogTemp, Error, TEXT("InventoryMainWidget: WBP_InventoryGrid is NULL"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("InventoryMainWidget: Grid Widget Found"));

    // 1. InventoryComponent 변수 넣기
    FObjectProperty* ObjProp =
        FindFProperty<FObjectProperty>(WBP_InventoryGrid->GetClass(), TEXT("InventoryComponent"));

    if (!ObjProp)
    {
        UE_LOG(LogTemp, Error, TEXT("InventoryMainWidget: InventoryComponent property NOT found in Grid"));
        return;
    }

    ObjProp->SetObjectPropertyValue_InContainer(WBP_InventoryGrid, CachedInventoryComponent);
    UE_LOG(LogTemp, Warning, TEXT("InventoryMainWidget: InventoryComponent injected into Grid"));

    // 2. BuildInventoryGrid 호출
    UFunction* Func = WBP_InventoryGrid->FindFunction(TEXT("BuildInventoryGrid"));
    if (!Func)
    {
        UE_LOG(LogTemp, Error, TEXT("InventoryMainWidget: BuildInventoryGrid function NOT found"));
        return;
    }

    WBP_InventoryGrid->ProcessEvent(Func, nullptr);
    UE_LOG(LogTemp, Warning, TEXT("InventoryMainWidget: BuildInventoryGrid called"));

}

void UVL_InventoryMainWidget::SetInventoryComponent(UVL_InventoryComponent* InInventoryComponent)
{
    CachedInventoryComponent = InInventoryComponent;
}