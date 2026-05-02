#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h" // 기본 타입을 위해 포함
#include "GimmickTypes.generated.h"

UENUM(BlueprintType)
enum class EGimmickState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Active      UMETA(DisplayName = "Active"),
    Destroyed   UMETA(DisplayName = "Destroyed"),
    Cooldown    UMETA(DisplayName = "Cooldown")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGimmickStateChanged, EGimmickState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGimmickDestroyed, AActor*, DestroyedGimmick);
