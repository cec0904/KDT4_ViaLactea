// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeWeaponBase.h"
#include "Components/CapsuleComponent.h"
#include "Engine/EngineTypes.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/Controller.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/Pawn.h"

namespace
{
	constexpr ECollisionChannel WeaponCollisionChannel = ECC_GameTraceChannel5;
}

AMeleeWeaponBase::AMeleeWeaponBase()
{
	WeaponCollisionRadius = 8.f;
	WeaponCollisionHalfHeight = 45.f;

	WeaponCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("WeaponCollision"));
	WeaponCollision->SetupAttachment(SKEquipmentMesh);
	WeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponCollision->SetCollisionObjectType(WeaponCollisionChannel);
	WeaponCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	WeaponCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	WeaponCollision->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
	WeaponCollision->SetGenerateOverlapEvents(true);
	WeaponCollision->OnComponentBeginOverlap.AddDynamic(this, &AMeleeWeaponBase::OnWeaponBeginOverlap);

	UpdateWeaponCollisionShape();
}

void AMeleeWeaponBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	UpdateWeaponCollisionShape();
}

void AMeleeWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CheckOverlapCandidatesDuringDamageWindow();
}

void AMeleeWeaponBase::StartPrimaryAction()
{
	if (CurrentMontage)
	{
		HandleDisableCollision();
	}

	ResetSwingHitActors();
	Super::StartPrimaryAction();
}

void AMeleeWeaponBase::StartSecondaryAction()
{
	if (CurrentMontage)
	{
		HandleDisableCollision();
	}

	ResetSwingHitActors();
	Super::StartSecondaryAction();
}

void AMeleeWeaponBase::StartAbilityAction()
{
	if (CurrentMontage)
	{
		HandleDisableCollision();
	}

	ResetSwingHitActors();
	Super::StartAbilityAction();
}

void AMeleeWeaponBase::ForceResetState()
{
	HandleDisableCollision();
	ResetSwingHitActors();
	Super::ForceResetState();
}

void AMeleeWeaponBase::OnMontageStarted(UAnimMontage* Montage)
{
	Super::OnMontageStarted(Montage);

	if (Montage && Montage == CurrentMontage)
	{
		ResetSwingHitActors();
	}
}

void AMeleeWeaponBase::OnNaturalMontageEnd()
{
	HandleDisableCollision();
	ResetSwingHitActors();
}

void AMeleeWeaponBase::OnInterruptedMontageEnd()
{
	HandleDisableCollision();
	ResetSwingHitActors();
}

void AMeleeWeaponBase::HandleEnableCollision()
{
	bDamageWindowOpen = true;

	if (WeaponCollision)
	{
		WeaponCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
}

void AMeleeWeaponBase::HandleDisableCollision()
{
	bDamageWindowOpen = false;

	if (WeaponCollision)
	{
		WeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AMeleeWeaponBase::HandleCanAction()
{
	ResetSwingHitActors();
}

void AMeleeWeaponBase::HandleResetHitActors()
{
	ResetSwingHitActors();
}

void AMeleeWeaponBase::OnWeaponBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!HasAuthority() || !bDamageWindowOpen)
	{
		return;
	}

	AActor* DamageCauser = GetOwner();
	if (!OtherActor || OtherActor == this || OtherActor == DamageCauser)
	{
		return;
	}

	if (HasActorBeenHitThisSwing(OtherActor))
	{
		return;
	}

	TryApplyDamageToActor(OtherActor, OtherComp, OverlappedComponent);
}

void AMeleeWeaponBase::ResetSwingHitActors()
{
	HitActorsThisSwing.Reset();
}

bool AMeleeWeaponBase::HasActorBeenHitThisSwing(AActor* OtherActor) const
{
	for (const TWeakObjectPtr<AActor>& HitActor : HitActorsThisSwing)
	{
		if (HitActor.Get() == OtherActor)
		{
			return true;
		}
	}

	return false;
}

void AMeleeWeaponBase::MarkActorHitThisSwing(AActor* OtherActor)
{
	HitActorsThisSwing.Add(OtherActor);
}

void AMeleeWeaponBase::UpdateWeaponCollisionShape()
{
	if (!WeaponCollision)
	{
		return;
	}

	const float ClampedRadius = FMath::Max(1.f, WeaponCollisionRadius);
	const float ClampedHalfHeight = FMath::Max(ClampedRadius, WeaponCollisionHalfHeight);

	WeaponCollision->SetCapsuleSize(ClampedRadius, ClampedHalfHeight);
	WeaponCollision->SetRelativeLocation(WeaponCollisionOffset);
	WeaponCollision->SetRelativeRotation(WeaponCollisionRotation);
}

void AMeleeWeaponBase::CheckOverlapCandidatesDuringDamageWindow()
{
	if (!HasAuthority() || !bDamageWindowOpen || !WeaponCollision)
	{
		return;
	}

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_PhysicsBody);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(MeleeWeaponDamageOverlap), false);
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(GetOwner());

	TArray<FOverlapResult> OverlapResults;
	const bool bHasOverlap = GetWorld()->OverlapMultiByObjectType(
		OverlapResults,
		WeaponCollision->GetComponentLocation(),
		WeaponCollision->GetComponentQuat(),
		ObjectQueryParams,
		FCollisionShape::MakeCapsule(
			WeaponCollision->GetScaledCapsuleRadius(),
			WeaponCollision->GetScaledCapsuleHalfHeight()),
		QueryParams);

	if (!bHasOverlap)
	{
		return;
	}

	for (const FOverlapResult& OverlapResult : OverlapResults)
	{
		AActor* OverlappingActor = OverlapResult.GetActor();
		if (!OverlappingActor || OverlappingActor == this || OverlappingActor == GetOwner())
		{
			continue;
		}

		TryApplyDamageToActor(OverlappingActor, OverlapResult.GetComponent(), WeaponCollision);
	}
}

void AMeleeWeaponBase::TryApplyDamageToActor(AActor* OtherActor, UPrimitiveComponent* HitComponent, UPrimitiveComponent* SourceComponent)
{
	if (!OtherActor || OtherActor == this || OtherActor == GetOwner())
	{
		return;
	}

	if (HasActorBeenHitThisSwing(OtherActor))
	{
		return;
	}

	MarkActorHitThisSwing(OtherActor);
	ApplyDamageToActor(OtherActor, HitComponent, SourceComponent);
}
