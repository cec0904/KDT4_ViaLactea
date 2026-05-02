
#include "Base/Component/VL_StatComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

UVL_StatComponent::UVL_StatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UVL_StatComponent::BeginPlay()
{
	Super::BeginPlay();
	BroadcastAll();
}

void UVL_StatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UVL_StatComponent, CurrentHP);
	DOREPLIFETIME(UVL_StatComponent, MaxHP);
	DOREPLIFETIME(UVL_StatComponent, CurrentStamina);
	DOREPLIFETIME(UVL_StatComponent, MaxStamina);
	DOREPLIFETIME(UVL_StatComponent, bIsDead);
}

void UVL_StatComponent::OnRep_CurrentHP()
{
	OnHPChanged.Broadcast(CurrentHP, MaxHP);
}

void UVL_StatComponent::OnRep_CurrentStamina()
{
	OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
}

void UVL_StatComponent::OnRep_IsDead()
{
	if (bIsDead)
	{
		OnDeath.Broadcast();
	}
}

void UVL_StatComponent::InitializeStats(float InMaxHP, float InMaxStamina, bool bFullRecover)
{
	MaxHP = FMath::Max(0.f, InMaxHP);
	MaxStamina = FMath::Max(0.f, InMaxStamina);

	if (bFullRecover)
	{
		CurrentHP = MaxHP;
		CurrentStamina = MaxStamina;
	}
	else
	{
		CurrentHP = FMath::Clamp(CurrentHP, 0.f, MaxHP);
		CurrentStamina = FMath::Clamp(CurrentStamina, 0.f, MaxStamina);
	}

	bIsDead = (CurrentHP <= 0.f);
	BroadcastAll();
}

float UVL_StatComponent::GetHPRatio() const
{
	if (MaxHP <= KINDA_SMALL_NUMBER)
	{
		return 0.f;
	}
	return FMath::Clamp(CurrentHP / MaxHP, 0.f, 1.f);
}

float UVL_StatComponent::GetStaminaRatio() const
{
	if (MaxStamina <= KINDA_SMALL_NUMBER)
	{
		return 0.f;
	}
	return FMath::Clamp(CurrentStamina / MaxStamina, 0.f, 1.f);
}

bool UVL_StatComponent::CanSpendStamina(float Amount) const
{
	return !bIsDead && CurrentStamina >= Amount;
}

bool UVL_StatComponent::ConsumeStamina(float Amount)
{
	if (Amount <= 0.f)
	{
		return true;
	}

	if (!CanSpendStamina(Amount))
	{
		return false;
	}

	CurrentStamina = FMath::Clamp(CurrentStamina - Amount, 0.f, MaxStamina);

	UWorld* World = GetWorld();

	if (World)
	{
		LastStaminaConsumeTime = World->GetTimeSeconds();
	}
	OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
	return true;
}

void UVL_StatComponent::ConsumeStaminaTick(float Amount)
{
	if (bIsDead || Amount <= 0.f)
	{
		return;
	}

	const float Old = CurrentStamina;
	CurrentStamina = FMath::Clamp(CurrentStamina - Amount, 0.f, MaxStamina);

	UWorld* World = GetWorld();

	if (World)
	{
		LastStaminaConsumeTime = World->GetTimeSeconds();
	}

	if (!FMath::IsNearlyEqual(Old, CurrentStamina))
	{
		OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
	}
}

void UVL_StatComponent::RecoverStamina(float DeltaTime)
{
	UWorld* World = GetWorld();

	if (bIsDead || MaxStamina <= 0.f || !World)
	{
		//UE_LOG(LogTemp, Warning, TEXT("RecoverStamina RETURN 1 | Dead=%d MaxStamina=%.2f World=%s"),
		//	bIsDead ? 1 : 0,
		//	MaxStamina,
		//	World ? TEXT("Valid") : TEXT("Null"));
		return;
	}

	const float Now = World->GetTimeSeconds();
	const float Elapsed = Now - LastStaminaConsumeTime;

	//UE_LOG(LogTemp, Warning, TEXT("RecoverStamina ENTER | Current=%.2f Max=%.2f Elapsed=%.2f Delay=%.2f"),
	//	CurrentStamina,
	//	MaxStamina,
	//	Elapsed,
	//	StaminaRegenDelay);

	if (Elapsed < StaminaRegenDelay)
	{
		/*UE_LOG(LogTemp, Warning, TEXT("RecoverStamina RETURN 2 | Delay not passed"));*/
		return;
	}

	const float OldValue = CurrentStamina;
	CurrentStamina = FMath::Clamp(CurrentStamina + StaminaRegenPerSecond * DeltaTime, 0.f, MaxStamina);

	/*UE_LOG(LogTemp, Warning, TEXT("RecoverStamina APPLY | Old=%.2f New=%.2f RegenPerSec=%.2f Delta=%.4f"),
		OldValue,
		CurrentStamina,
		StaminaRegenPerSecond,
		DeltaTime);*/

	if (!FMath::IsNearlyEqual(OldValue, CurrentStamina))
	{
		OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
		/*UE_LOG(LogTemp, Warning, TEXT("RecoverStamina BROADCAST"));*/
	}
}

float UVL_StatComponent::ApplyDamage(float DamageAmount)
{
	if (bIsDead || DamageAmount <= 0.f)
	{
		return 0.f;
	}

	const float OldHP = CurrentHP;
	CurrentHP = FMath::Clamp(CurrentHP - DamageAmount, 0.f, MaxHP);

	if (!FMath::IsNearlyEqual(OldHP, CurrentHP))
	{
		OnHPChanged.Broadcast(CurrentHP, MaxHP);
	}

	if (CurrentHP <= 0.f && !bIsDead)
	{
		bIsDead = true;
		OnDeath.Broadcast();
	}

	return OldHP - CurrentHP;
}

float UVL_StatComponent::HealHP(float HealAmount)
{
	if (bIsDead || HealAmount <= 0.f)
	{
		return 0.f;
	}

	const float OldHP = CurrentHP;
	CurrentHP = FMath::Clamp(CurrentHP + HealAmount, 0.f, MaxHP);

	if (!FMath::IsNearlyEqual(OldHP, CurrentHP))
	{
		OnHPChanged.Broadcast(CurrentHP, MaxHP);
	}

	return CurrentHP - OldHP;
}

void UVL_StatComponent::BroadcastAll()
{
	OnHPChanged.Broadcast(CurrentHP, MaxHP);
	OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
}