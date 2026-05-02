// Fill out your copyright notice in the Description page of Project Settings.

#include "BowBase.h"
#include "ArrowProjectile.h"
#include "BowAnimInstance.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Player/MainCharacterBase.h"
#include "Net/UnrealNetwork.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

//KDH 추가
#include "Player/VL_PlayerState.h"
#include "Base/Component/VL_InventoryComponent.h"
//

namespace
{
	constexpr ECollisionChannel ArrowTraceChannel = ECC_GameTraceChannel4;
}

ABowBase::ABowBase()
{
	WeaponType = EWeaponAnimType::Bow;
	EquipmentType = EEquipmentSlotType::Bow;
	Damage = 20.f;
	PrimaryAttackDamageMultipliers = { 1.0f };
	SecondaryActionDamageMultiplier = 1.0f;
	AbilityActionDamageMultiplier = 1.4f;
	BowDrawSoundAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("BowDrawSoundAudioComponent"));
	BowDrawSoundAudioComponent->SetAutoActivate(false);
	BowDrawSoundAudioComponent->bAutoActivate = false;
	if (SKEquipmentMesh)
	{
		BowDrawSoundAudioComponent->SetupAttachment(SKEquipmentMesh);
	}
	UE_LOG(LogTemp, Warning, TEXT("[BowBase] Constructor | EquipmentType=%d"), (int32)EquipmentType);
}

void ABowBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABowBase, bIsZooming);
	DOREPLIFETIME(ABowBase, bIsDrawing);
	DOREPLIFETIME(ABowBase, CurrentDrawDuration);
	DOREPLIFETIME(ABowBase, ReplicatedDrawAlpha);
	DOREPLIFETIME(ABowBase, bPendingArrowFire);
}

void ABowBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority())
	{
		ReplicatedDrawAlpha = CalculateCurrentDrawAlphaLocal();
	}

	UpdateBowAnimationData();
	if (DrawArrow)
	{
		if (ShouldHoldBowString())
		{
			UpdatePreviewArrowTransform();
		}
		else
		{
			DestroyDrawArrow();
		}
	}

	DrawPredictedArrowPathDebug();
}

void ABowBase::ConfigureBowAnimationInstance()
{
	if (!BowAnimInstanceClass || !SKEquipmentMesh || SKEquipmentMesh->GetAnimClass() == BowAnimInstanceClass)
	{
		return;
	}

	SKEquipmentMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	SKEquipmentMesh->SetAnimInstanceClass(BowAnimInstanceClass);
}

void ABowBase::OnEquip(ACharacter* NewOwner)
{
	Super::OnEquip(NewOwner);

	ConfigureBowAnimationInstance();
	UpdateBowAnimationData();
}

void ABowBase::OnRep_Owner()
{
	Super::OnRep_Owner();

	ConfigureBowAnimationInstance();
	UpdateBowAnimationData();
}

bool ABowBase::OnUnequip()
{
	if (SKEquipmentMesh)
	{
		if (UBowAnimInstance* BowAnimInstance = Cast<UBowAnimInstance>(SKEquipmentMesh->GetAnimInstance()))
		{
			BowAnimInstance->SetBowAnimationData(false, 0.f, false, FVector::ZeroVector, FVector::ZeroVector);
		}
	}

	bHasValidPullHandTarget = false;
	PullHandWorldLocation = FVector::ZeroVector;
	PullHandLocationCS = FVector::ZeroVector;
	PendingArrowSpeed = 0.f;
	PendingArrowLaunchDirection = FVector::ZeroVector;
	PendingArrowTargetPoint = FVector::ZeroVector;
	bPendingArrowFire = false;
	StopBowDrawSoundLocal();

	return Super::OnUnequip();
}

void ABowBase::CompleteUnequip()
{
	if (SKEquipmentMesh)
	{
		if (UBowAnimInstance* BowAnimInstance = Cast<UBowAnimInstance>(SKEquipmentMesh->GetAnimInstance()))
		{
			BowAnimInstance->SetBowAnimationData(false, 0.f, false, FVector::ZeroVector, FVector::ZeroVector);
		}
	}

	bHasValidPullHandTarget = false;
	PullHandWorldLocation = FVector::ZeroVector;
	PullHandLocationCS = FVector::ZeroVector;
	PendingArrowSpeed = 0.f;
	PendingArrowLaunchDirection = FVector::ZeroVector;
	PendingArrowTargetPoint = FVector::ZeroVector;
	bPendingArrowFire = false;
	StopBowDrawSoundLocal();

	Super::CompleteUnequip();
}

void ABowBase::StartSecondaryAction()
{
	if (bIsZooming || !OwnerCharacter || bIsDrawing) return;

	if (!HasArrowAmmo())
	{
		UE_LOG(LogTemp, Warning, TEXT("[BowBase] 화살 부족 - 조준 불가 | ItemID=%s"), *RequiredArrowItemID.ToString());
		return;
	}

	if (!HasAuthority() && OwnerCharacter->IsLocallyControlled())
	{
		ServerStartSecondaryAction();
	}

	bIsZooming = true;
	EnsureDrawArrowAttached();
	BroadcastAimState();
}

void ABowBase::ServerStartSecondaryAction_Implementation()
{
	if (bIsZooming || !OwnerCharacter || bIsDrawing)
	{
		return;
	}

	if (!HasArrowAmmo())
	{
		UE_LOG(LogTemp, Warning, TEXT("[BowBase] Server 화살 부족 - 조준 불가 | ItemID=%s"), *RequiredArrowItemID.ToString());
		return;
	}

	bIsZooming = true;
	EnsureDrawArrowAttached();
	BroadcastAimState();
}

void ABowBase::ForceResetState()
{
	// 활 전용 상태 먼저 정리
	bIsZooming = false;
	CurrentDrawDuration = 0.f;
	PendingArrowSpeed = 0.f;
	PendingArrowLaunchDirection = FVector::ZeroVector;
	PendingArrowTargetPoint = FVector::ZeroVector;
	bPendingArrowFire = false;
	if (bIsDrawing)
	{
		StopDrawing();
	}

	if (HasAuthority())
	{
		MulticastStopBowDrawSound();
	}
	else
	{
		StopBowDrawSoundLocal();
	}

	DestroyDrawArrow();
	BroadcastAimState();

	// 부모 상태 초기화
	Super::ForceResetState();
}

void ABowBase::CancelAction()
{
	ForceResetState();
}

void ABowBase::StopSecondaryAction()
{
	if (!bIsZooming || !OwnerCharacter)
	{
		return;
	}

	if (!HasAuthority() && OwnerCharacter->IsLocallyControlled())
	{
		ServerStopSecondaryAction();
	}

	bIsZooming = false;
	if (!bPendingArrowFire)
	{
		PendingArrowSpeed = 0.f;
		PendingArrowLaunchDirection = FVector::ZeroVector;
		PendingArrowTargetPoint = FVector::ZeroVector;
	}

	if (!bIsDrawing && !bPendingArrowFire)
	{
		DestroyDrawArrow();
	}

	// 발사 몽타주 재생 중이면 방송 생략 (OnNaturalMontageEnd에서 처리)
	if (!CurrentMontage)
	{
		BroadcastAimState();
	}
}

void ABowBase::ServerStopSecondaryAction_Implementation()
{
	StopSecondaryAction();
}

bool ABowBase::CanStartAction(EEquipmentActionType ActionType) const
{
	if (ActionType == EEquipmentActionType::Primary)
	{
		return OwnerCharacter && !bIsDrawing && HasArrowAmmo();
	}

	if (ActionType == EEquipmentActionType::Secondary)
	{
		return OwnerCharacter && !bIsZooming && !bIsDrawing && HasArrowAmmo();
	}

	return Super::CanStartAction(ActionType);
}

bool ABowBase::ShouldConsumeStaminaForAction(EEquipmentActionType ActionType) const
{
	return CanStartAction(ActionType) && GetStaminaCostForAction(ActionType) > KINDA_SMALL_NUMBER;
}

void ABowBase::StartPrimaryAction()
{
	if (!OwnerCharacter || bIsDrawing) return;

	if (!HasArrowAmmo())
	{
		UE_LOG(LogTemp, Warning, TEXT("[BowBase] 화살 부족 - 공격 불가 | ItemID=%s"), *RequiredArrowItemID.ToString());
		return;
	}

	if (!HasAuthority() && OwnerCharacter->IsLocallyControlled())
	{
		ServerStartPrimaryAction();
	}

	bIsDrawing = true;
	CurrentDrawDuration = 0.f;
	PendingArrowSpeed = 0.f;
	PendingArrowLaunchDirection = FVector::ZeroVector;
	PendingArrowTargetPoint = FVector::ZeroVector;
	bPendingArrowFire = false;
	DrawStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;
	BroadcastAimState();

	if (HasAuthority() && BowDrawSoundCue)
	{
		MulticastStartBowDrawSound();
	}

	EnsureDrawArrowAttached();

	// 시위 당기기 몽타주 재생
	if (DrawMontage)
	{
		CurrentMontage = DrawMontage;
		PlayOwnerMontage(DrawMontage, 1.f, NAME_None, true);
	}
}

void ABowBase::ServerStartPrimaryAction_Implementation()
{
	if (!OwnerCharacter || bIsDrawing)
	{
		return;
	}

	if (!HasArrowAmmo())
	{
		UE_LOG(LogTemp, Warning, TEXT("[BowBase] Server 화살 부족 - 공격 불가 | ItemID=%s"), *RequiredArrowItemID.ToString());
		return;
	}

	bIsDrawing = true;
	CurrentDrawDuration = 0.f;
	PendingArrowSpeed = 0.f;
	PendingArrowLaunchDirection = FVector::ZeroVector;
	PendingArrowTargetPoint = FVector::ZeroVector;
	bPendingArrowFire = false;
	DrawStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;
	BroadcastAimState();

	if (BowDrawSoundCue)
	{
		MulticastStartBowDrawSound();
	}

	EnsureDrawArrowAttached();

	if (DrawMontage)
	{
		CurrentMontage = DrawMontage;
		PlayOwnerMontage(DrawMontage, 1.f, NAME_None, true);
	}
}

void ABowBase::StopPrimaryAction()
{
	UE_LOG(LogTemp, Warning, TEXT("[StopPrimaryAction] bIsDrawing: %d"), bIsDrawing);
	// 드로우 중이 아닐 땐 처리할 것 없음
	if (!bIsDrawing)
	{
		return;
	}

	if (!OwnerCharacter)
	{
		return;
	}

	float HeldDuration = 0.f;
	float ChargeAlpha = 0.f;
	const float ChargedArrowSpeed = CalculateChargedArrowSpeed(HeldDuration, ChargeAlpha);
	CurrentDrawDuration = HeldDuration;
	PendingArrowSpeed = ChargedArrowSpeed;
	FVector LaunchSocketLocation = FVector::ZeroVector;
	FVector LaunchDirection = FVector::ZeroVector;
	FVector LaunchTargetPoint = FVector::ZeroVector;
	GetArrowLaunchData(LaunchSocketLocation, LaunchDirection, &LaunchTargetPoint);
	PendingArrowLaunchDirection = LaunchDirection;
	PendingArrowTargetPoint = LaunchTargetPoint;
	bPendingArrowFire = true;
	SetCurrentDamageMultiplier(FMath::Lerp(MinChargeDamageMultiplier, FullChargeDamageMultiplier, ChargeAlpha));

	if (!HasAuthority() && OwnerCharacter->IsLocallyControlled())
	{
		ServerStopPrimaryAction(HeldDuration, ChargeAlpha, ChargedArrowSpeed, LaunchDirection, LaunchTargetPoint);
	}

	StopDrawing();

	// 발사 몽타주 재생 + OnMontageEnded 감지를 위해 CurrentMontage 설정
	if (basicAttackMontages.IsValidIndex(0))
	{
		CurrentMontage = basicAttackMontages[0];

		// 클라 프리뷰 화살은 발사 입력 순간 손에서 제거
		if (!HasAuthority() && OwnerCharacter->IsLocallyControlled())
		{
			DestroyDrawArrow();
		}

		PlayOwnerMontage(CurrentMontage);
	}
	else
	{
		bPendingArrowFire = false;
		FireArrow(ChargedArrowSpeed);
		PendingArrowSpeed = 0.f;
	}
}

void ABowBase::ServerStopPrimaryAction_Implementation(float ClientHeldDuration, float ClientChargeAlpha, float ClientArrowSpeed, FVector ClientLaunchDirection, FVector ClientTargetPoint)
{
	if (!OwnerCharacter || !bIsDrawing)
	{
		return;
	}

	if (!HasArrowAmmo())
	{
		ForceResetState();
		UE_LOG(LogTemp, Warning, TEXT("[BowBase] Server 화살 부족 - 발사 취소 | ItemID=%s"), *RequiredArrowItemID.ToString());
		return;
	}

	const float MaxHeldDuration = FullChargeTime > KINDA_SMALL_NUMBER ? FullChargeTime : ClientHeldDuration;
	const float SanitizedHeldDuration = FMath::Clamp(ClientHeldDuration, 0.f, FMath::Max(0.f, MaxHeldDuration));
	const float SanitizedChargeAlpha = FMath::Clamp(ClientChargeAlpha, 0.f, 1.f);
	const float LowSpeed = FMath::Min(MinArrowSpeed, ArrowSpeed);
	const float HighSpeed = FMath::Max(MinArrowSpeed, ArrowSpeed);
	const float SanitizedArrowSpeed = FMath::Clamp(ClientArrowSpeed, LowSpeed, HighSpeed);

	CurrentDrawDuration = SanitizedHeldDuration;
	PendingArrowSpeed = SanitizedArrowSpeed;
	PendingArrowLaunchDirection = ClientLaunchDirection.GetSafeNormal();
	PendingArrowTargetPoint = ClientTargetPoint;
	bPendingArrowFire = true;

	SetCurrentDamageMultiplier(FMath::Lerp(MinChargeDamageMultiplier, FullChargeDamageMultiplier, SanitizedChargeAlpha));

	StopDrawing();

	if (basicAttackMontages.IsValidIndex(0))
	{
		CurrentMontage = basicAttackMontages[0];

		// 서버도 몽타주를 재생해야 서버 노티파이가 뜸
		PlayOwnerMontage(CurrentMontage);
	}
	else
	{
		bPendingArrowFire = false;
		FireArrow(SanitizedArrowSpeed);
		PendingArrowSpeed = 0.f;
		PendingArrowLaunchDirection = FVector::ZeroVector;
		PendingArrowTargetPoint = FVector::ZeroVector;
	}
}

void ABowBase::StopDrawing()
{
	bIsDrawing = false;
	DrawStartTime = 0.0;

	if (DrawMontage && OwnerCharacter)
	{
		UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			const bool bWasPlaying = AnimInstance->Montage_IsPlaying(DrawMontage);
			UE_LOG(LogTemp, Warning, TEXT("[StopDrawing] DrawMontage IsPlaying: %d"), bWasPlaying);
			StopOwnerMontage(DrawMontage, 0.0f);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[StopDrawing] SKIP - DrawMontage=%s OwnerCharacter=%s"),
			DrawMontage ? TEXT("valid") : TEXT("NULL"),
			OwnerCharacter ? TEXT("valid") : TEXT("NULL"));
	}

	if (CurrentMontage == DrawMontage)
	{
		CurrentMontage = nullptr;
	}
}

void ABowBase::OnNaturalMontageEnd()
{
	bPendingArrowFire = false;
	PendingArrowSpeed = 0.f;
	PendingArrowLaunchDirection = FVector::ZeroVector;
	PendingArrowTargetPoint = FVector::ZeroVector;
	CurrentMontage = nullptr;

	DestroyDrawArrow();

	if (bIsZooming && !bIsDrawing)
	{
		EnsureDrawArrowAttached();
	}

	UpdateBowAnimationData();
	BroadcastAimState();
}

void ABowBase::OnMontageStarted(UAnimMontage* Montage)
{
	const bool bIsMyMontage =
		(Montage == DrawMontage) || basicAttackMontages.Contains(Montage);

	if (bIsMyMontage)
	{
		CurrentMontage = Montage;
	}

	// 줌 상태에서 내 몽타주(드로우/발사)가 아닌 외부 몽타주가 시작되면 정리
	if (bIsZooming || bIsDrawing)
	{
		if (!bIsMyMontage)
		{
			ForceResetState();
			return;
		}
	}

	Super::OnMontageStarted(Montage);
}

//KDH [2026.04.14]
float ABowBase::GetCrosshairChargeAlpha() const
{
	if (!bIsDrawing && !bPendingArrowFire)
	{
		return 0.f;
	}

	return GetCurrentDrawAlpha();
}

bool ABowBase::ShouldShowCrosshairChargeRing() const
{
	// 시위를 당기는 중일 때만 보여준다
	return bIsDrawing;
}
//
void ABowBase::HandleFireBowArrow()
{

	if (!bPendingArrowFire)
	{
		return;
	}

	const float ArrowSpeedToFire = PendingArrowSpeed;
	bPendingArrowFire = false;
	PendingArrowSpeed = 0.f;

	if (DrawArrow)
	{
		FireArrow(ArrowSpeedToFire);
	}
	else
	{
		PendingArrowLaunchDirection = FVector::ZeroVector;
		PendingArrowTargetPoint = FVector::ZeroVector;
	}

	// 우클릭 줌 중이 아니라면, 활 공격으로 켜진 Strafe를 여기서 해제
	if (!bIsZooming)
	{
		BroadcastAimState(); // bIsZooming=false && bIsDrawing=false → false
	}

	UpdateBowAnimationData();
}

void ABowBase::BroadcastAimState()
{
	OnZoomChanged.Broadcast(bIsZooming || bIsDrawing);
}

void ABowBase::UpdateBowAnimationData()
{
	if (!SKEquipmentMesh)
	{
		return;
	}

	UBowAnimInstance* BowAnimInstance = Cast<UBowAnimInstance>(SKEquipmentMesh->GetAnimInstance());
	if (!BowAnimInstance)
	{
		return;
	}

	FVector PullTargetWS = FVector::ZeroVector;
	bHasValidPullHandTarget = TryGetPullHandWorldLocation(PullTargetWS);
	PullHandWorldLocation = PullTargetWS;
	PullHandLocationCS = bHasValidPullHandTarget
		? SKEquipmentMesh->GetComponentTransform().InverseTransformPosition(PullTargetWS)
		: FVector::ZeroVector;

	BowAnimInstance->SetBowAnimationData(
		ShouldHoldBowString(),
		GetCurrentDrawAlpha(),
		bHasValidPullHandTarget,
		PullHandLocationCS,
		PullHandWorldLocation);
}

void ABowBase::UpdatePreviewArrowTransform()
{
	if (!DrawArrow)
	{
		return;
	}

	FVector PreviewLocation = FVector::ZeroVector;
	FRotator PreviewRotation = FRotator::ZeroRotator;
	if (!TryGetPreviewArrowTransform(PreviewLocation, PreviewRotation))
	{
		return;
	}

	DrawArrow->SetActorLocationAndRotation(
		PreviewLocation,
		PreviewRotation,
		false,
		nullptr,
		ETeleportType::TeleportPhysics);
}

bool ABowBase::TryGetPullHandWorldLocation(FVector& OutWorldLocation) const
{
	OutWorldLocation = FVector::ZeroVector;

	FTransform PullHandWorldTransform = FTransform::Identity;
	if (!TryGetPullHandWorldTransform(PullHandWorldTransform))
	{
		return false;
	}

	OutWorldLocation = PullHandWorldTransform.GetLocation();
	return true;
}

bool ABowBase::TryGetPullHandWorldTransform(FTransform& OutWorldTransform) const
{
	OutWorldTransform = FTransform::Identity;

	USkeletalMeshComponent* PullHandMesh = ResolveAttachMeshComponent(PullHandSocketName);
	if (PullHandMesh && PullHandMesh->DoesSocketExist(PullHandSocketName))
	{
		OutWorldTransform = PullHandMesh->GetSocketTransform(PullHandSocketName, RTS_World);
		return true;
	}

	PullHandMesh = ResolveAttachMeshComponent(PullHandFallbackSocketName);
	if (PullHandMesh && PullHandMesh->DoesSocketExist(PullHandFallbackSocketName))
	{
		OutWorldTransform = PullHandMesh->GetSocketTransform(PullHandFallbackSocketName, RTS_World);
		return true;
	}

	return false;
}

bool ABowBase::TryGetPreviewArrowTransform(FVector& OutLocation, FRotator& OutRotation) const
{
	OutLocation = FVector::ZeroVector;
	OutRotation = FRotator::ZeroRotator;

	if (!SKEquipmentMesh)
	{
		return false;
	}

	FTransform HandSocketTransform = FTransform::Identity;
	const bool bHasHandSocket = TryGetPullHandWorldTransform(HandSocketTransform);

	if (ShouldHoldBowString() && bHasHandSocket)
	{
		const FVector TailLocation = HandSocketTransform.GetLocation();

		FName GuideSocketName = ArrowPreviewGuideSocket;
		if (GuideSocketName.IsNone() || !SKEquipmentMesh->DoesSocketExist(GuideSocketName))
		{
			GuideSocketName = ArrowSpawnSocket;
		}

		FVector GuideLocation = TailLocation + (SKEquipmentMesh->GetForwardVector() * 100.f);
		if (!GuideSocketName.IsNone() && SKEquipmentMesh->DoesSocketExist(GuideSocketName))
		{
			GuideLocation = SKEquipmentMesh->GetSocketLocation(GuideSocketName);
		}

		const FVector Direction = (GuideLocation - TailLocation).GetSafeNormal();
		if (Direction.IsNearlyZero())
		{
			return false;
		}

		OutLocation = TailLocation;
		OutRotation = UKismetMathLibrary::FindLookAtRotation(TailLocation, GuideLocation);
		return true;
	}

	if (bHasHandSocket)
	{
		OutLocation = HandSocketTransform.GetLocation();
		OutRotation = HandSocketTransform.GetRotation().Rotator();
		return true;
	}

	if (!SKEquipmentMesh->DoesSocketExist(ArrowAttachSocket))
	{
		return false;
	}

	const FTransform AttachSocketTransform = SKEquipmentMesh->GetSocketTransform(ArrowAttachSocket, RTS_World);
	OutLocation = AttachSocketTransform.GetLocation();
	OutRotation = AttachSocketTransform.GetRotation().Rotator();
	return true;
}

void ABowBase::DrawPredictedArrowPathDebug() const
{
	if (!bDrawPredictedPathDebug || !OwnerCharacter || !OwnerCharacter->IsLocallyControlled() || !GetWorld())
	{
		return;
	}

	if (!ShouldHoldBowString())
	{
		return;
	}

	FVector StartLocation = FVector::ZeroVector;
	FVector Direction = FVector::ZeroVector;
	if (!GetArrowLaunchData(StartLocation, Direction))
	{
		return;
	}

	FPredictProjectilePathParams PredictParams;
	PredictParams.StartLocation = StartLocation;
	PredictParams.LaunchVelocity = Direction * GetPredictedArrowSpeed();
	PredictParams.ProjectileRadius = 5.0f;
	PredictParams.MaxSimTime = PredictedPathMaxSimTime;
	PredictParams.SimFrequency = PredictedPathSimFrequency;
	PredictParams.OverrideGravityZ = GetWorld()->GetGravityZ() * 0.9f;
	PredictParams.bTraceWithCollision = true;
	PredictParams.TraceChannel = ArrowTraceChannel;
	PredictParams.ActorsToIgnore.Add(OwnerCharacter);
	PredictParams.ActorsToIgnore.Add(const_cast<ABowBase*>(this));
	if (DrawArrow)
	{
		PredictParams.ActorsToIgnore.Add(DrawArrow);
	}

	FPredictProjectilePathResult PredictResult;
	const bool bDidHit = UGameplayStatics::PredictProjectilePath(this, PredictParams, PredictResult);

	if (PredictResult.PathData.Num() < 2)
	{
		return;
	}

	for (int32 Index = 1; Index < PredictResult.PathData.Num(); ++Index)
	{
		DrawDebugLine(
			GetWorld(),
			PredictResult.PathData[Index - 1].Location,
			PredictResult.PathData[Index].Location,
			PredictedPathDebugColor.ToFColor(true),
			false,
			0.0f,
			0,
			PredictedPathDebugThickness);
	}

	if (bDidHit && PredictResult.HitResult.bBlockingHit)
	{
		DrawDebugSphere(
			GetWorld(),
			PredictResult.HitResult.ImpactPoint,
			20.0f,
			12,
			PredictedPathHitDebugColor.ToFColor(true),
			false,
			0.0f,
			0,
			2.0f);
	}
}

float ABowBase::GetCurrentDrawAlpha() const
{
	if (OwnerCharacter && !OwnerCharacter->IsLocallyControlled())
	{
		return ReplicatedDrawAlpha;
	}

	return CalculateCurrentDrawAlphaLocal();
}

bool ABowBase::GetArrowLaunchData(FVector& OutSocketLocation, FVector& OutDirection, FVector* OutTargetPoint) const
{
	OutSocketLocation = FVector::ZeroVector;
	OutDirection = FVector::ZeroVector;
	if (OutTargetPoint)
	{
		*OutTargetPoint = FVector::ZeroVector;
	}

	if (!OwnerCharacter || !SKEquipmentMesh || !GetWorld())
	{
		return false;
	}

	APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController());
	if (!PC)
	{
		return false;
	}

	FVector CameraLocation = FVector::ZeroVector;
	FRotator CameraRotation = FRotator::ZeroRotator;
	PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

	const FVector CameraForward = CameraRotation.Vector();
	OutSocketLocation = SKEquipmentMesh->GetSocketLocation(ArrowSpawnSocket);

	const FVector TraceEnd = CameraLocation + CameraForward * 100000.f;
	FVector TargetPoint = TraceEnd;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);
	Params.AddIgnoredActor(this);
	if (DrawArrow)
	{
		Params.AddIgnoredActor(DrawArrow);
	}

	if (GetWorld()->LineTraceSingleByChannel(Hit, CameraLocation, TraceEnd, ArrowTraceChannel, Params))
	{
		const float SocketDistanceAlongCamera = FVector::DotProduct(OutSocketLocation - CameraLocation, CameraForward);
		const float HitDistanceAlongCamera = FVector::DotProduct(Hit.ImpactPoint - CameraLocation, CameraForward);
		const bool bHitIsInFrontOfArrowSocket = HitDistanceAlongCamera > SocketDistanceAlongCamera + 10.f;

		if (bHitIsInFrontOfArrowSocket)
		{
			TargetPoint = Hit.ImpactPoint;
		}
	}

	if (OutTargetPoint)
	{
		*OutTargetPoint = TargetPoint;
	}

	OutDirection = (TargetPoint - OutSocketLocation).GetSafeNormal();
	return !OutDirection.IsNearlyZero();
}

float ABowBase::GetPredictedArrowSpeed() const
{
	if (bPendingArrowFire && PendingArrowSpeed > KINDA_SMALL_NUMBER)
	{
		return PendingArrowSpeed;
	}

	if (bIsDrawing)
	{
		float HeldDuration = 0.f;
		float ChargeAlpha = 0.f;
		return CalculateChargedArrowSpeed(HeldDuration, ChargeAlpha);
	}

	return FMath::Max(MinArrowSpeed, 0.f);
}

bool ABowBase::HasArrowAmmo() const
{
	if (!OwnerCharacter)
	{
		return false;
	}

	AVL_PlayerState* PS = OwnerCharacter->GetPlayerState<AVL_PlayerState>();
	UVL_InventoryComponent* InvComp = PS ? PS->GetInventoryComponent() : nullptr;
	return InvComp && InvComp->HasEnoughItemIncludingQuickSlots(RequiredArrowItemID, 1);
}

float ABowBase::CalculateCurrentDrawAlphaLocal() const
{
	if (bPendingArrowFire)
	{
		if (FullChargeTime <= KINDA_SMALL_NUMBER)
		{
			return 1.f;
		}

		const float LinearChargeAlpha = FMath::Clamp(CurrentDrawDuration / FullChargeTime, 0.f, 1.f);
		return FMath::InterpEaseOut(0.f, 1.f, LinearChargeAlpha, ChargeEaseOutExponent);
	}

	if (bIsZooming && !bIsDrawing)
	{
		const bool bReleaseMontagePlaying =
			CurrentMontage && basicAttackMontages.Contains(CurrentMontage);

		if (bReleaseMontagePlaying && !bPendingArrowFire)
		{
			return 0.f;
		}

		return FMath::Clamp(ZoomPreviewDrawAlpha, 0.f, 1.f);
	}

	float HeldDuration = 0.f;
	float ChargeAlpha = 0.f;
	CalculateChargedArrowSpeed(HeldDuration, ChargeAlpha);
	return bIsDrawing ? ChargeAlpha : 0.f;
}

void ABowBase::EnsureDrawArrowAttached()
{
	// 서버 권한이 아니고, 이 활의 오너도 아닌 클라는 프리뷰 화살을 만들면 안 됨.
	// 그런 클라는 서버가 복제해준 화살만 봐야 함.
	if (!HasAuthority() && (!OwnerCharacter || !OwnerCharacter->IsLocallyControlled()))
	{
		return;
	}

	if (DrawArrow || !ArrowClass || !SKEquipmentMesh || !GetWorld())
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwnerCharacter;
	SpawnParams.Instigator = OwnerCharacter ? OwnerCharacter->GetInstigator() : nullptr;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	DrawArrow = GetWorld()->SpawnActor<AArrowProjectile>(ArrowClass, FTransform::Identity, SpawnParams);
	if (!DrawArrow)
	{
		return;
	}

	DrawArrow->SetReplicates(HasAuthority());
	DrawArrow->SetReplicateMovement(HasAuthority());
	DrawArrow->SetHidePreviewFromOwner(HasAuthority() && OwnerCharacter && !OwnerCharacter->IsLocallyControlled());

	UpdatePreviewArrowTransform();
	DrawArrow->ForceNetUpdate();
}

bool ABowBase::ShouldHoldBowString() const
{
	const bool bReleaseMontagePlaying =
		CurrentMontage && basicAttackMontages.Contains(CurrentMontage) && !bIsDrawing;

	// 발사 노티파이 전까지는 당긴 상태 유지
	if (bIsDrawing || bPendingArrowFire)
	{
		return true;
	}

	// 발사 몽타주 중, 화살 나간 뒤에는 줌 유지 중이어도 시위 복귀
	if (bReleaseMontagePlaying)
	{
		return false;
	}

	// 평상시 우클릭 줌 상태는 시위/화살 붙는 게 맞음
	return bIsZooming;
}

float ABowBase::CalculateChargedArrowSpeed(float& OutHeldDuration, float& OutChargeAlpha) const
{
	OutHeldDuration = 0.f;
	OutChargeAlpha = 0.f;

	const UWorld* World = GetWorld();
	if (!World)
	{
		return ArrowSpeed;
	}

	if (DrawStartTime > 0.0)
	{
		OutHeldDuration = FMath::Max(0.f, static_cast<float>(World->GetTimeSeconds() - DrawStartTime));
	}

	if (FullChargeTime <= KINDA_SMALL_NUMBER)
	{
		return ArrowSpeed;
	}

	const float LinearChargeAlpha = FMath::Clamp(OutHeldDuration / FullChargeTime, 0.f, 1.f);
	OutChargeAlpha = FMath::InterpEaseOut(0.f, 1.f, LinearChargeAlpha, ChargeEaseOutExponent);

	const float LowSpeed = FMath::Min(MinArrowSpeed, ArrowSpeed);
	const float HighSpeed = FMath::Max(MinArrowSpeed, ArrowSpeed);
	return FMath::Lerp(LowSpeed, HighSpeed, OutChargeAlpha);
}

void ABowBase::FireArrow(float InArrowSpeed)
{
	if (!DrawArrow || !OwnerCharacter)
	{
		return;
	}

	if (!HasAuthority())
	{
		DestroyDrawArrow();
		PendingArrowLaunchDirection = FVector::ZeroVector;
		PendingArrowTargetPoint = FVector::ZeroVector;
		return;
	}

	UVL_InventoryComponent* InvComp = nullptr;
	AVL_PlayerState* PS = OwnerCharacter ? OwnerCharacter->GetPlayerState<AVL_PlayerState>() : nullptr;
	InvComp = PS ? PS->GetInventoryComponent() : nullptr;

	if (!InvComp || !InvComp->HasEnoughItemIncludingQuickSlots(RequiredArrowItemID, 1))
	{
		UE_LOG(LogTemp, Warning, TEXT("[FireArrow] 화살 부족 - 발사 취소 | ItemID=%s"), *RequiredArrowItemID.ToString());
		return;
	}

	FVector SocketLocation = DrawArrow->GetActorLocation();
	FVector Direction = PendingArrowLaunchDirection.GetSafeNormal();
	const FVector TargetPoint = PendingArrowTargetPoint;
	PendingArrowLaunchDirection = FVector::ZeroVector;
	PendingArrowTargetPoint = FVector::ZeroVector;

	if (SKEquipmentMesh && SKEquipmentMesh->DoesSocketExist(ArrowSpawnSocket))
	{
		SocketLocation = SKEquipmentMesh->GetSocketLocation(ArrowSpawnSocket);
	}

	if (!TargetPoint.IsNearlyZero())
	{
		Direction = (TargetPoint - SocketLocation).GetSafeNormal();
	}

	if (Direction.IsNearlyZero() && !GetArrowLaunchData(SocketLocation, Direction))
	{
		return;
	}

	DrawArrow->SetActorLocationAndRotation(SocketLocation, Direction.Rotation(), false, nullptr, ETeleportType::TeleportPhysics);
	DrawArrow->Damage = GetCurrentActionDamage();

	MulticastStopBowDrawSound();

	if (DrawArrow->ArrowRoot)
	{
		DrawArrow->ArrowRoot->IgnoreActorWhenMoving(this, true);
	}

	if (SKEquipmentMesh)
	{
		SKEquipmentMesh->IgnoreActorWhenMoving(DrawArrow, true);
	}

	DrawArrow->Fire(Direction, InArrowSpeed);

	if (BowReleaseSoundCue)
	{
		MulticastPlayBowSound(BowReleaseSoundCue, GetBowSoundLocation(), BowReleaseSoundVolumeMultiplier, BowReleaseSoundPitchMultiplier);
	}

	if (InvComp)
	{
		InvComp->RemoveItemPreferredQuickSlots(RequiredArrowItemID, 1);
		UE_LOG(LogTemp, Warning, TEXT("[FireArrow] 화살 1개 차감 | ItemID=%s"), *RequiredArrowItemID.ToString());
	}

	DrawArrow = nullptr;
}

FVector ABowBase::GetBowSoundLocation() const
{
	if (SKEquipmentMesh)
	{
		if (!ArrowSpawnSocket.IsNone() && SKEquipmentMesh->DoesSocketExist(ArrowSpawnSocket))
		{
			return SKEquipmentMesh->GetSocketLocation(ArrowSpawnSocket);
		}

		return SKEquipmentMesh->GetComponentLocation();
	}

	return GetActorLocation();
}

void ABowBase::MulticastPlayBowSound_Implementation(USoundBase* Sound, FVector Location, float VolumeMultiplier, float PitchMultiplier)
{
	if (!Sound)
	{
		return;
	}

	UAudioComponent* AudioComponent = UGameplayStatics::SpawnSoundAtLocation(
		this,
		Sound,
		Location,
		FRotator::ZeroRotator,
		VolumeMultiplier,
		PitchMultiplier,
		0.f,
		SoundAttenuationSettings,
		nullptr,
		true);

	if (!AudioComponent)
	{
		return;
	}

	const float FadeOutTime = BowReleaseSoundFadeOutTime;
	const float SoundDuration = Sound->GetDuration();
	const float FadeOutDelay = FMath::IsFinite(SoundDuration) && SoundDuration > FadeOutTime && SoundDuration < 1000.f
		? SoundDuration - FadeOutTime
		: 0.f;

	TWeakObjectPtr<UAudioComponent> WeakAudioComponent(AudioComponent);
	FTimerDelegate FadeOutDelegate;
	FadeOutDelegate.BindLambda([WeakAudioComponent, FadeOutTime]()
	{
		if (UAudioComponent* Audio = WeakAudioComponent.Get())
		{
			Audio->FadeOut(FadeOutTime, 0.f);
		}
	});

	FTimerHandle FadeOutTimerHandle;
	GetWorldTimerManager().SetTimer(FadeOutTimerHandle, FadeOutDelegate, FadeOutDelay, false);
}

void ABowBase::MulticastStartBowDrawSound_Implementation()
{
	StartBowDrawSoundLocal();
}

void ABowBase::MulticastStopBowDrawSound_Implementation()
{
	StopBowDrawSoundLocal();
}

void ABowBase::StartBowDrawSoundLocal()
{
	if (!BowDrawSoundAudioComponent || !BowDrawSoundCue)
	{
		return;
	}

	if (BowDrawSoundAudioComponent->IsPlaying())
	{
		BowDrawSoundAudioComponent->Stop();
	}

	BowDrawSoundAudioComponent->SetSound(BowDrawSoundCue);
	BowDrawSoundAudioComponent->SetPitchMultiplier(BowDrawSoundPitchMultiplier);
	BowDrawSoundAudioComponent->AttenuationSettings = SoundAttenuationSettings;
	BowDrawSoundAudioComponent->FadeIn(BowDrawSoundFadeInTime, BowDrawSoundVolumeMultiplier, 0.f);

}

void ABowBase::StopBowDrawSoundLocal()
{
	if (!BowDrawSoundAudioComponent)
	{
		return;
	}

	if (BowDrawSoundAudioComponent->IsPlaying())
	{
		BowDrawSoundAudioComponent->FadeOut(BowDrawSoundFadeOutTime, 0.f);
	}
}

void ABowBase::DestroyDrawArrow()
{
	if (DrawArrow)
	{
		DrawArrow->Destroy();
		DrawArrow = nullptr;
	}
}

void ABowBase::OnInterruptedMontageEnd()
{
	CancelAction();
}
