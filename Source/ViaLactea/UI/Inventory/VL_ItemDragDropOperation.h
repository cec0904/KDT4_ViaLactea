#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "Base/Data/VL_CommonStructs.h"
#include "Base/Data/EVLSlotType.h"
#include "VL_ItemDragDropOperation.generated.h"

UCLASS()
class VIALACTEA_API UVL_ItemDragDropOperation : public UDragDropOperation
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite)
    FInventorySlotData DraggedItem;

    UPROPERTY(BlueprintReadWrite)
    EVLSlotType FromSlotType = EVLSlotType::None;

    UPROPERTY(BlueprintReadWrite)
    int32 FromSlotIndex = INDEX_NONE;
};