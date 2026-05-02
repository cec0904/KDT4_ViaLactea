// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/UI/VL_UMGBase.h"
#include "Base/Data/VL_CommonStructs.h"
#include "Base/Data/EVLSlotType.h"
#include "VL_InventorySlotWidget.generated.h"

/**
 *
 */

class UImage;
class UTextBlock;
class UVL_InventoryComponent;
class UDragDropOperation;
class UDataTable;
class USoundBase;

UCLASS()
class VIALACTEA_API UVL_InventorySlotWidget : public UVL_UMGBase
{
    GENERATED_BODY()

public:
    //해당 슬롯 타입
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot")
    EVLSlotType SlotType = EVLSlotType::Inventory;
    //슬롯 칸
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot")
    int32 SlotIndex = INDEX_NONE;
    //해당슬롯에 들어있는 아이템 데이터 (아이템이름, 수량)
    UPROPERTY(BlueprintReadOnly, Category = "Slot")
    FInventorySlotData SlotData;

    UPROPERTY(BlueprintReadWrite, Category = "Slot")
    TObjectPtr<UVL_InventoryComponent> InventoryComponent;

    bool bIsBeingDragged = false;
public:
    //슬롯 초기화, UI생성시 호출됨.
    UFUNCTION(BlueprintCallable)
    void InitSlot(UVL_InventoryComponent* InInventoryComponent, EVLSlotType InSlotType, int32 InSlotIndex);
    //슬롯 UI 업데이트, 아이콘과 수량 변경
    UFUNCTION(BlueprintCallable)
    void SetSlotData(const FInventorySlotData& InData);


protected:
    //마우스 버튼 드래그 감지
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    //실제 드래그 생성
    virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
    //드래그한거 드롭 처리
    virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

    virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

protected:
    //아이템 이미지 아이콘
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> IMG_ItemIcon;
    //아이템 수량
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> TXT_Quantity;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    TObjectPtr<UDataTable> ItemDataTable;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|UI")
    TObjectPtr<USoundBase> PickItemSound;


};
