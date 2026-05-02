// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/Actor/VL_GimmickActorBase.h"
#include "Base/AI/GimmickTypes.h"
#include "Base/Interfaces/VL_BossGimmickInterface.h"
#include "VL_BossGimmickActor.generated.h"

UCLASS()
class VIALACTEA_API AVL_BossGimmickActor : public AVL_GimmickActorBase, public IVL_BossGimmickInterface
{
	GENERATED_BODY()

public:
    AVL_BossGimmickActor();

protected:
    virtual void BeginPlay() override;

    // --- 컴포넌트 ---
    // 블루프린트에서 모양과 콜리전을 쉽게 세팅할 수 있도록 루트와 메쉬만 기본 제공
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* MeshComp;

    // --- 변수 ---
    UPROPERTY(ReplicatedUsing = OnRep_CurrentState, VisibleAnywhere, BlueprintReadOnly, Category = "Gimmick|State")
    EGimmickState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gimmick|Stat")
    float MaxHealth;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gimmick|Stat")
    float CurrentHealth;

public:
    // --- 이벤트 (델리게이트) ---
    UPROPERTY(BlueprintAssignable, Category = "Gimmick|Events")
    FOnGimmickStateChanged OnStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Gimmick|Events")
    FOnGimmickDestroyed OnGimmickDestroyed;

    // --- 핵심 함수 ---
    // 1. 상태 변경 (서버에서 호출)
    UFUNCTION(BlueprintCallable, Category = "Gimmick")
    virtual void SetGimmickState(EGimmickState NewState);

    // 2. 데미지 처리 (화살 타격, 보스 돌진 등 모든 타격을 여기서 1차로 받음)
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

protected:
    // 상태가 변했을 때 연출(이펙트, 머티리얼 변경 등)을 처리할 가상 함수
    UFUNCTION(BlueprintNativeEvent, Category = "Gimmick")
    void HandleStateVisuals(EGimmickState State);

    // 네트워크 리플리케이션용 (클라이언트에서 이펙트 재생)
    UFUNCTION()
    void OnRep_CurrentState();

public:
    // --- 인터페이스 구현부 (IBossGimmickInterface) ---
    virtual bool IsTargetable_Implementation() const override;
    virtual FVector GetGimmickTargetLocation_Implementation() const override;
};
