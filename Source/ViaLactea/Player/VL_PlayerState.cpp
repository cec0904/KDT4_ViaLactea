// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/VL_PlayerState.h"
#include "Base/Component/VL_InventoryComponent.h"
#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"

AVL_PlayerState::AVL_PlayerState()
{
    UE_LOG(LogTemp, Warning, TEXT("AVL_PlayerState Constructor Called"));

    InventoryComponent = CreateDefaultSubobject<UVL_InventoryComponent>(TEXT("InventoryComponent"));

}

void AVL_PlayerState::SetPlayerIdentity(const FString& InUserId, int32 InCharacterSlotId, const FString& InCharacterName)
{
    PlayerSaveData.UserId = InUserId;
    PlayerSaveData.CharacterSlotId = InCharacterSlotId;
    PlayerSaveData.CharacterName = InCharacterName;
}

FVLPlayerSaveData AVL_PlayerState::BuildPlayerSaveData() const
{
    FVLPlayerSaveData OutSaveData = PlayerSaveData;

    const APawn* OwnerPawn = GetPawn();
    if (OwnerPawn)
    {
        OutSaveData.SavedTransform = OwnerPawn->GetActorTransform();
    }

    return OutSaveData;
}

void AVL_PlayerState::ApplyPlayerSaveData(const FVLPlayerSaveData& InSaveData)
{
    PlayerSaveData = InSaveData;

    APawn* OwnerPawn = GetPawn();
    if (OwnerPawn)
    {
        OwnerPawn->SetActorTransform(PlayerSaveData.SavedTransform);
    }
}

void AVL_PlayerState::BeginPlay()
{
    Super::BeginPlay();



    UE_LOG(LogTemp, Warning, TEXT("AVL_PlayerState BeginPlay Called"));

    if (HasAuthority())
    {
        UE_LOG(LogTemp, Warning, TEXT("Server: Initialize Inventory / QuickSlots"));
        InitializeInventory();
        InitializeQuickSlots();

        if (bUseDemoInventory)
        {
            InitDemoInventory();
        }
    }

}

void AVL_PlayerState::InitDemoInventory()
{
    if (!HasAuthority() || !InventoryComponent)
    {
        return;
    }

    // 이미 뭔가 들어있으면 다시 지급 안 함
    if (InventorySlots.Num() > 0)
    {
        for (const FInventorySlotData& Slot : InventorySlots)
        {
            if (!Slot.IsEmpty())
            {
                UE_LOG(LogTemp, Warning, TEXT("InitDemoInventory Skipped: Inventory already has items."));
                return;
            }
        }
    }

    InventoryComponent->AddItem(TEXT("Potion_HP_Small"), 3);
    InventoryComponent->AddItem(TEXT("Sword_01"), 1);
    InventoryComponent->AddItem(TEXT("Shield_01"), 1);
    InventoryComponent->AddItem(TEXT("Bow_01"), 1);
    InventoryComponent->AddItem(TEXT("Arrow_01"), 50);
    InventoryComponent->AddItem(TEXT("GreatSword_01"), 1);
    InventoryComponent->AddItem(TEXT("Pickex"), 1);
    InventoryComponent->AddItem(TEXT("GreatHammer_01"), 1);
    InventoryComponent->AddItem(TEXT("Hammer_01"), 1);

    UE_LOG(LogTemp, Warning, TEXT("InitDemoInventory Success"));
}

void AVL_PlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AVL_PlayerState, InventorySlots);
    DOREPLIFETIME(AVL_PlayerState, QuickSlots);
}

void AVL_PlayerState::OnRep_InventorySlots()
{
    UE_LOG(LogTemp, Warning, TEXT("OnRep_InventorySlots Called | Num=%d"), InventorySlots.Num());
    OnInventoryChanged.Broadcast();
}

void AVL_PlayerState::OnRep_QuickSlots()
{
    UE_LOG(LogTemp, Warning, TEXT("OnRep_QuickSlots Called | Num=%d"), QuickSlots.Num());
    OnQuickSlotChanged.Broadcast();
}

void AVL_PlayerState::InitializeInventory()
{
    if (!HasAuthority())
    {
        return;
    }

    if (MaxSlotCount <= 0)
    {
        return;
    }

    // 중복 초기화 방지
    if (InventorySlots.Num() == MaxSlotCount)
    {
        return;
    }

    //슬롯 개수만큼 배열 생성
    InventorySlots.SetNum(MaxSlotCount);
    OnInventoryChanged.Broadcast();
    UE_LOG(LogTemp, Warning, TEXT("Inventory Num = %d, QuickSlots Num = %d"),
        InventorySlots.Num(), QuickSlots.Num());
}

void AVL_PlayerState::InitializeQuickSlots()
{
    if (!HasAuthority())
    {
        return;
    }

    if (MaxQuickSlotCount <= 0)
    {
        return;
    }

    // 중복 초기화 방지
    if (QuickSlots.Num() == MaxQuickSlotCount)
    {
        return;
    }


    QuickSlots.SetNum(MaxQuickSlotCount);
    UE_LOG(LogTemp, Warning, TEXT("Inventory Num = %d, QuickSlots Num = %d"),
        InventorySlots.Num(), QuickSlots.Num());
}

const TArray<FInventorySlotData>& AVL_PlayerState::GetInventorySlots() const
{
    return InventorySlots;
}

const TArray<FInventorySlotData>& AVL_PlayerState::GetQuickSlots() const
{
    return QuickSlots;
}
