#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "VFXData.h"
#include "VL_LaserProjectile.generated.h"

class UNiagaraComponent;
class UCapsuleComponent;
class UVL_VFXDataAsset;

UCLASS()
class VIALACTEA_API AVL_LaserProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVL_LaserProjectile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	void InitializeLaser(const UVL_VFXDataAsset* InDataAsset, FGameplayTag InTag, AActor* InTargetActor, float InDuration, float InSize);

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing = OnRep_VFXData)
	FVFXData VFXData;

protected:
    // 1. 컴포넌트 구성
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* DefaultRoot;

    // 레이저 비주얼 (나이아가라)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UNiagaraComponent* LaserVisualComp;


    // 디버그 드로잉 여부 (개발용)
    UPROPERTY(EditAnywhere, Category = "Laser Settings")
    bool bShowDebugLine = true;

    // 2. 레이저 로직 변수
    UPROPERTY(Replicated)
    FVector StartLocation;      // 레이저 시작점 (보스 소켓 위치)

    UPROPERTY(Replicated)
    FVector FinalTargetLocation; // 최종 목표 지점 (Z가 올라간 상태)
    
    UPROPERTY(Replicated)
    FRotator InitialRotation; // 처음 레이저가 닿는 바닥 지점 (XY는 Target과 동일, Z만 낮음)

    UPROPERTY(Replicated)
    float ServerStartTime;

    UPROPERTY(Replicated)
    float MaxDuration;          // 목표까지 도달하는 총 시간 (2.0s)

    float CurrentElapsedTime;   // 경과 시간

    float DamageAmount;

    //레이저 사이즈 x, y
    float LaserRadius = 50.f;
    float LaserMaxRange = 8000.f;

    UPROPERTY(Replicated)
    bool bIsLaserActive;

    // 3. 내부 처리 함수
    UFUNCTION()
    void OnRep_VFXData();

    void UpdateLaserTransform(float DeltaTime);
    void ProcessDamage(FVector BeamStart, FVector BeamEnd);

    // 패턴 종료 시 호출
    void DeactivateLaser();

private:
    // 중복 데미지 방지 등을 위한 타이머나 액터 목록
    TArray<AActor*> HitActors;
    AActor* TargetActor;
};
