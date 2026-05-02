
#pragma once

#include "CoreMinimal.h"
#include "Base/Character/VL_CharacterBase.h"
#include "VL_AICharacterBase.generated.h"


class UAnimMontage;
class UVL_MonsterDataAsset;
class AMainCharacterBase;
class UWidgetComponent;

DECLARE_MULTICAST_DELEGATE(FOnAttackFinished);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAIHPChanged, float, NewHP, float, MaxHP);

UCLASS()
class VIALACTEA_API AVL_AICharacterBase : public AVL_CharacterBase
{
    GENERATED_BODY()
public:
    AVL_AICharacterBase();

    FOnAttackFinished OnAttackFinished; // 공격 종료 대리자 및 핸들러

protected:
    virtual void BeginPlay() override;

    UAnimMontage* GetAttackMontage() const;


    UPROPERTY(VisibleAnywhere, Category = "Weapon")
    TObjectPtr<UStaticMeshComponent> WeaponMesh;

public:
    UFUNCTION(BlueprintCallable, Category = "Data")
    virtual UVL_MonsterDataAsset* GetMonsterDataAsset() const;


    UFUNCTION(BlueprintCallable, Category = "Stat")
    FORCEINLINE float GetCurrentHP() const { return CurrentHP; }

    // 최대 체력을 반환하는 Getter (데이터 에셋 기반)
    UFUNCTION(BlueprintCallable, Category = "Stat")
    float GetMaxHP() const;

    // 체력 비율(0.0 ~ 1.0)을 반환하는 편의 함수 (UI/페이즈 구분용)
    UFUNCTION(BlueprintCallable, Category = "Stat")
    float GetHPRatio() const;

    UPROPERTY(BlueprintAssignable, Category = "Stat")
    FOnAIHPChanged OnHPChanged;

    // 공격 실행 (BTTask_Attack에서 호출)
    bool Attack();
    void StopAttack();

    void SetIsAttacking(bool bNewState) { bIsAttacking = bNewState; }
    bool GetIsAttacking() const { return bIsAttacking; }
    void SetIsHit(bool bNewState) { bIsHit = bNewState; }
    bool GetIsGroggy() const { return bIsGroggy; }
    void DecreaseGroggyGauge(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AttackCheck(bool bCanParry, bool bShouldKnockback);


    void StopAllMontage(float BlendTime = 0.25f, UAnimMontage* SpecificMontage = nullptr);

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_StopMontage(float BlendTime, UAnimMontage* SpecificMontage);

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayAttack();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_StopAttack();

    float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser);

    void StartGroggyDuration(float Duration);

    // 서버에서 데미지를 처리하고 호출할 멀티캐스트 함수
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayHit();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayGroggy();

    // 실제 재생 로직 (기존 PlayHitMontage를 내부 로직으로 활용)
    void Internal_PlayMontage(UAnimMontage* MontageToPlay, bool bIsGroggyType);

    float PlayGroggyMontage();

    void OnHitMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    void OnGroggyMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    void HitEnded();

    void SetGroggy(bool IsGroggy);

    UFUNCTION(BlueprintCallable, Category = "Combat|Knockback")
    void ApplyKnockback(AActor* DamageCauser);

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_OnDeath();

    virtual void Die();

    // 이동 활성화 함수
    void EnableMovement();

    // 공격 몽타주가 종료될 때 호출될 함수 
    UFUNCTION()
    void HandleAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

protected:
    virtual void PossessedBy(AController* NewController) override;

    // 내부 상태 변수
    UPROPERTY(Replicated, VisibleAnywhere, Category = "Combat")
    bool bIsAttacking = false;

    UPROPERTY(Replicated, VisibleAnywhere, Category = "Combat")
    bool bIsHit = false; // 피격 중 여부
    // 그로기 상태 여부 (중복 실행 방지)
    UPROPERTY(Replicated, VisibleAnywhere, Category = "Combat")
    bool bIsGroggy = false;

    UPROPERTY(Replicated, VisibleAnywhere, Category = "Combat")
    bool bIsDead = false;

    UPROPERTY(Replicated, VisibleAnywhere, Category = "Combat|Groggy")
    float CurrentGroggyGauge = 0.f;

    //패링 
    void HandleParrySuccess(AMainCharacterBase* Parrier);

    /*UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayParriedReaction();*/

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|HPBar")
    float HPBarVisibleDistance = 2000.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI|HPBar")
    TObjectPtr<UWidgetComponent> HPWidgetComponent = nullptr;

    FTimerHandle HPBarVisibilityTimerHandle;

    void UpdateHPBarVisibility();

public:
    // 그로기 로직 처리 함수
    virtual bool ApplyGroggy(float DamageAmount);

    void ApplyHealing();

    void ApplyHealing(float HealAmount);


protected:
    UPROPERTY(ReplicatedUsing = OnRep_CurrentHP)
    float CurrentHP;

    UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Stat")
    float MaxHP = 1000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
    float BaseDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
    float BaseDefense;

    // 피격 위치에 대한 회전 tick 제어 변수
    AActor* HitActor = nullptr;

    bool bIsRotating = false;

    // 그로기 회복용 타이머 핸들
    FTimerHandle GroggyRecoveryTimerHandle;
    // 그로기 지속용 타이머 핸들
    FTimerHandle GroggyingTimerHandle;

    void StartRotatingToTarget();

public:
    void EndGroggyState();

protected:
    UFUNCTION()
    void OnRep_CurrentHP();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    virtual void Tick(float DeltaTime) override;
private:

    void StopRotationLogic();
    float RotationCurrentTime = 0.f;
    float MaxRotationTime = 3.f;
    float RotationSpeed = 0.f;
};
