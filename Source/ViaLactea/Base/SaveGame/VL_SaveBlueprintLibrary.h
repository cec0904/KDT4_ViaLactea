// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VL_SaveBlueprintLibrary.generated.h"

/**
 * 
 */
UCLASS()
class VIALACTEA_API UVL_SaveBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Save", meta = (WorldContext = "WorldContextObject"))
	static bool SaveCurrentPlayer(UObject* WorldContextObject, APlayerController* PlayerController);

	UFUNCTION(BlueprintCallable, Category = "Save", meta = (WorldContext = "WorldContextObject"))
	static bool LoadSelectedCharacter(UObject* WorldContextObject, APlayerController* PlayerController, const FString& UserId, int32 CharacterSlotId);

	UFUNCTION(BlueprintCallable, Category = "Save", meta = (WorldContext = "WorldContextObject"))
	static bool SetCurrentPlayerIdentity(UObject* WorldContextObject, APlayerController* PlayerController, const FString& UserId, int32 CharacterSlotId, const FString& CharacterName);


	
};
