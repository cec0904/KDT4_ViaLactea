#pragma once

#include "CoreMinimal.h"
#include "Base/Data/Item/VL_ItemDataAsset.h"
#include "VL_WeaponDataAsset.generated.h"

class UVL_AnimComboSet;
class UVL_AnimSkillSet;
class UNiagaraSystem;

UCLASS()
class VIALACTEA_API UVL_WeaponDataAsset : public UVL_ItemDataAsset
{
	GENERATED_BODY()

public:
    // --- 애니메이션 참조 ---
    UPROPERTY(EditDefaultsOnly, Category = "Combat|Animations")
    UVL_AnimComboSet* ComboSet;

    UPROPERTY(EditDefaultsOnly, Category = "Combat|Animations")
    UVL_AnimSkillSet* SkillSet;

    // --- 공격 기본 정보 ---
    UPROPERTY(EditDefaultsOnly, Category = "Combat|Stat")
    float BaseDamage = 20.f;

    UPROPERTY(EditDefaultsOnly, Category = "Combat|Stat")
    float AttackSpeed = 1.0f; // 공격 속도 배율

    UPROPERTY(EditDefaultsOnly, Category = "Combat|Stat")
    float CriticalChance = 0.05f; // 치명타 확률

    //UPROPERTY(EditDefaultsOnly, Category = "Combat|Visual")
    //FName HandSocketName = FName("RightHandSocket"); // 장착할 손 소켓

    //UPROPERTY(EditDefaultsOnly, Category = "Combat|Visual")
    //FName SheathSocketName = FName("BackSocket"); // 비전투 시 수납할 소켓

    // --- 사운드 & 이펙트 ---
    UPROPERTY(EditDefaultsOnly, Category = "Combat|Effect")
    TSoftObjectPtr<USoundBase> HitSound; // 적중 시 사운드

    UPROPERTY(EditDefaultsOnly, Category = "Combat|Effect")
    TSoftObjectPtr<UNiagaraSystem> HitEffect; 

    virtual FPrimaryAssetId GetPrimaryAssetId() const override
    {
        return FPrimaryAssetId(FPrimaryAssetType("WeaponData"), DataID);
    }
};
