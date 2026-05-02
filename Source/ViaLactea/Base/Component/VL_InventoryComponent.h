// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Base/Data/VL_CommonStructs.h"
#include "Base/Data/EVLSlotType.h"
#include "VL_InventoryComponent.generated.h"

class UDataTable;
struct FItemDataRow;
class AVL_PlayerState;
class USoundBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHeldItemChanged);


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class VIALACTEA_API UVL_InventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UVL_InventoryComponent();

	UFUNCTION(BlueprintCallable)
	void BroadcastInventoryChanged();

	UFUNCTION(BlueprintCallable)
	void BroadcastQuickSlotChanged();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	UPROPERTY(BlueprintAssignable, Category = "Inventory|HeldItem")
	FOnHeldItemChanged OnHeldItemChanged;

private:
	UPROPERTY()
	FInventorySlotData HeldItem;

	UPROPERTY()
	TObjectPtr<AVL_PlayerState> CachedPlayerState = nullptr;

protected:
	AVL_PlayerState* GetVLPlayerState() const;

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory|HeldItem")
	bool HasHeldItem() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|HeldItem")
	const FInventorySlotData& GetHeldItem() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|HeldItem")
	void ClearHeldItem();

	UFUNCTION(BlueprintCallable, Category = "Inventory|HeldItem")
	bool SetHeldItem(const FInventorySlotData& NewHeldItem);

public:
	//인벤토리 디버깅용
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void DebugPrintInventory() const;

	UFUNCTION(BlueprintCallable, Category = "QuickSlot")
	void DebugPrintQuickSlots() const;

public:

	//아이템 데이터 연결
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TObjectPtr<UDataTable> ItemDataTable = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|Item")
	TObjectPtr<USoundBase> DrinkPotionSound;

public:

	// 아이템 데이터 찾기
	const FItemDataRow* GetItemData(FName ItemID) const;

	// 아이템 획득
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddItem(FName ItemID, int32 Amount);
	//아이템 선택
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool UseItem(EVLSlotType SlotType, int32 SlotIndex);

	const TArray<FInventorySlotData>& GetQuickSlots() const;

	UFUNCTION(BlueprintCallable, Category = "QuickSlot")
	bool UseQuickSlot(int32 QuickSlotIndex);

public:
	//인벤토리<->퀵슬롯 이동 함수
	//인벤토리->인벤토리
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool MoveInventoryToInventory(int32 FromIndex, int32 ToIndex);
	//인벤토리->퀵슬롯
	UFUNCTION(BlueprintCallable, Category = "QuickSlot")
	bool MoveInventoryToQuickSlot(int32 InventoryIndex, int32 QuickSlotIndex);
	//퀵슬롯->인벤토리
	UFUNCTION(BlueprintCallable, Category = "QuickSlot")
	bool MoveQuickSlotToInventory(int32 QuickSlotIndex, int32 InventoryIndex);
	//퀵슬롯->퀵슬롯
	UFUNCTION(BlueprintCallable, Category = "QuickSlot")
	bool MoveQuickSlotToQuickSlot(int32 FromQuickSlotIndex, int32 ToQuickSlotIndex);

public:
	//슬롯의 아이템 정보 가져오기
	UFUNCTION(BlueprintCallable)
	FInventorySlotData GetSlotData(EVLSlotType InSlotType, int32 InSlotIndex) const;
	//슬롯에 아이템 넣기
	UFUNCTION(BlueprintCallable)
	bool SetSlotData(EVLSlotType InSlotType, int32 InSlotIndex, const FInventorySlotData& NewData);
	//슬롯 비우기
	UFUNCTION(BlueprintCallable)
	bool ClearSlot(EVLSlotType InSlotType, int32 InSlotIndex);
	//아이템 이동or교환
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool MoveOrSwapSlot(EVLSlotType FromType, int32 FromIndex, EVLSlotType ToType, int32 ToIndex);

public:
	//아이템삭제
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItem(FName ItemID, int32 Amount);
	//아이템이 충분한지.
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool HasEnoughItem(FName ItemID, int32 Amount) const;

	//화살 퀵슬롯 확인 순서
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool HasEnoughItemIncludingQuickSlots(FName ItemID, int32 Amount) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItemPreferredQuickSlots(FName ItemID, int32 Amount);

protected:
	int32 FindEmptySlotIndex() const;

	int32 FindSameItemSlotIndex(FName ItemID) const;
	//이동 슬롯 데이터
	bool MoveSlotData(FInventorySlotData& FromSlot, FInventorySlotData& ToSlot);

private:
	//내부에서 슬롯 배열 찾는 helper 함수
	TArray<FInventorySlotData>* GetSlotArray(EVLSlotType SlotType);

};
