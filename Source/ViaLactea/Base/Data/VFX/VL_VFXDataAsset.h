#pragma once

#include "CoreMinimal.h"
#include "Base/Data/VL_GameDataBase.h"
#include "GameplayTagContainer.h"
#include "NiagaraSystem.h"
#include "VL_VFXDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FVLEffectInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSoftObjectPtr<USoundBase> Sound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSoftObjectPtr<UNiagaraSystem> ImpactNiagara = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSoftObjectPtr<USoundBase> ImpactSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float CollisionRadius = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float ZSpeed = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float MovingPower = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics")
    FVector SpawnOffset = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool IsVFXMoving = false;

};

UCLASS()
class VIALACTEA_API UVL_VFXDataAsset : public UVL_GameDataBase
{
	GENERATED_BODY()
public:
    // 패턴 태그를 키로, 나이아가라 시스템을 값으로 하는 맵
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Boss")
    TMap<FGameplayTag, FVLEffectInfo> BossVFXMap;

    const FVLEffectInfo* GetEffectInfoByTag(FGameplayTag Tag) const
    {
        return BossVFXMap.Find(Tag);
    }

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraSystem* GetVFXByTag(FGameplayTag Tag) const
    {
        if (const FVLEffectInfo* FoundInfo = BossVFXMap.Find(Tag))
        {
            return FoundInfo->NiagaraSystem.LoadSynchronous();
        }
        return nullptr;
    }
};
