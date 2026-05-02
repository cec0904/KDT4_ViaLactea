#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "VL_CommonEnums.h"
#include "Weapon/CharacterEquipmentBase.h"
#include "VL_DataRows.generated.h"

class UTexture2D;
class AActor;

USTRUCT(BlueprintType)
struct FItemDataRow : public FTableRowBase
{
	GENERATED_BODY()

public:

	// 내부 식별용 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FText ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FText ItemDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	EItemType ItemType = EItemType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	EConsumableType ConsumableType = EConsumableType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	EEquipmentType EquipmentType = EEquipmentType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	EVLEquipPolicy EquipPolicy = EVLEquipPolicy::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TSubclassOf<ACharacterEquipmentBase> EquipmentClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TObjectPtr<UTexture2D> Icon = nullptr;

	// 겹치기 가능 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	bool bCanStack = false;

	// 최대 중첩 수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", meta = (ClampMin = "1"))
	int32 MaxStackCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	bool bCanDrop = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 GoldValue = 0;

	// 월드에 드랍되었을 때 사용할 액터 클래스
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TSubclassOf<AActor> WorldActorClass = nullptr;
};
