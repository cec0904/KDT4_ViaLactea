// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ViaLacteaGameMode.generated.h"

/**
 *  Simple GameMode for a third person game
 */
UCLASS(abstract)
class AViaLacteaGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	
	/** Constructor */
	AViaLacteaGameMode();
};



