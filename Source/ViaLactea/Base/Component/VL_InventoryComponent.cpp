// Fill out your copyright notice in the Description page of Project Settings.


#include "Base/Component/VL_InventoryComponent.h"
#include "Engine/DataTable.h"
#include "Base/Data/VL_DataRows.h"

#include "Player/VL_PlayerState.h"
#include "GameFramework/Actor.h"

#include "Player/MainCharacterBase.h"
#include "Player/VL_PlayerController.h"
#include "GameFramework/PlayerController.h"

#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

// Sets default values for this component's properties
UVL_InventoryComponent::UVL_InventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

void UVL_InventoryComponent::BroadcastInventoryChanged()
{
	AVL_PlayerState* PS = GetVLPlayerState();
	if (PS)
	{
		UE_LOG(LogTemp, Warning, TEXT("BroadcastInventoryChanged Called"));
		PS->OnInventoryChanged.Broadcast();
	}
}

void UVL_InventoryComponent::BroadcastQuickSlotChanged()
{
	AVL_PlayerState* PS = GetVLPlayerState();
	if (PS)
	{
		PS->OnQuickSlotChanged.Broadcast();
	}
}


// Called when the game starts
void UVL_InventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	CachedPlayerState = Cast<AVL_PlayerState>(GetOwner());

	if (!CachedPlayerState)
	{
		UE_LOG(LogTemp, Error, TEXT("UVL_InventoryComponent BeginPlay Failed: Owner is not AVL_PlayerState."));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UVL_InventoryComponent BeginPlay Success: CachedPlayerState = %s"),
			*GetNameSafe(CachedPlayerState));
	}

}

AVL_PlayerState* UVL_InventoryComponent::GetVLPlayerState() const
{
	AVL_PlayerState* PS = CachedPlayerState.Get();

	if (!PS)
	{
		PS = Cast<AVL_PlayerState>(GetOwner());
	}

	return PS;
}

const TArray<FInventorySlotData>& UVL_InventoryComponent::GetQuickSlots() const
{
	AVL_PlayerState* PS = GetVLPlayerState();
	if (!PS)
	{
		static TArray<FInventorySlotData> Empty;
		return Empty;
	}

	return PS->GetQuickSlotsRef();
}

bool UVL_InventoryComponent::UseQuickSlot(int32 QuickSlotIndex)
{
	AVL_PlayerState* PS = GetVLPlayerState();
	if (!PS)
	{
		UE_LOG(LogTemp, Warning, TEXT("UseQuickSlot Failed: PlayerState is nullptr."));
		return false;
	}

	const TArray<FInventorySlotData>& QuickSlots = PS->GetQuickSlotsRef();

	if (!QuickSlots.IsValidIndex(QuickSlotIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("UseQuickSlot Failed: Invalid QuickSlotIndex = %d"), QuickSlotIndex);
		return false;
	}

	const FInventorySlotData& SlotData = QuickSlots[QuickSlotIndex];

	if (SlotData.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("UseQuickSlot: Empty Slot Index = %d"), QuickSlotIndex);
		return false;
	}

	const FItemDataRow* ItemData = GetItemData(SlotData.ItemID);
	if (!ItemData)
	{
		UE_LOG(LogTemp, Warning, TEXT("UseQuickSlot Failed: Invalid ItemData for [%s]"), *SlotData.ItemID.ToString());
		return false;
	}

	// 캐릭터 가져오기
	APlayerController* PC = Cast<APlayerController>(PS->GetOwner());
	if (!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("UseQuickSlot Failed: PlayerController is nullptr."));
		return false;
	}

	AMainCharacterBase* MainCharacter = Cast<AMainCharacterBase>(PC->GetPawn());
	if (!MainCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("UseQuickSlot Failed: MainCharacter is nullptr."));
		return false;
	}

	// 1) 소모품이면 즉시 사용
	if (ItemData->ItemType == EItemType::Consumable)
	{
		UE_LOG(LogTemp, Warning, TEXT("UseQuickSlot: Consumable Use [%s]"), *SlotData.ItemID.ToString());
		return UseItem(EVLSlotType::QuickSlot, QuickSlotIndex);
	}

	// 2) 장비면 장비 클래스를 캐릭터에 넘겨 장착 처리
	if (ItemData->ItemType == EItemType::Equipment)
	{
		if (!ItemData->EquipmentClass)return false;
		return MainCharacter->HandleEquipmentItem(SlotData.ItemID, ItemData->EquipmentClass);
	}

	UE_LOG(LogTemp, Warning, TEXT("UseQuickSlot Failed: Unsupported ItemType [%s]"), *SlotData.ItemID.ToString());
	return false;
}

bool UVL_InventoryComponent::HasHeldItem() const
{
	return !HeldItem.IsEmpty();
}

const FInventorySlotData& UVL_InventoryComponent::GetHeldItem() const
{
	return HeldItem;
}

void UVL_InventoryComponent::ClearHeldItem()
{

	HeldItem.Clear();
	OnHeldItemChanged.Broadcast();
}

bool UVL_InventoryComponent::SetHeldItem(const FInventorySlotData& NewHeldItem)
{
	HeldItem = NewHeldItem;
	OnHeldItemChanged.Broadcast();
	return true;
}

void UVL_InventoryComponent::DebugPrintInventory() const
{
	AVL_PlayerState* PS = GetVLPlayerState();
	if (!PS)
	{
		UE_LOG(LogTemp, Warning, TEXT("DebugPrintInventory Failed: PlayerState is nullptr."));
		return;
	}

	const TArray<FInventorySlotData>& Inventory = PS->GetInventorySlotsRef();

	UE_LOG(LogTemp, Warning, TEXT("=== Inventory Debug Start ==="));

	for (int32 i = 0; i < Inventory.Num(); ++i)
	{
		const FInventorySlotData& Slot = Inventory[i];

		if (Slot.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("Slot[%d] : Empty"), i);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Slot[%d] : ItemID=%s, Quantity=%d"),
				i, *Slot.ItemID.ToString(), Slot.Quantity);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("=== Inventory Debug End ==="));
}


void UVL_InventoryComponent::DebugPrintQuickSlots() const
{
	AVL_PlayerState* PS = GetVLPlayerState();
	if (!PS)
	{
		UE_LOG(LogTemp, Warning, TEXT("DebugPrintQuickSlots Failed: PlayerState is nullptr."));
		return;
	}

	const TArray<FInventorySlotData>& QuickSlots = PS->GetQuickSlotsRef();

	UE_LOG(LogTemp, Warning, TEXT("=== QuickSlot Debug Start ==="));

	for (int32 i = 0; i < QuickSlots.Num(); ++i)
	{
		const FInventorySlotData& Slot = QuickSlots[i];

		if (Slot.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("QuickSlot[%d] : Empty"), i);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("QuickSlot[%d] : ItemID=%s, Quantity=%d"),
				i, *Slot.ItemID.ToString(), Slot.Quantity);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("=== QuickSlot Debug End ==="));
}

const FItemDataRow* UVL_InventoryComponent::GetItemData(FName ItemID) const
{
	if (ItemID.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("GetItemData Failed: ItemID is None."));
		return nullptr;
	}

	if (!ItemDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetItemData Failed: ItemDataTable is nullptr."));
		return nullptr;
	}

	const FItemDataRow* FoundRow = ItemDataTable->FindRow<FItemDataRow>(ItemID, TEXT("GetItemData"));

	if (!FoundRow)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetItemData Failed: No row found for ItemID [%s]."), *ItemID.ToString());
		return nullptr;
	}

	return FoundRow;
}

bool UVL_InventoryComponent::AddItem(FName ItemID, int32 Amount)
{
	//나중에 편집해야할것.
	AVL_PlayerState* PS = GetVLPlayerState();
	if (!PS)
	{
		UE_LOG(LogTemp, Warning, TEXT("AddItem Failed: PlayerState is nullptr."));
		return false;
	}

	if (!PS->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("AddItem Failed: Only server can modify inventory."));
		return false;
	}
	//
	if (ItemID.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("AddItem Failed: ItemID is None."));
		return false;
	}

	if (Amount <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("AddItem Failed: Amount must be greater than 0."));
		return false;
	}

	TArray<FInventorySlotData>& Inventory = PS->GetInventorySlotsRef();

	const FItemDataRow* ItemData = GetItemData(ItemID);
	if (!ItemData)
	{
		UE_LOG(LogTemp, Warning, TEXT("AddItem Failed: Invalid item data for [%s]."), *ItemID.ToString());
		return false;
	}

	if (ItemData->bCanStack)
	{
		int32 SameSlotIndex = FindSameItemSlotIndex(ItemID);

		if (SameSlotIndex != INDEX_NONE)
		{
			FInventorySlotData& Slot = Inventory[SameSlotIndex];

			if (Slot.Quantity < ItemData->MaxStackCount)
			{
				const int32 AddableAmount = ItemData->MaxStackCount - Slot.Quantity;
				const int32 ActualAddAmount = FMath::Min(AddableAmount, Amount);

				Slot.Quantity += ActualAddAmount;
				Amount -= ActualAddAmount;

				UE_LOG(LogTemp, Warning, TEXT("AddItem: Stacked [%s] x%d in slot %d."),
					*ItemID.ToString(), ActualAddAmount, SameSlotIndex);

				if (Amount <= 0)
				{
					BroadcastInventoryChanged();
					return true;
				}
			}
		}
	}

	while (Amount > 0)
	{
		const int32 EmptySlotIndex = FindEmptySlotIndex();
		if (EmptySlotIndex == INDEX_NONE)
		{
			UE_LOG(LogTemp, Warning, TEXT("AddItem Failed: Inventory is full. Remaining amount: %d"), Amount);
			return false;
		}

		FInventorySlotData& EmptySlot = Inventory[EmptySlotIndex];
		EmptySlot.ItemID = ItemID;

		if (ItemData->bCanStack)
		{
			const int32 ActualAddAmount = FMath::Min(ItemData->MaxStackCount, Amount);
			EmptySlot.Quantity = ActualAddAmount;
			Amount -= ActualAddAmount;
		}
		else
		{
			EmptySlot.Quantity = 1;
			Amount -= 1;
		}

		UE_LOG(LogTemp, Warning, TEXT("AddItem: Added [%s] x%d to empty slot %d."),
			*ItemID.ToString(), EmptySlot.Quantity, EmptySlotIndex);
	}

	BroadcastInventoryChanged();

	return true;
}

bool UVL_InventoryComponent::UseItem(EVLSlotType SlotType, int32 SlotIndex)
{
	AVL_PlayerState* PS = GetVLPlayerState();
	if (!PS)
	{
		UE_LOG(LogTemp, Warning, TEXT("UseItem Failed: PlayerState is nullptr."));
		return false;
	}

	FInventorySlotData SlotData = GetSlotData(SlotType, SlotIndex);
	if (SlotData.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("UseItem Failed: Slot is empty. Type=%d Index=%d"),
			(int32)SlotType, SlotIndex);
		return false;
	}

	const FItemDataRow* ItemData = GetItemData(SlotData.ItemID);
	if (!ItemData)
	{
		UE_LOG(LogTemp, Warning, TEXT("UseItem Failed: Invalid ItemData. ItemID=%s"),
			*SlotData.ItemID.ToString());
		return false;
	}

	if (ItemData->ConsumableType == EConsumableType::Ammo)
	{
		UE_LOG(LogTemp, Warning, TEXT("UseItem: Ammo cannot be used directly [%s]"), *SlotData.ItemID.ToString());
		return false;
	}

	switch (ItemData->ItemType)
	{
	case EItemType::Consumable:
		UE_LOG(LogTemp, Warning, TEXT("UseItem: Consumable Used [%s]"), *SlotData.ItemID.ToString());

		if (AVL_PlayerState* VLPS = GetVLPlayerState())
		{
			if (AVL_PlayerController* VLPC = Cast<AVL_PlayerController>(VLPS->GetOwner()))
			{
				VLPC->ClientPlayDrinkPotionSound(DrinkPotionSound);
			}
		}

		// 일단 1개 소비
		if (SlotData.Quantity > 1)
		{
			SlotData.Quantity -= 1;
			SetSlotData(SlotType, SlotIndex, SlotData);
		}
		else
		{
			ClearSlot(SlotType, SlotIndex);
		}
		return true;

	case EItemType::Equipment:
		UE_LOG(LogTemp, Warning, TEXT("UseItem: Equipment Equip Requested [%s]"), *SlotData.ItemID.ToString());

		// 지금은 로그만
		// 나중에 장착/해제 로직 연결
		return true;

	default:
		UE_LOG(LogTemp, Warning, TEXT("UseItem: Unsupported ItemType for [%s]"), *SlotData.ItemID.ToString());
		return false;
	}
}

bool UVL_InventoryComponent::MoveInventoryToInventory(int32 FromIndex, int32 ToIndex)
{
	AVL_PlayerState* PS = GetVLPlayerState();
	if (!PS)
	{
		return false;
	}

	TArray<FInventorySlotData>& Inventory = PS->GetInventorySlotsRef();

	if (!Inventory.IsValidIndex(FromIndex) || !Inventory.IsValidIndex(ToIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveInventoryToInventory Failed: Invalid index. From=%d, To=%d"), FromIndex, ToIndex);
		return false;
	}

	if (FromIndex == ToIndex)
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveInventoryToInventory Failed: FromIndex and ToIndex are same."));
		return false;
	}

	const bool bResult = MoveSlotData(Inventory[FromIndex], Inventory[ToIndex]);

	if (bResult)
	{
		BroadcastInventoryChanged();
		UE_LOG(LogTemp, Warning, TEXT("MoveInventoryToInventory Success: %d -> %d"), FromIndex, ToIndex);
	}

	return bResult;
}

bool UVL_InventoryComponent::MoveInventoryToQuickSlot(int32 InventoryIndex, int32 QuickSlotIndex)
{
	AVL_PlayerState* PS = GetVLPlayerState();
	if (!PS)
	{
		return false;
	}

	TArray<FInventorySlotData>& Inventory = PS->GetInventorySlotsRef();
	TArray<FInventorySlotData>& QuickSlots = PS->GetQuickSlotsRef();

	if (!Inventory.IsValidIndex(InventoryIndex) || !QuickSlots.IsValidIndex(QuickSlotIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveInventoryToQuickSlot Failed: Invalid index. Inventory=%d, QuickSlot=%d"), InventoryIndex, QuickSlotIndex);
		return false;
	}

	const bool bResult = MoveSlotData(Inventory[InventoryIndex], QuickSlots[QuickSlotIndex]);

	if (bResult)
	{
		BroadcastInventoryChanged();
		BroadcastQuickSlotChanged();
		UE_LOG(LogTemp, Warning, TEXT("MoveInventoryToQuickSlot Success: Inventory[%d] -> QuickSlot[%d]"), InventoryIndex, QuickSlotIndex);
	}

	return bResult;
}

bool UVL_InventoryComponent::MoveQuickSlotToInventory(int32 QuickSlotIndex, int32 InventoryIndex)
{
	AVL_PlayerState* PS = GetVLPlayerState();
	if (!PS)
	{
		return false;
	}

	TArray<FInventorySlotData>& QuickSlots = PS->GetQuickSlotsRef();
	TArray<FInventorySlotData>& Inventory = PS->GetInventorySlotsRef();

	if (!QuickSlots.IsValidIndex(QuickSlotIndex) || !Inventory.IsValidIndex(InventoryIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveQuickSlotToInventory Failed: Invalid index. QuickSlot=%d, Inventory=%d"), QuickSlotIndex, InventoryIndex);
		return false;
	}

	const bool bResult = MoveSlotData(QuickSlots[QuickSlotIndex], Inventory[InventoryIndex]);

	if (bResult)
	{
		BroadcastInventoryChanged();
		BroadcastQuickSlotChanged();
		UE_LOG(LogTemp, Warning, TEXT("MoveQuickSlotToInventory Success: QuickSlot[%d] -> Inventory[%d]"), QuickSlotIndex, InventoryIndex);
	}

	return bResult;
}

bool UVL_InventoryComponent::MoveQuickSlotToQuickSlot(int32 FromQuickSlotIndex, int32 ToQuickSlotIndex)
{
	AVL_PlayerState* PS = GetVLPlayerState();
	if (!PS)
	{
		return false;
	}

	TArray<FInventorySlotData>& QuickSlots = PS->GetQuickSlotsRef();

	if (!QuickSlots.IsValidIndex(FromQuickSlotIndex) || !QuickSlots.IsValidIndex(ToQuickSlotIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveQuickSlotToQuickSlot Failed: Invalid index. From=%d, To=%d"), FromQuickSlotIndex, ToQuickSlotIndex);
		return false;
	}

	if (FromQuickSlotIndex == ToQuickSlotIndex)
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveQuickSlotToQuickSlot Failed: From and To are same."));
		return false;
	}

	const bool bResult = MoveSlotData(QuickSlots[FromQuickSlotIndex], QuickSlots[ToQuickSlotIndex]);

	if (bResult)
	{
		BroadcastQuickSlotChanged();
		UE_LOG(LogTemp, Warning, TEXT("MoveQuickSlotToQuickSlot Success: %d -> %d"), FromQuickSlotIndex, ToQuickSlotIndex);
	}

	return bResult;
}

FInventorySlotData UVL_InventoryComponent::GetSlotData(EVLSlotType InSlotType, int32 InSlotIndex) const
{
	AVL_PlayerState* PS = GetVLPlayerState();
	if (!PS)
	{
		return FInventorySlotData();
	}

	const TArray<FInventorySlotData>* SlotArray = nullptr;

	switch (InSlotType)
	{
	case EVLSlotType::Inventory:
		SlotArray = &PS->GetInventorySlotsRef();
		break;

	case EVLSlotType::QuickSlot:
		SlotArray = &PS->GetQuickSlotsRef();
		break;

	default:
		return FInventorySlotData();
	}

	if (!SlotArray->IsValidIndex(InSlotIndex))
	{
		return FInventorySlotData();
	}

	return (*SlotArray)[InSlotIndex];
}

bool UVL_InventoryComponent::SetSlotData(EVLSlotType InSlotType, int32 InSlotIndex, const FInventorySlotData& NewData)
{
	//나중에 편집해야할것.
	AVL_PlayerState* PS = GetVLPlayerState();
	if (!PS)
	{
		UE_LOG(LogTemp, Warning, TEXT("SetSlotData Failed: PlayerState is nullptr."));
		return false;
	}

	if (!PS->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("SetSlotData Failed: Only server can modify inventory."));
		return false;
	}
	//

	TArray<FInventorySlotData>* SlotArray = GetSlotArray(InSlotType);

	if (!SlotArray || !SlotArray->IsValidIndex(InSlotIndex))
	{
		return false;
	}

	(*SlotArray)[InSlotIndex] = NewData;

	if (InSlotType == EVLSlotType::Inventory)
	{
		BroadcastInventoryChanged();
	}
	else if (InSlotType == EVLSlotType::QuickSlot)
	{
		BroadcastQuickSlotChanged();
	}

	return true;
}

bool UVL_InventoryComponent::ClearSlot(EVLSlotType InSlotType, int32 InSlotIndex)
{
	//나중에 편집해야할것.
	AVL_PlayerState* PS = GetVLPlayerState();
	if (!PS)
	{
		UE_LOG(LogTemp, Warning, TEXT("ClearSlot Failed: PlayerState is nullptr."));
		return false;
	}

	if (!PS->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("ClearSlot Failed: Only server can modify inventory."));
		return false;
	}
	//

	TArray<FInventorySlotData>* SlotArray = GetSlotArray(InSlotType);

	if (!SlotArray || !SlotArray->IsValidIndex(InSlotIndex))
	{
		return false;
	}

	(*SlotArray)[InSlotIndex].Clear();

	if (InSlotType == EVLSlotType::Inventory)
	{
		BroadcastInventoryChanged();
	}
	else if (InSlotType == EVLSlotType::QuickSlot)
	{
		BroadcastQuickSlotChanged();
	}

	return true;
}

bool UVL_InventoryComponent::MoveOrSwapSlot(EVLSlotType FromType, int32 FromIndex, EVLSlotType ToType, int32 ToIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("MoveOrSwapSlot Called"));
	//나중에 편집해야할것.
	AVL_PlayerState* PS = GetVLPlayerState();
	if (!PS)
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveOrSwapSlot Failed: PlayerState is nullptr."));
		return false;
	}

	if (!PS->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveOrSwapSlot Failed: Only server can modify inventory."));
		return false;
	}
	//
	TArray<FInventorySlotData>* FromArray = GetSlotArray(FromType);
	TArray<FInventorySlotData>* ToArray = GetSlotArray(ToType);

	if (!FromArray || !ToArray)
	{
		return false;
	}

	if (!FromArray->IsValidIndex(FromIndex) || !ToArray->IsValidIndex(ToIndex))
	{
		return false;
	}
	// 같은 슬롯이면 무시
	if (FromArray == ToArray && FromIndex == ToIndex)
	{
		return false;
	}

	FInventorySlotData& FromSlot = (*FromArray)[FromIndex];
	FInventorySlotData& ToSlot = (*ToArray)[ToIndex];

	// 도착칸 비어있으면 이동
	if (!MoveSlotData(FromSlot, ToSlot))
	{
		return false;
	}

	if (FromType == EVLSlotType::Inventory || ToType == EVLSlotType::Inventory)
	{
		BroadcastInventoryChanged();
	}
	if (FromType == EVLSlotType::QuickSlot || ToType == EVLSlotType::QuickSlot)
	{
		BroadcastQuickSlotChanged();
	}

	return true;
}

bool UVL_InventoryComponent::RemoveItem(FName ItemID, int32 Amount)
{
	//나중에 편집해야할것.
	AVL_PlayerState* PS = GetVLPlayerState();
	if (!PS)
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveItem Failed: PlayerState is nullptr."));
		return false;
	}

	if (!PS->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveItem Failed: Only server can modify inventory."));
		return false;
	}
	//

	if (ItemID.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveItem Failed: ItemID is None."));
		return false;
	}

	if (Amount <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveItem Failed: Amount must be greater than 0."));
		return false;
	}

	if (!HasEnoughItem(ItemID, Amount))
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveItem Failed: Not enough item [%s]. Requested: %d"), *ItemID.ToString(), Amount);
		return false;
	}

	TArray<FInventorySlotData>& Inventory = PS->GetInventorySlotsRef();

	int32 RemainingAmount = Amount;

	for (FInventorySlotData& Slot : Inventory)
	{
		if (Slot.IsEmpty() || Slot.ItemID != ItemID)
		{
			continue;
		}

		if (Slot.Quantity <= RemainingAmount)
		{
			RemainingAmount -= Slot.Quantity;
			Slot.Clear();
		}
		else
		{
			Slot.Quantity -= RemainingAmount;
			RemainingAmount = 0;
		}

		if (RemainingAmount <= 0)
		{
			BroadcastInventoryChanged();
			UE_LOG(LogTemp, Warning, TEXT("RemoveItem Success: Removed [%s] x%d"), *ItemID.ToString(), Amount);
			return true;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("RemoveItem Failed: Unexpected logic error for [%s]."), *ItemID.ToString());
	return false;
}

bool UVL_InventoryComponent::HasEnoughItem(FName ItemID, int32 Amount) const
{
	if (ItemID.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("HasEnoughItem Failed: ItemID is None."));
		return false;
	}

	if (Amount <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("HasEnoughItem Failed: Amount must be greater than 0."));
		return false;
	}

	AVL_PlayerState* PS = GetVLPlayerState();
	if (!PS)
	{
		UE_LOG(LogTemp, Warning, TEXT("HasEnoughItem Failed: PlayerState is nullptr."));
		return false;
	}

	const TArray<FInventorySlotData>& Inventory = PS->GetInventorySlotsRef();

	int32 TotalAmount = 0;

	for (const FInventorySlotData& Slot : Inventory)
	{
		if (!Slot.IsEmpty() && Slot.ItemID == ItemID)
		{
			TotalAmount += Slot.Quantity;

			if (TotalAmount >= Amount)
			{
				return true;
			}
		}
	}

	return false;
}

bool UVL_InventoryComponent::HasEnoughItemIncludingQuickSlots(FName ItemID, int32 Amount) const
{
	if (ItemID.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("HasEnoughItemIncludingQuickSlots Failed: ItemID is None."));
		return false;
	}

	if (Amount <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("HasEnoughItemIncludingQuickSlots Failed: Amount must be greater than 0."));
		return false;
	}

	AVL_PlayerState* PS = GetVLPlayerState();
	if (!PS)
	{
		UE_LOG(LogTemp, Warning, TEXT("HasEnoughItemIncludingQuickSlots Failed: PlayerState is nullptr."));
		return false;
	}

	int32 TotalAmount = 0;

	// 1) 퀵슬롯 먼저 확인
	const TArray<FInventorySlotData>& QuickSlots = PS->GetQuickSlotsRef();
	for (const FInventorySlotData& Slot : QuickSlots)
	{
		if (!Slot.IsEmpty() && Slot.ItemID == ItemID)
		{
			TotalAmount += Slot.Quantity;
			if (TotalAmount >= Amount)
			{
				return true;
			}
		}
	}

	// 2) 인벤토리 확인
	const TArray<FInventorySlotData>& Inventory = PS->GetInventorySlotsRef();
	for (const FInventorySlotData& Slot : Inventory)
	{
		if (!Slot.IsEmpty() && Slot.ItemID == ItemID)
		{
			TotalAmount += Slot.Quantity;
			if (TotalAmount >= Amount)
			{
				return true;
			}
		}
	}

	return false;
}

bool UVL_InventoryComponent::RemoveItemPreferredQuickSlots(FName ItemID, int32 Amount)
{
	AVL_PlayerState* PS = GetVLPlayerState();
	if (!PS)
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveItemPreferredQuickSlots Failed: PlayerState is nullptr."));
		return false;
	}

	if (!PS->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveItemPreferredQuickSlots Failed: Only server can modify inventory."));
		return false;
	}

	if (ItemID.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveItemPreferredQuickSlots Failed: ItemID is None."));
		return false;
	}

	if (Amount <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveItemPreferredQuickSlots Failed: Amount must be greater than 0."));
		return false;
	}

	if (!HasEnoughItemIncludingQuickSlots(ItemID, Amount))
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveItemPreferredQuickSlots Failed: Not enough item [%s]. Requested: %d"),
			*ItemID.ToString(), Amount);
		return false;
	}

	int32 RemainingAmount = Amount;

	// 1) 퀵슬롯 먼저, 낮은 인덱스부터
	TArray<FInventorySlotData>& QuickSlots = PS->GetQuickSlotsRef();
	for (FInventorySlotData& Slot : QuickSlots)
	{
		if (Slot.IsEmpty() || Slot.ItemID != ItemID)
		{
			continue;
		}

		if (Slot.Quantity <= RemainingAmount)
		{
			RemainingAmount -= Slot.Quantity;
			Slot.Clear();
		}
		else
		{
			Slot.Quantity -= RemainingAmount;
			RemainingAmount = 0;
		}

		if (RemainingAmount <= 0)
		{
			BroadcastQuickSlotChanged();
			BroadcastInventoryChanged();
			UE_LOG(LogTemp, Warning, TEXT("RemoveItemPreferredQuickSlots Success: Removed [%s] x%d"), *ItemID.ToString(), Amount);
			return true;
		}
	}

	// 2) 인벤토리 다음, 낮은 인덱스부터
	TArray<FInventorySlotData>& Inventory = PS->GetInventorySlotsRef();
	for (FInventorySlotData& Slot : Inventory)
	{
		if (Slot.IsEmpty() || Slot.ItemID != ItemID)
		{
			continue;
		}

		if (Slot.Quantity <= RemainingAmount)
		{
			RemainingAmount -= Slot.Quantity;
			Slot.Clear();
		}
		else
		{
			Slot.Quantity -= RemainingAmount;
			RemainingAmount = 0;
		}

		if (RemainingAmount <= 0)
		{
			BroadcastQuickSlotChanged();
			BroadcastInventoryChanged();
			UE_LOG(LogTemp, Warning, TEXT("RemoveItemPreferredQuickSlots Success: Removed [%s] x%d"), *ItemID.ToString(), Amount);
			return true;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("RemoveItemPreferredQuickSlots Failed: Unexpected logic error for [%s]."), *ItemID.ToString());
	return false;
}

//빈슬롯 찾기
int32 UVL_InventoryComponent::FindEmptySlotIndex() const
{
	AVL_PlayerState* PS = GetVLPlayerState();
	if (!PS)
	{
		return INDEX_NONE;
	}

	const TArray<FInventorySlotData>& Slots = PS->GetInventorySlotsRef();

	for (int32 i = 0; i < Slots.Num(); ++i)
	{
		if (Slots[i].IsEmpty())
		{
			return i;
		}
	}

	return INDEX_NONE;
}

//같은 아이템 슬롯 찾기
int32 UVL_InventoryComponent::FindSameItemSlotIndex(FName ItemID) const
{
	AVL_PlayerState* PS = GetVLPlayerState();
	if (!PS)
	{
		return INDEX_NONE;
	}

	const TArray<FInventorySlotData>& Slots = PS->GetInventorySlotsRef();

	for (int32 i = 0; i < Slots.Num(); ++i)
	{
		if (Slots[i].ItemID == ItemID)
		{
			return i;
		}
	}

	return INDEX_NONE;
}

bool UVL_InventoryComponent::MoveSlotData(FInventorySlotData& FromSlot, FInventorySlotData& ToSlot)
{
	if (FromSlot.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveSlotData Failed: FromSlot is empty."));
		return false;
	}

	// 목적지가 비어 있으면 그냥 이동
	if (ToSlot.IsEmpty())
	{
		Swap(FromSlot, ToSlot);
		UE_LOG(LogTemp, Warning, TEXT("MoveSlotData Success: Moved to empty slot."));
		return true;
	}

	// 같은 아이템이면 스택 합치기 시도
	if (FromSlot.ItemID == ToSlot.ItemID)
	{
		const FItemDataRow* ItemData = GetItemData(FromSlot.ItemID);
		if (!ItemData)
		{
			UE_LOG(LogTemp, Warning, TEXT("MoveSlotData Failed: Invalid item data for stack merge [%s]."),
				*FromSlot.ItemID.ToString());
			return false;
		}

		if (ItemData->bCanStack)
		{
			if (ToSlot.Quantity >= ItemData->MaxStackCount)
			{
				UE_LOG(LogTemp, Warning, TEXT("MoveSlotData: Target slot already full."));
				return false;
			}

			const int32 EmptySpace = ItemData->MaxStackCount - ToSlot.Quantity;
			const int32 MoveAmount = FMath::Min(FromSlot.Quantity, EmptySpace);

			ToSlot.Quantity += MoveAmount;
			FromSlot.Quantity -= MoveAmount;

			UE_LOG(LogTemp, Warning, TEXT("MoveSlotData Success: Stacked %d of [%s]."),
				MoveAmount, *FromSlot.ItemID.ToString());

			if (FromSlot.Quantity <= 0)
			{
				FromSlot.Clear();
			}

			return true;
		}
	}

	// 다른 아이템이거나 스택 불가능이면 스왑
	Swap(FromSlot, ToSlot);

	UE_LOG(LogTemp, Warning, TEXT("MoveSlotData Success: Swapped slots."));
	return true;
}

TArray<FInventorySlotData>* UVL_InventoryComponent::GetSlotArray(EVLSlotType SlotType)
{
	AVL_PlayerState* PS = GetVLPlayerState();
	if (!PS)
	{
		return nullptr;
	}

	switch (SlotType)
	{
	case EVLSlotType::Inventory:
		return &PS->GetInventorySlotsRef();

	case EVLSlotType::QuickSlot:
		return &PS->GetQuickSlotsRef();

	default:
		return nullptr;
	}
}
