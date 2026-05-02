// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "VFXData.h"
#include "VL_MeteorProjectile.generated.h"

class USphereComponent;
class UNiagaraComponent;
class UProjectileMovementComponent;
class UVL_VFXDataAsset;
struct FVLEffectInfo;

UCLASS()
class VIALACTEA_API AVL_MeteorProjectile : public AActor
{
	GENERATED_BODY()
public:
	// Sets default values for this actor's properties
	AVL_MeteorProjectile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	void InitializeProjectile(const UVL_VFXDataAsset* InDataAsset, FGameplayTag InTag, float InDamage, float InRadius, FVector InVelocity, FVector InSpawnOffset, bool InIsVFXMoving);
	// void InitializeProjectile(const UVL_VFXDataAsset* InDataAsset, FGameplayTag InTag, float InDamage);

public:
	UPROPERTY(ReplicatedUsing = OnRep_VFXData)
	FVFXData VFXData;

protected:
	// 실제 충돌 처리를 담당할 구체 컴포넌트 (루트)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* CollisionComp;

	// 운석 시각 효과 (나이아가라)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UNiagaraComponent* NiagaraComp;

	// 이동을 담당할 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UProjectileMovementComponent* ProjectileMovementComp;

	// 물체에 닿는 폭발 소리까지 있다면 추가해야함
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<USoundBase> ExplosionSound;

	//UPROPERTY(Replicated)
	//const UVL_VFXDataAsset* VFXDataAsset;

	//UPROPERTY(ReplicatedUsing = OnRep_EffectTag)
	//FGameplayTag EffectTag;


	// 충돌 시 호출될 함수 (Overlap)
	UFUNCTION()
	void OnMeteorOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnMeteorHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	void DeleteProjectile();

	UFUNCTION()
	void OnRep_VFXData();

	// 서버에서 충돌 시 호출하여 모든 클라이언트에게 폭발 이펙트를 재생하라고 지시
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayImpactVFX(FVector HitLocation);

	// 2. 복제 설정 함수
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


protected:
	UPROPERTY()
	float DamageAmount;
	
	UPROPERTY(Replicated)
	float SphereSize = 50.f;
	
	// 게임 로직용 변수
	UPROPERTY()
	TArray<AActor*> OverlappedActors;

	FTimerHandle DebugTimerHandle;

	UPROPERTY(Replicated)
	bool IsVFXMoving= false;

	UPROPERTY(Replicated)
	bool bHasExploded = false;
};
