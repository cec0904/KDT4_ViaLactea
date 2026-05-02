// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Struct/VL_CharacterSlotInfo.h"
#include "VL_AccountSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class VIALACTEA_API UVL_AccountSaveGame : public USaveGame
{
	GENERATED_BODY()
	

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Account")
	FString UserId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Account")
	TArray<FVLCharacterSlotInfo> CharacterSlots;
};
