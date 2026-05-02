#pragma once

#include "CoreMinimal.h"
#include "VL_CommonStructs.generated.h"

USTRUCT(BlueprintType)
struct FInventorySlotData
{
	GENERATED_BODY()

public:
	// DataTable RowName과 연결될 ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	FName ItemID = NAME_None;

	// 해당 슬롯에 들어있는 수량
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 Quantity = 0;

	bool IsEmpty() const
	{
		return ItemID.IsNone() || Quantity <= 0;
	}

	void Clear()
	{
		ItemID = NAME_None;
		Quantity = 0;
	}
};