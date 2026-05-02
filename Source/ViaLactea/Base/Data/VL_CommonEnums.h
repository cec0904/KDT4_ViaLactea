#pragma once

#include "CoreMinimal.h"
#include "VL_CommonEnums.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
	None		UMETA(DisplayName = "None"),
	Material	UMETA(DisplayName = "Material"),
	Consumable	UMETA(DisplayName = "Consumable"),
	Equipment	UMETA(DisplayName = "Equipment"),
	Quest		UMETA(DisplayName = "Quest"),
	Etc			UMETA(DisplayName = "Etc")
};

UENUM(BlueprintType)
enum class EConsumableType : uint8
{
	None		UMETA(DisplayName = "None"),
	HealHP		UMETA(DisplayName = "Heal HP"),
	RestoreMP	UMETA(DisplayName = "Restore MP"),
	RestoreStamina UMETA(DisplayName = "Restore Stamina"),
	Buff		UMETA(DisplayName = "Buff"),
	Ammo		UMETA(DisplayName = "Ammo")
};

UENUM(BlueprintType)
enum class EEquipmentType : uint8
{
	None		UMETA(DisplayName = "None"),
	Weapon		UMETA(DisplayName = "Weapon"),
	Shield		UMETA(DisplayName = "Shield"),
	Helmet		UMETA(DisplayName = "Helmet"),
	Armor		UMETA(DisplayName = "Armor"),
	Gloves		UMETA(DisplayName = "Gloves"),
	Boots		UMETA(DisplayName = "Boots"),
	Accessory	UMETA(DisplayName = "Accessory")
};

UENUM(BlueprintType)
enum class EVLEquipPolicy : uint8
{
	None            UMETA(DisplayName = "None"),
	OneHandWeapon   UMETA(DisplayName = "One Hand Weapon"),
	OffHand         UMETA(DisplayName = "Off Hand"),
	TwoHandWeapon   UMETA(DisplayName = "Two Hand Weapon")
};