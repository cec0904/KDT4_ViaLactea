#pragma once

#include "CoreMinimal.h"
#include "Base/Data/VL_GameDataBase.h"
#include "Engine/Scene.h"
#include "VL_WorldRegionDataAsset.generated.h"

class ANiagaraActor;
class USoundBase;

UCLASS()
class VIALACTEA_API UVL_WorldRegionDataAsset : public UVL_GameDataBase
{
	GENERATED_BODY()
public:
    // 지역 식별 정보
    UPROPERTY(EditAnywhere, Category = "Identity")
    FText RegionName;

    // 환경 설정 (심리스 로딩 시 활용)
    UPROPERTY(EditAnywhere, Category = "Environment")
    TSoftObjectPtr<UWorld> LevelReference; // 실제 로드할 레벨

    UPROPERTY(EditAnywhere, Category = "Environment")
    TSoftClassPtr<AActor> SkySphereClass; // 지역별 특수 하늘/날씨

    // 게임플레이 데이터
    UPROPERTY(EditAnywhere, Category = "Gameplay")
    int32 RecommendedLevel;

    UPROPERTY(EditAnywhere, Category = "Gameplay")
    TArray<TSoftClassPtr<AActor>> SpawnableMonsters; // 등장 몬스터 목록

    // 사운드 및 연출
    UPROPERTY(EditAnywhere, Category = "Audio")
    TSoftObjectPtr<USoundBase> BackgroundMusic;

public:

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PostProcess")
    FPostProcessSettings PostProcessSettings;

    // 만약 포스트 프로세스 머티리얼(필터 등)을 따로 쓰고 싶다면 아래를 사용하세요.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PostProcess")
    TArray<TSoftObjectPtr<UMaterialInterface>> PostProcessMaterials;

    UPROPERTY(EditAnywhere, Category = "VFX")
    TSoftClassPtr<ANiagaraActor> WeatherEffectClass; // 눈, 비, 모래바람 등 니아가라 시스템

    // 2. 환경 사운드 (Audio)
    UPROPERTY(EditAnywhere, Category = "Audio")
    TSoftObjectPtr<USoundBase> AmbientSound; // 바람 소리, 숲의 새소리 등

    // 3. 기믹 및 물리 (Physics/Gimmick)
    UPROPERTY(EditAnywhere, Category = "Physics")
    float GravityScale = 1.0f; // 특정 행성이나 저중력 구역 대응

    UPROPERTY(EditAnywhere, Category = "Physics")
    FLinearColor FogColor; // 안개 색상 커스텀

    
    virtual FPrimaryAssetId GetPrimaryAssetId() const override
    {
        return FPrimaryAssetId("WorldRegion", GetFName());
    }
};
