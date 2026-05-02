#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "VL_CooldownProvider.generated.h"


UINTERFACE(MinimalAPI)
class UVL_CooldownProvider : public UInterface
{
	GENERATED_BODY()
};


class VIALACTEA_API IVL_CooldownProvider
{
	GENERATED_BODY()

public:
	virtual float GetCooldownValue(FGameplayTag Tag) const = 0;
};
