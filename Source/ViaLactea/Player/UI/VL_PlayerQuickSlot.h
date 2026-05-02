// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/UI/VL_UMGBase.h"

#include "VL_PlayerQuickSlot.generated.h"


/**
 *
 */

class UVL_QuickSlotOne;
//UI 가로상자
class UHorizontalBox;
class UVL_InventoryComponent;

UCLASS()
class VIALACTEA_API UVL_PlayerQuickSlot : public UVL_UMGBase
{
	GENERATED_BODY()

public:

	virtual void NativeConstruct() override;

protected:

	//가로박스
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UHorizontalBox* HB_Slots;

	// 생성할 한 칸 슬롯 위젯 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "QuickSlot")
	TSubclassOf<UVL_QuickSlotOne> QuickSlotOneClass;

public:

	UPROPERTY(BlueprintReadWrite, Category = "QuickSlot")
	TObjectPtr<UVL_InventoryComponent> InventoryComponent;
	//9개의 슬롯을 담을 배열
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "QuickSlot")
	TArray<UVL_QuickSlotOne*> QuickSlotArray;

	// 현재 선택된 슬롯 인덱스 9개
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "QuickSlot")
	int32 CurrentSelectedSlot = 0;

	// 슬롯이 클릭되었을 때
	UFUNCTION()
	void OnSlotClicked(int32 SlotIndex);

	//퀵슬롯바 선택변경함수
	UFUNCTION(BlueprintCallable, Category = "QuickSlot")
	void SelectSlot(int32 NewIndex);

	UFUNCTION(BlueprintCallable, Category = "QuickSlot")
	void SetInventoryComponent(UVL_InventoryComponent* InInventoryComponent);

	UFUNCTION()
	void RefreshQuickSlot();

	UFUNCTION()
	void HandleQuickSlotChanged();

private:
	void UpdateSelectionVisual();

};
