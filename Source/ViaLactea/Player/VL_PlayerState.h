// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/FrameWork/VL_PlayerStateBase.h"
#include "Base/Data/VL_CommonStructs.h"

#include "../Base/SaveGame/Struct/VL_PlayerSaveData.h"

#include "VL_PlayerState.generated.h"


/**
 *
 */

class UVL_InventoryComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnQuickSlotChanged);

UCLASS()
class VIALACTEA_API AVL_PlayerState : public AVL_PlayerStateBase
{
    GENERATED_BODY()

public:
    AVL_PlayerState();

public:
    //getter
    FORCEINLINE UVL_InventoryComponent* GetInventoryComponent() const { return InventoryComponent; }

    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnInventoryChanged OnInventoryChanged;

    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnQuickSlotChanged OnQuickSlotChanged;

public:
    // Save
    // 플레이어 1명당 붙는 상태 보관
    UFUNCTION(BlueprintCallable, Category = "Save")
    void SetPlayerIdentity(const FString& InUserId, int32 InCharacterSlotId, const FString& InCharacterName);

    UFUNCTION(BlueprintCallable, Category = "Save")
    FVLPlayerSaveData BuildPlayerSaveData() const;

    UFUNCTION(BlueprintCallable, Category = "Save")
    void ApplyPlayerSaveData(const FVLPlayerSaveData& InSaveData);

    UFUNCTION(BlueprintCallable, Category = "Save")
    FString GetUserId() const { return PlayerSaveData.UserId; }

    UFUNCTION(BlueprintCallable, Category = "Save")
    int32 GetCharacterSlotId() const { return PlayerSaveData.CharacterSlotId; }

    UFUNCTION(BlueprintCallable, Category = "Save")
    FString GetCharacterName() const { return PlayerSaveData.CharacterName; }


protected:
    virtual void BeginPlay() override;

    //인벤토리 부착
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    TObjectPtr<UVL_InventoryComponent> InventoryComponent = nullptr;

    // 시연용 초기 아이템 지급 사용 여부
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Demo")
    bool bUseDemoInventory = true;

    // 시연용 초기 인벤토리 세팅
    void InitDemoInventory();

protected:
    // VL_PlayerSaveData.h 구조체에서 변수 받아오기
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Save")
    FVLPlayerSaveData PlayerSaveData;


public:

    UPROPERTY(ReplicatedUsing = OnRep_InventorySlots, VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    TArray<FInventorySlotData> InventorySlots;

    UPROPERTY(ReplicatedUsing = OnRep_QuickSlots, VisibleAnywhere, BlueprintReadOnly, Category = "QuickSlot")
    TArray<FInventorySlotData> QuickSlots;

    //인벤토리슬롯최대치
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 MaxSlotCount = 42;

    //퀵슬롯 최대치
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QuickSlot")
    int32 MaxQuickSlotCount = 9;
    //클라인벤토리
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    UFUNCTION()
    void OnRep_InventorySlots();

    UFUNCTION()
    void OnRep_QuickSlots();
    //클라인벤토리~
public:
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void InitializeInventory();

    UFUNCTION(BlueprintCallable, Category = "QuickSlot")
    void InitializeQuickSlots();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    const TArray<FInventorySlotData>& GetInventorySlots() const;

    UFUNCTION(BlueprintCallable, Category = "QuickSlot")
    const TArray<FInventorySlotData>& GetQuickSlots() const;

    //기존이 읽기전용 이므로 복사안하고 원본 배열 직접 수정하기 위해 원본 배열 참조 형식 생성.
    // UI에서 읽을땐 위에있는걸로, 로직에서는 Ref를 사용하면 된다.
    FORCEINLINE TArray<FInventorySlotData>& GetInventorySlotsRef() { return InventorySlots; }
    FORCEINLINE const TArray<FInventorySlotData>& GetInventorySlotsRef() const { return InventorySlots; }

    FORCEINLINE TArray<FInventorySlotData>& GetQuickSlotsRef() { return QuickSlots; }
    FORCEINLINE const TArray<FInventorySlotData>& GetQuickSlotsRef() const { return QuickSlots; }

};
