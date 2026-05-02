// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/Data/VL_GameDataBase.h"
#include "VL_ItemDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class VIALACTEA_API UVL_ItemDataAsset : public UVL_GameDataBase
{
	GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, Category = "Item|Visual")
    TSoftObjectPtr<UTexture2D> ItemIcon; // 인벤토리용 아이콘

    UPROPERTY(EditDefaultsOnly, Category = "Item|Visual")
    TSoftObjectPtr<UStaticMesh> MeshOnGround; // 바닥에 떨어졌을 때 메쉬

    UPROPERTY(EditDefaultsOnly, Category = "Item|Stat")
    int32 MaxStack = 1; // 중첩 가능 수  무기는 1일거고, 포션 같은 아이템은 여러 개 일 것임

    UPROPERTY(EditDefaultsOnly, Category = "Item|Stat")
    float Weight = 0.1f; // 무게가 있다면 넣기

    // 아이템 설명 (UI 툴팁용)
    UPROPERTY(EditDefaultsOnly, Category = "Item|Description")
    FText ItemDescription;

    UPROPERTY(EditDefaultsOnly, Category = "Item|UI")
    TSubclassOf<UUserWidget> InteractionWidgetClass; // 띄울 UI 클래스

    virtual FPrimaryAssetId GetPrimaryAssetId() const override
    {
        return FPrimaryAssetId(FPrimaryAssetType("ItemData"), DataID);
    }
};
