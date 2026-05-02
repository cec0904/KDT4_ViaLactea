// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/Data/Character/VL_CharacterDataAsset.h"
#include "VL_PlayerDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class VIALACTEA_API UVL_PlayerDataAsset : public UVL_CharacterDataAsset
{
    GENERATED_BODY()

//public:
//    // 플레이어 전용: 직업군 분류
//    //UPROPERTY(EditAnywhere, Category = "Player|Identity")
//    //EPlayerJobType JobType;
//
//    // 플레이어 전용: 레벨업 시 스탯 상승 곡선 (데이터 테이블 참조)
//    UPROPERTY(EditAnywhere, Category = "Player|Growth")
//    TSoftObjectPtr<UDataTable> StatGrowthTable;
//
//    // 플레이어 전용: 기본 인벤토리 시작 아이템 목록
//    UPROPERTY(EditAnywhere, Category = "Player|Inventory")
//    TArray<FPrimaryAssetId> StartingItems;
//
//    virtual FPrimaryAssetId GetPrimaryAssetId() const override
//    {
//        return FPrimaryAssetId(FPrimaryAssetType("PlayerData"), DataID);
//    }
};
