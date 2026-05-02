// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Struct/VL_PlayerSaveData.h"
#include "VL_CharacterSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class VIALACTEA_API UVL_CharacterSaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Save")
	FVLPlayerSaveData PlayerData;
};
