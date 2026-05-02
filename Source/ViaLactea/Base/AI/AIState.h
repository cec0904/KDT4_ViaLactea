#pragma once

#include "CoreMinimal.h"
#include "AIState.generated.h"

class AAIController;
class UBlackboardComponent;
class AVL_AICharacterBase;
class AVL_Boss1;
class UVL_MonsterDataAsset;

UENUM(BlueprintType)
enum class EAIState : uint8
{
    Idle            UMETA(DisplayName = "Idle"), // 0
    Patrol          UMETA(DisplayName = "Patrol"), // 1
    IsReturning     UMETA(DisplayName = "IsReturning"), // 2
    Alert           UMETA(DisplayName = "Alert"), // 3
    Combat          UMETA(DisplayName = "Combat"), // 4
    Chase           UMETA(DisplayName = "Chase"), // 5
    Groggy          UMETA(DisplayName = "Groggy"),
    Dead            UMETA(DisplayName = "Dead"),
    Returnto        UMETA(DisplayName = "Returnto"),
    PhaseChanging   UMETA(DisplayName = "PhaseChanging")
};

USTRUCT(BlueprintType)
struct FBTS_CommonMemory
{
    GENERATED_BODY()

    TWeakObjectPtr<AAIController> CachedAIC;
    TWeakObjectPtr<UBlackboardComponent> CachedBB;
    TWeakObjectPtr<AVL_AICharacterBase> CachedPawn;
    TWeakObjectPtr<AVL_Boss1> CachedBoss;
    TWeakObjectPtr<UVL_MonsterDataAsset> CachedMonsterData;
};