// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "VFXData.h"
#include "VL_LightningProjectile.generated.h"

class USphereComponent;
class UNiagaraComponent;
class UVL_VFXDataAsset;


UCLASS()
class VIALACTEA_API AVL_LightningProjectile : public AActor
{
	GENERATED_BODY()

public:
    AVL_LightningProjectile();

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void ApplyInstantDamage();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION()
    void OnRep_VFXData();

    void UpdateProjectileState();

public:
    UPROPERTY(ReplicatedUsing = OnRep_VFXData)
    FVFXData VFXData;

    // 데미지 반경 및 충돌 판정을 위한 구체 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* DamageSphere;

    // 낙뢰 이펙트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UNiagaraComponent* StrikeVFX;

    // 보스에서 주입받을 데미지 값
    float StrikeDamage = 40.0f;

    // 초기화 함수
    void InitializeStrike(UVL_VFXDataAsset* InAsset, FGameplayTag InTag, float InDamage);
};
