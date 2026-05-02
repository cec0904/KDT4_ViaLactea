// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VL_HealthComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VIALACTEA_API UVL_HealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UVL_HealthComponent();

	UPROPERTY(BlueprintReadWrite, Category = "Components")
	float CurHP = 0.f;

	UPROPERTY(BlueprintReadWrite, Category = "Components")
	float MaxHP = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Components")
	bool bIsDead = false;


	// 데미지 받아서 체력 깎이는거
	UFUNCTION(BlueprintCallable, Category = "Components")
	void ApplyDamage(float DamageAmount, AController* EventInstigator = nullptr, AActor* DamageCauser = nullptr);

	// 체력 회복
	UFUNCTION(BlueprintCallable, Category = "Components")
	void ApplyHeal(float HealAmount);

protected:

	// Called when the game starts
	virtual void BeginPlay() override;

	// 죽었을 때, hp 가 0이 되었을 때
	void OnDead();

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
