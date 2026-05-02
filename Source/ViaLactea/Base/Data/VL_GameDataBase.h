// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h" // FGameplayTag를 쓰려면 반드시 포함
#include "VL_GameDataBase.generated.h"


UCLASS()
class VIALACTEA_API UVL_GameDataBase : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
    // 에디터에서 수정 가능하고 블루프린트에서 읽을 수 있게 세팅
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BaseInfo")
    FName DataID;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BaseInfo")
    FText DataName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BaseInfo")
    FGameplayTag ObjectTypeTag;
    // 에디터 프로젝트 세팅 -> 게임플레이 태그 -> 새 게임플레이 태그 소스 추가 에서 추가로 만들어야됨

    // Asset Manager가 식별할 수 있도록 ID 반환 (PrimaryDataAsset의 특성 이용하기)
    virtual FPrimaryAssetId GetPrimaryAssetId() const override
    {
        /*

        Project Settings > Asset Manager로 이동합니다.

        Primary Asset Types to Scan에 요소를 추가합니다.

        Primary Asset Type: 코드에 적은 문자열(GameData)과 똑같이 입력 과정 필요
        */

        return FPrimaryAssetId(FPrimaryAssetType("GameData"), DataID);
    }
};
