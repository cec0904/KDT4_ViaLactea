// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "VL_GameManager.generated.h"

struct FStreamableHandle;

DECLARE_DELEGATE_OneParam(FOnDataLoadedNative, FPrimaryAssetId);

UCLASS()
class VIALACTEA_API UVL_GameManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	void LoadDataAsync(FPrimaryAssetId AssetId, FOnDataLoadedNative OnLoadedCallback);

public:
    bool IsAssetLoaded(FPrimaryAssetId AssetId) const;

    void ClearDataCache();
protected:
    // 로드 완료 시 실행될 내부 콜백
    void OnLoadCompleted(FPrimaryAssetId AssetId);

    // [중요] 로드 중인 핸들을 보관하여 로드 취소를 방지
    TMap<FPrimaryAssetId, TSharedPtr<FStreamableHandle>> LoadingHandles;

    // [중요] 로드 완료된 데이터 에셋을 보관 (나중에 꺼내 쓰기 위함)
    UPROPERTY()
    TMap<FPrimaryAssetId, TObjectPtr<class UVL_GameDataBase>> LoadedDataMap;

};
