// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/UI/VL_UMGBase.h"
#include "Base/Data/VL_CommonStructs.h"
#include "VL_HeldItemWidget.generated.h"

/**
 * 
 */

class UImage;
class UTextBlock;
class UVL_InventoryComponent;

UCLASS()
class VIALACTEA_API UVL_HeldItemWidget : public UVL_UMGBase
{
	GENERATED_BODY()
	
public:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "HeldItem")
    void SetInventoryComponent(UVL_InventoryComponent* NewInventoryComponent);

    UFUNCTION()
    void RefreshHeldItem();

protected:
    void UpdatePositionToMouse();

protected:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> ItemIcon = nullptr;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> QuantityText = nullptr;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "HeldItem")
    TObjectPtr<UVL_InventoryComponent> InventoryComponent = nullptr;

};
