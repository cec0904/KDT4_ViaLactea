#pragma once

#include "CoreMinimal.h"
#include "VL_PlayerSaveData.generated.h"

USTRUCT(BlueprintType)
struct FVLPlayerSaveData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FString UserId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	int32 CharacterSlotId = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FString CharacterName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FTransform SavedTransform;
};