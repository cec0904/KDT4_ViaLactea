// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ViaLactea_BFLibrary.generated.h"

/**
 * 
 */
UCLASS()
class VIALACTEA_API UViaLactea_BFLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	// JoinSession 성공 이후 호출: 세션의 ConnectString을 얻고 ClientTravel까지 수행
	UFUNCTION(BlueprintCallable, Category = "Online|Session", meta = (WorldContext = "WorldContextObject"))
	static bool ClientTravelToGameSession(UObject* WorldContextObject, APlayerController* PlayerController, bool bAbsolute = true);
};
