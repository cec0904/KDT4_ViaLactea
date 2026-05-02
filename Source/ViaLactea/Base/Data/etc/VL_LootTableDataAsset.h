// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/Data/VL_GameDataBase.h"
#include "VL_LootTableDataAsset.generated.h"

class UVL_ItemDataAsset;

// 일단 아이템데이터 에셋을 참조하도록 만들어 놓음 테이블로 교체 가능

USTRUCT(BlueprintType)
struct FLootResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    TSoftObjectPtr<UVL_ItemDataAsset> ItemData;

    UPROPERTY(BlueprintReadOnly)
    int32 Quantity =0;
};

USTRUCT(BlueprintType)
struct FLootEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
    TSoftObjectPtr<UVL_ItemDataAsset> ItemData;

    /** 드롭 확률 (0.0 ~ 1.0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DropChance = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot", meta = (ClampMin = "1"))
    int32 MinCount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot", meta = (ClampMin = "1"))
    int32 MaxCount = 1;
};

UCLASS()
class VIALACTEA_API UVL_LootTableDataAsset : public UVL_GameDataBase
{
	GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
    TArray<FLootEntry> LootEntries;
	
    UFUNCTION(BlueprintCallable, Category = "Loot")
    TArray<FLootResult> RollLoot() const;


    virtual FPrimaryAssetId GetPrimaryAssetId() const override
    {
        return FPrimaryAssetId(FPrimaryAssetType("LootTable"), DataID);
    }
};
