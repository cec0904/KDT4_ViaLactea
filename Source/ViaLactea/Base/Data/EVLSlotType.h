#pragma once

#include "CoreMinimal.h"
#include "EVLSlotType.generated.h"

//Drag&Drop할때 슬롯타입정보를 가져오기위한 코드
UENUM(BlueprintType)
enum class EVLSlotType : uint8
{
    None,
    Inventory,
    QuickSlot,

    MAX
};