#pragma once

#include "CoreMinimal.h"
#include "Base/Data/VL_GameDataBase.h"
#include "VL_CharacterTypes.h"
#include "GameplayTagContainer.h"
#include "Base/Interfaces/VL_CooldownProvider.h"
#include "VL_CharacterDataAsset.generated.h"

UCLASS()
class VIALACTEA_API UVL_CharacterDataAsset : public UVL_GameDataBase, public IVL_CooldownProvider
{
	GENERATED_BODY()
public:
    // --- 비주얼 (Visual) ---
    // 캐릭터의 본대 메쉬 (소프트 레퍼런스)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Visual")
    TSoftObjectPtr<USkeletalMesh> SkeletalMesh = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Visual")
    TSubclassOf<UAnimInstance> AnimBlueprintClass = nullptr;

    // 피격 시 재생할 몽타주
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Animation")
    TSoftObjectPtr<UAnimMontage> HitMontage = nullptr;

    // 사망 시 재생할 몽타주
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Animation")
    TSoftObjectPtr<UAnimMontage> DeathMontage = nullptr;

    // 무기가 없을 때 맨손 공격 (콤보 없음)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Animation")
    TSoftObjectPtr<UAnimMontage> AttackMontage = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Animation")
    TSoftObjectPtr<UAnimMontage> HeavyAttack = nullptr;

    UPROPERTY(EditAnywhere, Category = "Attack|Socket")
    FName WeaponSocketName = TEXT("RightHandSocket");

    // --- 이동 (Movement) ---
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Movement")
    float RotationRate = 540.0f; // 회전 속도

    UPROPERTY(EditAnywhere, Category = "Stats")
    FVLCharacterStats BaseStats;

    // --- 물리/충돌 (Physics) ---
    // 캐릭터마다 덩치가 다르므로 캡슐 컴포넌트 크기를 데이터화합니다.
    //UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Physics")
    //float CapsuleRadius = 34.0f;

    //UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Physics")
    //float CapsuleHalfHeight = 88.0f;


public:
    // 기본적으로는 0을 반환하거나, 공통 스킬 쿨타임이 있다면 여기서 처리
    virtual float GetCooldownValue(FGameplayTag Tag) const override;

    // --- 에셋 매니저 식별자 재정의 ---
    virtual FPrimaryAssetId GetPrimaryAssetId() const override
    {
        // 타입을 "Character"로 분리하여 관리하는 것을 추천합니다.
        return FPrimaryAssetId(FPrimaryAssetType("Character"), DataID);
    }
};