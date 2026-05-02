// Fill out your copyright notice in the Description page of Project Settings.


#include "Base/Data/VL_GameManager.h"
#include "Engine/AssetManager.h" // 데이터 관리를 위한 필수 헤더
#include "VL_GameDataBase.h"
#include "CustomLog/CustomLog.h"

/*
* 
FOnDataLoadedNative Callback;
Callback.BindUObject(this, &UVL_GameManager::OnLoadCompleted);    
LoadDataAsync(ItemId, Callback);

게임 시작 또는 레벨 진입 시 (Pre-loading)
특정 이벤트 발생 시 (On-Demand) ex) 캐릭터가 무기를 얻으면 
위젯이나 다른 클래스에서 호출
if (UVL_GameManager* GM = GetWorld()->GetGameInstance<UVL_GameManager>())

LoadDataAsync->대기 -> 교체 애니메이션(시간이 걸린다면) -> OnLoadCompleted 완료 가  깔끔할듯 합니다.
*/

void UVL_GameManager::LoadDataAsync(FPrimaryAssetId AssetId, FOnDataLoadedNative OnLoadedCallback)
{
    // 이미 로드된 경우 즉시 콜백 실행
    if (IsAssetLoaded(AssetId))
    {
        OnLoadedCallback.ExecuteIfBound(AssetId);
        return;
    }

    FSoftObjectPath Path = UAssetManager::Get().GetPrimaryAssetPath(AssetId);

    // 델리게이트를 스트리머블 핸들에 바인딩하여 로드 완료 시점에 실행
    UAssetManager::GetStreamableManager().RequestAsyncLoad(Path, FStreamableDelegate::CreateLambda([this, AssetId, OnLoadedCallback]() {
        OnLoadedCallback.ExecuteIfBound(AssetId);
        }));
}

bool UVL_GameManager::IsAssetLoaded(FPrimaryAssetId AssetId) const
{
    // 1. AssetManager를 통해 해당 ID의 에셋 객체가 이미 존재하는지 확인
    UObject* LoadedObject = UAssetManager::Get().GetPrimaryAssetObject(AssetId);

    // 2. 객체가 존재하고 유효하다면 이미 로드된 상태임
    return (LoadedObject != nullptr);
}

void UVL_GameManager::OnLoadCompleted(FPrimaryAssetId AssetId)
{
    UAssetManager& Manager = UAssetManager::Get();

    // 4. 로드된 에셋 포인터 가져오기
    UVL_GameDataBase* LoadedItem = Manager.GetPrimaryAssetObject<UVL_GameDataBase>(AssetId);

    if (LoadedItem)
    {
        // 1. 강한 참조를 위해 맵에 보관 (GC 방지)
        LoadedDataMap.Add(AssetId, LoadedItem);

        // 2. 로딩이 끝났으므로 핸들 제거 (메모리 정리)
        LoadingHandles.Remove(AssetId);

        CUSTOM_LOG("성공적으로 로드됨: %s", *LoadedItem->DataName.ToString());
        // 이후 아이템 스폰이나 UI 업데이트 로직 수행
    }
}
// 맵을 완전히 이동하거나 이미 참조한 ptr을 사용하지 않을거라고 확신할 때만 캐시 날리기
// 우리게임은 자유로운 무기 교체가 가능하기 때문에 굳이 ptr 정리를 안해줘도 됩니다.
void UVL_GameManager::ClearDataCache()
{
    // 로딩 중인 핸들 모두 취소
    for (auto& Pair : LoadingHandles)
    {
        if (Pair.Value.IsValid()) Pair.Value->CancelHandle();
    }
    LoadingHandles.Empty();

    // 로드된 데이터 참조 제거 (이제 GC가 수거 가능해짐)
    LoadedDataMap.Empty();
}