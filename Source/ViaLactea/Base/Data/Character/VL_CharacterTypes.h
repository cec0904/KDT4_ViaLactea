#pragma once

#include "CoreMinimal.h"
#include "VL_CharacterTypes.generated.h"

// 캐릭터/몬스터 등급
UENUM(BlueprintType)
enum class EVLCharacterRank : uint8
{
    Common      UMETA(DisplayName = "Common"),
    Elite       UMETA(DisplayName = "Elite"),
    Normal      UMETA(DisplayName = "Normal"),
    Boss        UMETA(DisplayName = "Boss"),
    Player      UMETA(DisplayName = "Player")
};

// 스킬 속성 시스템
UENUM(BlueprintType)
enum class EVLElementalType : uint8
{
    None        UMETA(DisplayName = "None"),
    Fire        UMETA(DisplayName = "Fire"),
    Water       UMETA(DisplayName = "Water"),
    Ice         UMETA(DisplayName = "Ice"),
    Electric    UMETA(DisplayName = "Electric")
};

// 핵심 스탯 구조체 (공용)
USTRUCT(BlueprintType)
struct FVLCharacterStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxHP = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AttackPower = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Defense = 5.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxMoveSpeed = 600.f;
};