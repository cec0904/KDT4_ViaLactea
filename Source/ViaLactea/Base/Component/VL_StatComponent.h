#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"
#include "VL_StatComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHPChanged, float, NewHP, float, MaxHP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStaminaChanged, float, NewStamina, float, MaxStamina);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class VIALACTEA_API UVL_StatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVL_StatComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_CurrentHP, VisibleAnywhere, BlueprintReadOnly, Category = "Stat|HP")
	float CurrentHP = 100.f;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Stat|HP")
	float MaxHP = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentStamina, VisibleAnywhere, BlueprintReadOnly, Category = "Stat|Stamina")
	float CurrentStamina = 100.f;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Stat|Stamina")
	float MaxStamina = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_IsDead, VisibleAnywhere, BlueprintReadOnly, Category = "Stat")
	bool bIsDead = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat|Stamina")
	float StaminaRegenPerSecond = 25.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat|Stamina")
	float StaminaRegenDelay = 1.5f;

	float LastStaminaConsumeTime = -1000.f;

	UFUNCTION()
	void OnRep_CurrentHP();

	UFUNCTION()
	void OnRep_CurrentStamina();

	UFUNCTION()
	void OnRep_IsDead();

public:
	UPROPERTY(BlueprintAssignable, Category = "Stat")
	FOnHPChanged OnHPChanged;

	UPROPERTY(BlueprintAssignable, Category = "Stat")
	FOnStaminaChanged OnStaminaChanged;

	UPROPERTY(BlueprintAssignable, Category = "Stat")
	FOnDeath OnDeath;

public:
	UFUNCTION(BlueprintCallable, Category = "Stat")
	void InitializeStats(float InMaxHP, float InMaxStamina, bool bFullRecover = true);

	UFUNCTION(BlueprintCallable, Category = "Stat")
	float GetCurrentHP() const { return CurrentHP; }

	UFUNCTION(BlueprintCallable, Category = "Stat")
	float GetMaxHP() const { return MaxHP; }

	UFUNCTION(BlueprintCallable, Category = "Stat")
	float GetCurrentStamina() const { return CurrentStamina; }

	UFUNCTION(BlueprintCallable, Category = "Stat")
	float GetMaxStamina() const { return MaxStamina; }

	UFUNCTION(BlueprintCallable, Category = "Stat")
	float GetHPRatio() const;

	UFUNCTION(BlueprintCallable, Category = "Stat")
	float GetStaminaRatio() const;

	UFUNCTION(BlueprintCallable, Category = "Stat")
	bool IsDead() const { return bIsDead; }

	UFUNCTION(BlueprintCallable, Category = "Stat")
	bool CanSpendStamina(float Amount) const;

	UFUNCTION(BlueprintCallable, Category = "Stat")
	bool ConsumeStamina(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Stat")
	void ConsumeStaminaTick(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Stat")
	void RecoverStamina(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Stat")
	float ApplyDamage(float DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "Stat")
	float HealHP(float HealAmount);

	UFUNCTION(BlueprintCallable, Category = "Stat")
	void BroadcastAll();

};
