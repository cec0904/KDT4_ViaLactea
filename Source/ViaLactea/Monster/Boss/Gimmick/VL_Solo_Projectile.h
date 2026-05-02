#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "VFXData.h"
#include "VL_Solo_Projectile.generated.h"

class USphereComponent;
class UNiagaraComponent;
struct FVLEffectInfo;

UCLASS()
class VIALACTEA_API AVL_Solo_Projectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVL_Solo_Projectile();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_VFXData, meta = (ExposeOnSpawn = "true"), Category = "VFX")
    FVFXData VFXData;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
    // 스폰 직후 보스가 호출해줄 초기화 함수
    void InitializeProjectile(UVL_VFXDataAsset* InDataAsset, FGameplayTag InLoopTag, FGameplayTag InExplosionTag);

    // --- 컴포넌트 ---
    UPROPERTY(VisibleAnywhere, Category = "Components")
    USphereComponent* CollisionComponent;

    UPROPERTY()
    UNiagaraComponent* NiagaraComponent;

    UPROPERTY(EditAnywhere, Category = "Settings")
    float LifeSpan = 3.0f; // 총 지속 시간

    UPROPERTY(EditAnywhere, Category = "Settings")
    float DamageInterval = 0.5f; // 데미지 주기

    UPROPERTY(EditAnywhere, Category = "Settings")
    float DamageAmount = 3.0f;

private:
    FTimerHandle DamageTimerHandle;
    FTimerHandle LifeSpanTimerHandle;
    FTimerHandle ExplosionDamageTimerHandle;

    // 폭발용 태그 저장
    FGameplayTag SavedExplosionTag;
private:

    void ApplyTickDamage(); // 주기적 데미지 처리
    void Explode();         // 최종 폭발 처리
    void ExecuteExplosionDamage();

    UFUNCTION()
    void OnRep_VFXData();

    void InitCosmetics();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_Explode(FGameplayTag ExplosionTag);
};
