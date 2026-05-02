// Fill out your copyright notice in the Description page of Project Settings.


#include "Base/Component/VL_HealthComponent.h"

// Sets default values for this component's properties
UVL_HealthComponent::UVL_HealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}




// Called when the game starts
void UVL_HealthComponent::BeginPlay()
{
	Super::BeginPlay();

	CurHP = MaxHP;
	// ...
	
}

void UVL_HealthComponent::ApplyDamage(float DamageAmount, AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsDead)
	{
		return;
	}

	// Clamp(x, min, max)
	// x가 min보다 작으면 min으로 바꿈
	// x가 max보다 크면 max로 바꿈
	// Clamp(-5, 0, 100) → 0
	// Clamp(30, 0, 100) → 30
	// Clamp(150, 0, 100) → 100
	// Clamp(x, min, max)

	CurHP = FMath::Clamp(CurHP - DamageAmount, 0.f, MaxHP);

	if (CurHP <= 0)
	{
		OnDead();
	}
}

void UVL_HealthComponent::ApplyHeal(float HealAmount)
{
	if (bIsDead)	// 죽었을 때 부활시킬지 말지 정하기
	{
		return;
	}

	CurHP = FMath::Clamp(CurHP + HealAmount, 0.f, MaxHP);
}

void UVL_HealthComponent::OnDead()
{
	bIsDead = true;
}

// Called every frame
void UVL_HealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

