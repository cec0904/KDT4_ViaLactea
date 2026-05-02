// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Struct/VL_WorldSlotInfo.h"
#include "VL_WorldSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class VIALACTEA_API UVL_WorldSaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FString OwnerUserId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	int32 OwnerCharacterSlotId = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	TArray<FVLWorldSlotInfo> WorldSlots;
};
