// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/UI/VL_UMGBase.h"
#include "VL_QuickSlotOne.generated.h"

/**
 *
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuickSlotClicked, int32, SlotIndex);

class UImage;
class UTextBlock;
class UButton;
class UTexture2D;


UCLASS()
class VIALACTEA_API UVL_QuickSlotOne : public UVL_UMGBase
{
	GENERATED_BODY()

public:

	//화면에 처음 나타날시 슬롯 비워두기.
	virtual void NativeConstruct() override;

protected:

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UImage* ItemIcon; // 아이템 아이콘 이미지

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* ItemCountText; // 아이템 개수 텍스트

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* SlotNumberText; // 슬롯 번호

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* SelectionVisual; // 선택되었을 때 나타나는 하이라이트 이미지

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UButton* SlotButton; //슬롯 버튼

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

protected:

	//버튼 클릭 시 실행될 LOG.
	//사용자에게도 알림 보내기 가능.
	UFUNCTION()
	void OnSlotClicked();

	UPROPERTY(BlueprintReadOnly, Category = "QuickSlot")
	int32 SlotIndex = -1;

public:
	// 슬롯의 번호를 설정
	UFUNCTION(BlueprintCallable, Category = "QuickSlot")
	void SetSlotIndex(int32 InSlotIndex);

	//슬롯아이콘이 보여지는것을 변경, 몇번 슬롯인지 보여짐
	UFUNCTION(BlueprintCallable, Category = "QuickSlot")
	void UpdateSlotDisplay(UTexture2D* NewIcon, int32 NewCount);

	//슬롯선택시 하이라이트 표시
	UFUNCTION(BlueprintCallable, Category = "QuickSlot")
	void SetHighlight(bool bIsSelected);

	//슬롯 빈칸일때
	UFUNCTION(BlueprintCallable, Category = "QuickSlot")
	void ClearSlot();

	//부모가 바인딩해서 받는 이벤트 
	UPROPERTY(BlueprintAssignable, Category = "QuickSlot")
	FOnQuickSlotClicked OnQuickSlotClicked;

	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

};
