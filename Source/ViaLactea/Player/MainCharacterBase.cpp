// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacterBase.h"

#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Weapon/CharacterEquipmentBase.h"
#include "Weapon/CharacterWeaponBase.h"
#include "Weapon/BowBase.h"
#include "Weapon/ShieldBase.h"
#include "Base/Character/VL_AICharacterBase.h"
#include "CustomLog/CustomLog.h"
#include "Base/Data/VL_DataRows.h"

//0424김한별
#include "Monster/Boss/VL_Boss1.h"
#include "Base/Component/VL_AggroComponent.h"

#include "Animation/AnimInstance.h"
#include "Components/InputComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"

#include "Base/Actor/VL_InteractActorBase.h"
#include "Base/Component/VL_StatComponent.h"

#include "HAL/PlatformTime.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/PlayerController.h"

#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

#include "Net/UnrealNetwork.h"
#include "Player/VL_PlayerController.h"

namespace
{
	const TCHAR* GetRoleText(const AActor* Actor)
	{
		if (!Actor)
		{
			return TEXT("None");
		}

		switch (Actor->GetLocalRole())
		{
		case ROLE_Authority: return TEXT("Authority");
		case ROLE_AutonomousProxy: return TEXT("AutonomousProxy");
		case ROLE_SimulatedProxy: return TEXT("SimulatedProxy");
		default: return TEXT("None");
		}
	}

	const TCHAR* GetFacingSourceText(EFacingSource Source)
	{
		switch (Source)
		{
		case EFacingSource::None: return TEXT("None");
		case EFacingSource::ControllerRotation: return TEXT("ControllerRotation");
		case EFacingSource::InputDirection: return TEXT("InputDirection");
		case EFacingSource::RollDirection: return TEXT("RollDirection");
		case EFacingSource::RollRecovery: return TEXT("RollRecovery");
		default: return TEXT("Unknown");
		}
	}
}

AMainCharacterBase::AMainCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	//KDH 추가. 스탯
	StatComponent = CreateDefaultSubobject<UVL_StatComponent>(TEXT("StatComponent"));

	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -87.f));
	GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	GetMesh()->bEnableUpdateRateOptimizations = false;
	GetMesh()->SetComponentTickInterval(0.f);

	VisibleMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("VisibleMesh"));
	VisibleMesh->SetupAttachment(GetMesh());

	SetNetUpdateFrequency(100.f);
	SetMinNetUpdateFrequency(33.f);

	//스프링암 컴포넌트
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);

	// 스프링암의 카메라 렉을 사용할수 있다. 천천히 따라가기 
	SpringArm->bEnableCameraRotationLag = true;
	// 피치 야 롤
	SpringArm->SetRelativeRotation(FRotator(-30.f, 0.f, 0.f));

	//카메라 컴포넌트 
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);


	SpringArm->bUsePawnControlRotation = true;
	SpringArm->bInheritRoll = false;
	SpringArm->bEnableCameraLag = true;
	SpringArm->ProbeChannel = ECC_Visibility; // 캐릭터/몬스터 캡슐을 무시하고 지형만 감지
	SpringArm->bEnableCameraRotationLag = true;
	SpringArm->CameraLagSpeed = 10.f;
	SpringArm->CameraRotationLagSpeed = 10.f;
	SpringArm->SetRelativeLocation(FVector::ZeroVector);
	CurrentCameraSocketOffset = DefaultCameraSocketOffset;
	TargetCameraSocketOffset = DefaultCameraSocketOffset;
	SpringArm->SocketOffset = CurrentCameraSocketOffset;
	TargetArmLength = SpringArm->TargetArmLength;
	Camera->SetRelativeLocation(FVector::ZeroVector);

	bUseControllerRotationYaw = false;
	

	UCharacterMovementComponent* CharMovement = GetCharacterMovement();
	CharMovement->MaxAcceleration = 800.f;
	CharMovement->BrakingFrictionFactor = 1.f;
	CharMovement->bUseSeparateBrakingFriction = true;
	CharMovement->GroundFriction = 5.f;
	CharMovement->MaxWalkSpeed = 200.f;
	CharMovement->MinAnalogWalkSpeed = 150.f;
	CharMovement->BrakingDecelerationWalking = 2000.f;
	CharMovement->bCanWalkOffLedgesWhenCrouching = true;
	CharMovement->bUseFlatBaseForFloorChecks = true;
	CharMovement->RotationRate = FRotator(0, 200.f, 0);
	CharMovement->bOrientRotationToMovement = true;

	bCanMove = true;
}

void AMainCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMainCharacterBase, Gait);
	DOREPLIFETIME(AMainCharacterBase, RotationMode);
	DOREPLIFETIME(AMainCharacterBase, RightWeapon);
	DOREPLIFETIME(AMainCharacterBase, LeftWeapon);
	DOREPLIFETIME(AMainCharacterBase, bIsWeaponEquip);
	DOREPLIFETIME(AMainCharacterBase, CurrentWeaponType);
	DOREPLIFETIME(AMainCharacterBase, bIsAiming);
	DOREPLIFETIME(AMainCharacterBase, ReplicatedAimYaw);
	DOREPLIFETIME(AMainCharacterBase, bIsDead);
}

float AMainCharacterBase::GetCurrentHP() const
{
	return StatComponent ? StatComponent->GetCurrentHP() : 0.f;
}

float AMainCharacterBase::GetMaxHP() const
{
	return StatComponent ? StatComponent->GetMaxHP() : 0.f;
}

float AMainCharacterBase::GetHPRatio() const
{
	return StatComponent ? StatComponent->GetHPRatio() : 0.f;
}

float AMainCharacterBase::GetStaminaRatio() const
{
	return StatComponent ? StatComponent->GetStaminaRatio() : 0.f;
}

void AMainCharacterBase::ServerStartPrimaryAction_Implementation()
{
	if (!CanDoAction() && BufferServerPrimaryActionRequest())
	{
		return;
	}

	StartPrimaryAction();
}

void AMainCharacterBase::ServerStartSecondaryAction_Implementation()
{
	StartSecondaryAction();
}

ABowBase* AMainCharacterBase::GetEquippedBow() const
{
	return Cast<ABowBase>(RightWeapon);
}

void AMainCharacterBase::ServerConsumeJumpStamina_Implementation()
{
	if (!StatComponent || bIsDead)
	{
		return;
	}

	StatComponent->ConsumeStamina(JumpStaminaCost);
}

float AMainCharacterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (!HasAuthority() || ActualDamage <= 0.f || bIsDead || !StatComponent || StatComponent->IsDead())
	{
		return ActualDamage;
	}

	if (bIsInvincible)
	{
		UE_LOG(LogTemp, Log, TEXT("[PlayerDamage] Character=%s ignored %.2f damage by invincible window"),
			*GetNameSafe(this),
			ActualDamage);
		return 0.f;
	}

	if (bCanParry)
	{
		const float ConsumedStamina = ConsumeDefenseStaminaFromDamage(ActualDamage, ParryStaminaDamageMultiplier, TEXT("Parry"));
		ApplyParryGroggyToAttacker(EventInstigator, DamageCauser);

		UE_LOG(LogTemp, Log, TEXT("[PlayerDamage] Character=%s parried %.2f damage and consumed %.2f stamina"),
			*GetNameSafe(this),
			ActualDamage,
			ConsumedStamina);
		return 0.f;
	}

	if (bIsBlocking)
	{
		const float ConsumedStamina = ConsumeDefenseStaminaFromDamage(ActualDamage, BlockStaminaDamageMultiplier, TEXT("Block"));

		UE_LOG(LogTemp, Log, TEXT("[PlayerDamage] Character=%s blocked %.2f damage and consumed %.2f stamina"),
			*GetNameSafe(this),
			ActualDamage,
			ConsumedStamina);
		return 0.f;
	}

	const float AppliedDamage = StatComponent->ApplyDamage(ActualDamage);

	UE_LOG(LogTemp, Log, TEXT("[PlayerDamage] Character=%s Damage=%.2f HP=%.2f/%.2f"),
		*GetNameSafe(this),
		AppliedDamage,
		StatComponent->GetCurrentHP(),
		StatComponent->GetMaxHP());

	if (!StatComponent->IsDead())
	{
		PlayHitReaction();
	}

	return AppliedDamage;
}

float AMainCharacterBase::ConsumeDefenseStaminaFromDamage(float IncomingDamage, float DamageMultiplier, const TCHAR* DefenseLabel)
{
	if (!StatComponent || IncomingDamage <= 0.f || DamageMultiplier <= 0.f)
	{
		return 0.f;
	}

	const float PreviousStamina = StatComponent->GetCurrentStamina();
	const float StaminaDamage = IncomingDamage * DamageMultiplier;
	StatComponent->ConsumeStaminaTick(StaminaDamage);

	const float ConsumedStamina = PreviousStamina - StatComponent->GetCurrentStamina();

	UE_LOG(LogTemp, Log, TEXT("[PlayerDamage] Character=%s %s stamina damage %.2f (Consumed=%.2f, Stamina=%.2f/%.2f)"),
		*GetNameSafe(this),
		DefenseLabel ? DefenseLabel : TEXT("Defense"),
		StaminaDamage,
		ConsumedStamina,
		StatComponent->GetCurrentStamina(),
		StatComponent->GetMaxStamina());

	return ConsumedStamina;
}

float AMainCharacterBase::PlayMontageLocally(UAnimMontage* Montage, float PlayRate, FName StartSection, bool bFreezeAtEnd, bool bStopAllMontages)
{
	if (!Montage)
	{
		return 0.f;
	}

	UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInstance)
	{
		return 0.f;
	}

	const float Duration = AnimInstance->Montage_Play(
		Montage,
		PlayRate,
		EMontagePlayReturnType::MontageLength,
		0.f,
		bStopAllMontages);

	if (Duration > 0.f && StartSection != NAME_None)
	{
		AnimInstance->Montage_JumpToSection(StartSection, Montage);
	}

	if (!bFreezeAtEnd || Duration <= 0.f || !GetMesh())
	{
		return Duration;
	}

	if (FAnimMontageInstance* MontageInstance = AnimInstance->GetActiveInstanceForMontage(Montage))
	{
		// Hold on the final frame until an explicit stop arrives.
		MontageInstance->bEnableAutoBlendOut = false;
	}

	return Duration;
}

void AMainCharacterBase::StopMontageLocally(UAnimMontage* Montage, float BlendOutTime)
{
	if (!Montage || !GetMesh())
	{
		return;
	}

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->Montage_Stop(BlendOutTime, Montage);
	}
}

void AMainCharacterBase::RequestMontageHitStop(UAnimMontage* Montage, float Duration, float PausedPlayRate)
{
	if (!Montage || Duration <= 0.f)
	{
		return;
	}

	if (HasAuthority())
	{
		MulticastApplyMontageHitStop(Montage, Duration, PausedPlayRate);
		return;
	}

	ApplyLocalMontageHitStop(Montage, Duration, PausedPlayRate);
}

void AMainCharacterBase::ApplyLocalMontageHitStop(UAnimMontage* Montage, float Duration, float PausedPlayRate)
{
	if (!Montage || Duration <= 0.f || !GetMesh())
	{
		return;
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance)
	{
		return;
	}

	if (ActiveHitStopMontage && ActiveHitStopMontage != Montage)
	{
		RestoreMontageHitStop();
		AnimInstance = GetMesh()->GetAnimInstance();
		if (!AnimInstance)
		{
			return;
		}
	}

	FAnimMontageInstance* MontageInstance = AnimInstance->GetActiveInstanceForMontage(Montage);
	if (!MontageInstance)
	{
		return;
	}

	if (ActiveHitStopMontage != Montage)
	{
		StoredHitStopPlayRate = MontageInstance->GetPlayRate();
		ActiveHitStopMontage = Montage;
	}

	constexpr float MinHitStopPlayRate = 0.05f;
	const float EffectiveHitStopPlayRate = PausedPlayRate <= KINDA_SMALL_NUMBER
		? MinHitStopPlayRate
		: PausedPlayRate;

	if (bHitStopMontagePaused)
	{
		AnimInstance->Montage_Resume(Montage);
	}

	AnimInstance->Montage_SetPlayRate(Montage, EffectiveHitStopPlayRate);

	bHitStopMontagePaused = false;

	GetWorld()->GetTimerManager().ClearTimer(MontageHitStopTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(
		MontageHitStopTimerHandle,
		this,
		&AMainCharacterBase::RestoreMontageHitStop,
		Duration,
		false);
}

void AMainCharacterBase::RestoreMontageHitStop()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(MontageHitStopTimerHandle);
	}

	UAnimMontage* MontageToRestore = ActiveHitStopMontage;
	const float PlayRateToRestore = StoredHitStopPlayRate;
	const bool bWasMontagePaused = bHitStopMontagePaused;

	ActiveHitStopMontage = nullptr;
	StoredHitStopPlayRate = 1.0f;
	bHitStopMontagePaused = false;

	if (!MontageToRestore || !GetMesh())
	{
		return;
	}

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		if (FAnimMontageInstance* MontageInstance = AnimInstance->GetActiveInstanceForMontage(MontageToRestore))
		{
			MontageInstance->SetPlayRate(PlayRateToRestore);

			if (bWasMontagePaused)
			{
				AnimInstance->Montage_Resume(MontageToRestore);
			}
		}
	}
}

bool AMainCharacterBase::IsCurrentAbilityMontageActive() const
{
	const ACharacterWeaponBase* RightHandWeapon = Cast<ACharacterWeaponBase>(RightWeapon);
	if (!RightHandWeapon || !RightHandWeapon->CurrentMontage)
	{
		return false;
	}

	return RightHandWeapon->IsAbilityMontage(RightHandWeapon->CurrentMontage);
}

void AMainCharacterBase::ReleaseAbilityRootMotionControl()
{
	if (bAbilityRootMotionReleased || !GetMesh())
	{
		return;
	}

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->SetRootMotionMode(ERootMotionMode::IgnoreRootMotion);
		bAbilityRootMotionReleased = true;
	}
}

void AMainCharacterBase::RestoreAbilityRootMotionControl()
{
	if (!bAbilityRootMotionReleased || !GetMesh())
	{
		bAbilityRootMotionReleased = false;
		return;
	}

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->SetRootMotionMode(ERootMotionMode::RootMotionFromMontagesOnly);
	}

	bAbilityRootMotionReleased = false;
}

float AMainCharacterBase::PlayNetworkedMontage(UAnimMontage* Montage, float PlayRate, FName StartSection, bool bFreezeAtEnd, bool bSkipOwnerIfLocallyControlled, bool bStopAllMontages)
{
	if (!Montage)
	{
		return 0.f;
	}

	if (HasAuthority())
	{
		MulticastPlayNetworkedMontage(Montage, PlayRate, StartSection, bFreezeAtEnd, false, bStopAllMontages);
		return Montage->GetPlayLength() / FMath::Max(PlayRate, KINDA_SMALL_NUMBER);
	}

	const float Duration = PlayMontageLocally(Montage, PlayRate, StartSection, bFreezeAtEnd, bStopAllMontages);
	ServerPlayNetworkedMontage(Montage, PlayRate, StartSection, bFreezeAtEnd, bSkipOwnerIfLocallyControlled, bStopAllMontages);
	return Duration;
}

float AMainCharacterBase::PlayPredictedWeaponMontageLocally(UAnimMontage* Montage, float PlayRate, FName StartSection, bool bFreezeAtEnd, bool bStopAllMontages)
{
	return PlayMontageLocally(Montage, PlayRate, StartSection, bFreezeAtEnd, bStopAllMontages);
}

void AMainCharacterBase::StopNetworkedMontage(UAnimMontage* Montage, float BlendOutTime)
{
	if (!Montage)
	{
		return;
	}

	if (HasAuthority())
	{
		MulticastStopNetworkedMontage(Montage, BlendOutTime);
		return;
	}

	StopMontageLocally(Montage, BlendOutTime);
	ServerStopNetworkedMontage(Montage, BlendOutTime);
}

void AMainCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	MovementComp = GetCharacterMovement();
	if (MovementComp)
	{
		MovementComp->ProcessRootMotionPreConvertToWorld.BindUObject(this, &AMainCharacterBase::ProcessRollRootMotionPreConvertToWorld);
	}

	if (HasAuthority())
	{
		bIsDead = false;
		bDeathStateApplied = false;
		bDeathRagdollStarted = false;

		if (StatComponent)
		{
			StatComponent->InitializeStats(InitialMaxHP, InitialMaxStamina, true);
		}
	}

	APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (IsValid(PlayerController))
	{
		UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
		if (IsValid(InputSubsystem) && DefaultMappingContext)
		{
			InputSubsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}


	bIsAiming = false;
	bCanMove = true;
	bCanAction = true;
	bCanRun = true;
	ActionState = ECharacterActionState::Idle;
	TargetControlRotation = Controller ? Controller->GetControlRotation() : FRotator::ZeroRotator;
	CurrentCameraSocketOffset = (RotationMode == ERotationMode::Strafe) ? StrafeCameraSocketOffset : DefaultCameraSocketOffset;
	TargetCameraSocketOffset = CurrentCameraSocketOffset;

	if (SpringArm)
	{
		SpringArm->SetRelativeLocation(FVector::ZeroVector);
		SpringArm->SocketOffset = CurrentCameraSocketOffset;
	}

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->OnMontageStarted.AddUniqueDynamic(this, &AMainCharacterBase::OnGlobalMontageStarted);
		AnimInstance->OnMontageEnded.AddUniqueDynamic(this, &AMainCharacterBase::OnGlobalMontageEnded);
	}

	if (StatComponent)
	{
		StatComponent->OnDeath.AddDynamic(this, &AMainCharacterBase::HandleStatDeath);
	}

	if (HasAuthority())
	{
		TArray<AActor*> OutActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AVL_AICharacterBase::StaticClass(), OutActors);

		for (AActor* Actor : OutActors)
		{
			if (AVL_Boss1* Boss = Cast<AVL_Boss1>(Actor))
			{
				Boss->GetAggroComponent()->RegisterInitialTarget(this);
			}
		}
	}

}

void AMainCharacterBase::Destroyed()
{
	if (HasAuthority())
	{
		CleanupOwnedEquipment(0.f);
	}

	Super::Destroyed();
}

// Called every frame
void AMainCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority())
	{
		ReplicatedAimYaw = FRotator::NormalizeAxis(GetBaseAimRotation().Yaw);
	}

	if (HasAuthority() && StatComponent && !bIsDead)
	{
		/*UE_LOG(LogTemp, Warning, TEXT("Gait = %d"), (int32)Gait);*/

		const bool bRunningGait = Gait == EGait::Run || Gait == EGait::Sprint;
		const bool bActuallyMoving = MovementComp
			&& MovementComp->IsMovingOnGround()
			&& MovementComp->Velocity.SizeSquared2D() > FMath::Square(10.f);

		if (bRunningGait && bActuallyMoving)
		{
			const float Cost = SprintStaminaCostPerSecond * DeltaTime;
			StatComponent->ConsumeStaminaTick(Cost);

			if (!StatComponent->CanSpendStamina(KINDA_SMALL_NUMBER))
			{
				if (Gait == EGait::Sprint)
				{
					StopSprint();
				}
				else
				{
					StopRun();
				}
			}
		}
		else
		{
			StatComponent->RecoverStamina(DeltaTime);
		}
	}

	if (bIsDead)
	{
		return;
	}

	RecoverActionWindowIfIdle();
	UpdateMovementParameters();
	CalcDirectionalValueFromVelocity();
	UpdateSmoothRotation(DeltaTime);
	UpdateLockOn(DeltaTime);
	UpdateCharacterFacing(DeltaTime);

	if (ActionState == ECharacterActionState::Rolling && RollDebugFrameCount > 0)
	{
		const FVector CurrentLocation = GetActorLocation();
		const FVector DeltaLocation = CurrentLocation - RollDebugLastLocation;
		const FVector Delta2D = FVector(DeltaLocation.X, DeltaLocation.Y, 0.f);
		const FVector ActorForward = GetActorForwardVector().GetSafeNormal2D();
		const FVector ActorRight = GetActorRightVector().GetSafeNormal2D();
		const FRotator ControlRot = Controller ? Controller->GetControlRotation() : FRotator::ZeroRotator;
		const FVector ControlForward = FRotationMatrix(FRotator(0.f, ControlRot.Yaw, 0.f)).GetUnitAxis(EAxis::X).GetSafeNormal2D();
		const FVector TargetForward = FRotationMatrix(FRotator(0.f, TargetRotation.Yaw, 0.f)).GetUnitAxis(EAxis::X).GetSafeNormal2D();

		UE_LOG(LogTemp, Warning, TEXT("[RollDebug][MoveFrame] Char=%s Role=%s Frame=%d ActorYaw=%.2f TargetYaw=%.2f ControlYaw=%.2f Delta=(%.2f, %.2f, %.2f) Dist2D=%.2f DotActorF=%.2f DotActorR=%.2f DotControlF=%.2f DotTargetF=%.2f"),
			*GetNameSafe(this),
			GetRoleText(this),
			RollDebugFrameCount,
			GetActorRotation().Yaw,
			TargetRotation.Yaw,
			ControlRot.Yaw,
			DeltaLocation.X,
			DeltaLocation.Y,
			DeltaLocation.Z,
			Delta2D.Size(),
			FVector::DotProduct(Delta2D, ActorForward),
			FVector::DotProduct(Delta2D, ActorRight),
			FVector::DotProduct(Delta2D, ControlForward),
			FVector::DotProduct(Delta2D, TargetForward));

		RollDebugLastLocation = CurrentLocation;
		--RollDebugFrameCount;
	}

	UpdateAimOffsetValues();
	UpdateCameraSocketOffset(DeltaTime);
	UpdateWeaponIKData();

}

// Lifecycle and engine callbacks
void AMainCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = Cast< UEnhancedInputComponent>(PlayerInputComponent);

	if (EnhancedInputComponent)
	{
		//점프 입력 셋팅 
		if (IsValid(JumpAction))
		{
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AMainCharacterBase::OnJumpPressed);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AMainCharacterBase::OnJumpReleased);
		}

		if (IsValid(RotationAction))
		{
			EnhancedInputComponent->BindAction(RotationAction, ETriggerEvent::Triggered, this, &AMainCharacterBase::Rotation);
			EnhancedInputComponent->BindAction(RotationAction, ETriggerEvent::Completed, this, &AMainCharacterBase::Rotation);
			EnhancedInputComponent->BindAction(RotationAction, ETriggerEvent::Canceled, this, &AMainCharacterBase::Rotation);
		}
		if (IsValid(LockOnAction))
		{
			EnhancedInputComponent->BindAction(LockOnAction, ETriggerEvent::Started, this, &AMainCharacterBase::OnLockOnPressed);
		}

		if (IsValid(MoveAction))
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMainCharacterBase::Move);
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &AMainCharacterBase::ClearMoveInput);
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Canceled, this, &AMainCharacterBase::ClearMoveInput);
		}
		if (IsValid(RunAction))
		{
			EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Started, this, &AMainCharacterBase::OnRunPressed);
			EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Completed, this, &AMainCharacterBase::OnRunReleased);
		}
		if (IsValid(SprintAction))
		{
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &AMainCharacterBase::StartSprint);
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AMainCharacterBase::StopSprint);
		}
		if (IsValid(LeftClick))
		{
			EnhancedInputComponent->BindAction(LeftClick, ETriggerEvent::Started, this, &AMainCharacterBase::OnPrimaryPressed);
			EnhancedInputComponent->BindAction(LeftClick, ETriggerEvent::Completed, this, &AMainCharacterBase::OnPrimaryReleased);
		}
		if (IsValid(RightClick))
		{
			EnhancedInputComponent->BindAction(RightClick, ETriggerEvent::Started, this, &AMainCharacterBase::OnSecondaryPressed);
			EnhancedInputComponent->BindAction(RightClick, ETriggerEvent::Completed, this, &AMainCharacterBase::OnSecondaryReleased);
		}
		if (IsValid(AbilityAction))
		{
			EnhancedInputComponent->BindAction(AbilityAction, ETriggerEvent::Started, this, &AMainCharacterBase::OnAbilityPressed);
			EnhancedInputComponent->BindAction(AbilityAction, ETriggerEvent::Completed, this, &AMainCharacterBase::OnAbilityReleased);
		}
		if (IsValid(ChangeRotationModeAction))
		{
			EnhancedInputComponent->BindAction(ChangeRotationModeAction, ETriggerEvent::Started, this, &AMainCharacterBase::ChangeRotationMode);
		}
		if (IsValid(CrouchAction))
		{
			EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AMainCharacterBase::Crouching);
		}
		if (IsValid(RollAction))
		{
			EnhancedInputComponent->BindAction(RollAction, ETriggerEvent::Started, this, &AMainCharacterBase::OnRollPressed);
			EnhancedInputComponent->BindAction(RollAction, ETriggerEvent::Completed, this, &AMainCharacterBase::OnRollReleased);
		}
		if (IsValid(InputActionInteraction))
		{
			EnhancedInputComponent->BindAction(InputActionInteraction, ETriggerEvent::Started, this, &AMainCharacterBase::Interaction);
		}
		if (IsValid(CameraZoomAction))
		{
			EnhancedInputComponent->BindAction(CameraZoomAction, ETriggerEvent::Triggered, this, &AMainCharacterBase::OnCameraZoom);
		}
	}
}

void AMainCharacterBase::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	const bool bIsCurrentlyFalling = GetCharacterMovement() && GetCharacterMovement()->IsFalling();

	// 낙하 시작 -> InAir 세팅 (절벽에서 걸어서 떨어지는 경우 포함)
	if (bIsCurrentlyFalling)
	{
		if (ActionState != ECharacterActionState::InAir)
		{
			CancelEquippedActions();
			SetCharacterState(ECharacterActionState::InAir);
		}
	}
	else if (PrevMovementMode == MOVE_Falling)
	{
		RestorePostAirborneState();
	}
}

void AMainCharacterBase::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	LandingVelocity = MovementComp->Velocity;
	bJustLanded = true;
	RestorePostAirborneState();

	GetWorld()->GetTimerManager().ClearTimer(RetriggerableDelayHandle);

	TWeakObjectPtr<AMainCharacterBase> WeakThis(this);

	GetWorld()->GetTimerManager().SetTimer(
		RetriggerableDelayHandle,
		FTimerDelegate::CreateLambda([WeakThis]()
			{
				if (!WeakThis.IsValid()) return;
				WeakThis->bJustLanded = false;
			}),
		0.3f,
		false
	);
}

// Equipment
void AMainCharacterBase::BindEquipmentDelegates(ACharacterEquipmentBase* Equipment)
{
	if (!Equipment) return;

	if (ABowBase* BowEquipment = Cast<ABowBase>(Equipment))
	{
		BowEquipment->OnZoomChanged.RemoveAll(this);
		BowEquipment->OnZoomChanged.AddUObject(this, &AMainCharacterBase::OnBowZoomChanged);
	}

	if (AShieldBase* ShieldEquipment = Cast<AShieldBase>(Equipment))
	{
		ShieldEquipment->OnBlockChanged.RemoveAll(this);
		ShieldEquipment->OnBlockChanged.AddUObject(this, &AMainCharacterBase::OnShieldBlockChanged);
		ShieldEquipment->OnGuardChanged.RemoveAll(this);
		ShieldEquipment->OnGuardChanged.AddUObject(this, &AMainCharacterBase::OnShieldGuardChanged);
	}
}


ACharacterEquipmentBase* AMainCharacterBase::GetOrCreateEquipment(TSubclassOf<ACharacterEquipmentBase> EquipmentClass)
{
	if (!HasAuthority() || !EquipmentClass || !GetWorld())
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;

	ACharacterEquipmentBase* SpawnedEquipment =
		GetWorld()->SpawnActor<ACharacterEquipmentBase>(EquipmentClass, GetActorTransform(), SpawnParams);

	if (!SpawnedEquipment)
	{
		return nullptr;
	}

	SpawnedEquipment->SetReplicates(true);
	SpawnedEquipment->SetReplicateMovement(false);
	SpawnedEquipment->bNetUseOwnerRelevancy = true;

	SpawnedEquipment->SetActorHiddenInGame(true);
	SpawnedEquipment->SetActorEnableCollision(false);
	return SpawnedEquipment;
}

void AMainCharacterBase::FinalizePendingEquipEquipment()
{
	if (!PendingEquipEquipment)
	{
		return;
	}

	PendingEquipEquipment->AttachEquipmentToOwner();
	PendingEquipEquipment->ShowEquipment();
	PendingEquipEquipment = nullptr;
	UpdateEquipmentStateFromWeapons();
}

void AMainCharacterBase::FinalizePendingUnequipEquipment()
{
	ACharacterEquipmentBase* Equipment = PendingUnequipEquipment;
	if (!Equipment)
	{
		return;
	}

	if (RightWeapon == Equipment)
	{
		RightWeapon = nullptr;
	}
	else if (LeftWeapon == Equipment)
	{
		LeftWeapon = nullptr;
	}

	PendingUnequipEquipment = nullptr;
	UpdateEquipmentStateFromWeapons();
	Equipment->CompleteUnequip();
}

void AMainCharacterBase::QueuePendingEquipmentItem(FName ItemID, TSubclassOf<ACharacterEquipmentBase> EquipmentClass)
{
	PendingEquipmentItemID = ItemID;
	PendingEquipmentClass = EquipmentClass;
}

bool AMainCharacterBase::ConsumePendingEquipmentItem()
{
	if (PendingEquipmentItemID.IsNone() || !PendingEquipmentClass)
	{
		return false;
	}

	const FName ItemID = PendingEquipmentItemID;
	TSubclassOf<ACharacterEquipmentBase> EquipmentClass = PendingEquipmentClass;
	PendingEquipmentItemID = NAME_None;
	PendingEquipmentClass = nullptr;

	return HandleEquipmentItem(ItemID, EquipmentClass);
}

void AMainCharacterBase::Unequipment(ACharacterEquipmentBase* Equipment)
{
	if (!Equipment)
	{
		return;
	}

	CancelEquippedActions();
	SetCanAction(false);

	PendingUnequipEquipment = Equipment;
	if (Equipment->OnUnequip())
	{
		// 몽타주 없음 → 즉시 완료
		FinalizePendingUnequipEquipment();
	}
}

void AMainCharacterBase::EquipItem(ACharacterEquipmentBase* NewEquipment)
{
	if (!NewEquipment)
	{
		return;
	}

	CancelEquippedActions();

	auto IsTwoHandedType = [](ACharacterEquipmentBase* Equip) {
		return Equip && (Equip->EquipmentType == EEquipmentSlotType::TwoHanded
		              || Equip->EquipmentType == EEquipmentSlotType::Bow);
	};

	switch (NewEquipment->EquipmentType)
	{
	case EEquipmentSlotType::TwoHanded:
	case EEquipmentSlotType::Bow:
		if (RightWeapon) RightWeapon->CompleteUnequip();
		if (LeftWeapon)  LeftWeapon->CompleteUnequip();
		RightWeapon = NewEquipment;
		LeftWeapon = nullptr;
		break;

	case EEquipmentSlotType::RightHand:
		if (IsTwoHandedType(RightWeapon))
			LeftWeapon = nullptr;
		if (RightWeapon) RightWeapon->CompleteUnequip();
		RightWeapon = NewEquipment;
		break;

	case EEquipmentSlotType::LeftHand:
	case EEquipmentSlotType::Armor:
	case EEquipmentSlotType::Shield:
		if (IsTwoHandedType(RightWeapon))
		{
			RightWeapon->CompleteUnequip();
			RightWeapon = nullptr;
		}
		if (LeftWeapon) LeftWeapon->CompleteUnequip();
		LeftWeapon = NewEquipment;
		break;

	default:
		return;
	}

	BindEquipmentDelegates(NewEquipment);
	NewEquipment->DetachEquipmentFromOwner();
	NewEquipment->HideEquipment();

	SetCanAction(false);
	NewEquipment->OnEquip(this);

	if (!NewEquipment->EquipMontage)
	{
		NewEquipment->AttachEquipmentToOwner();
		NewEquipment->ShowEquipment();
		UpdateEquipmentStateFromWeapons();
	}
	else
	{
		PendingEquipEquipment = NewEquipment;
	}
}

void AMainCharacterBase::OnRep_IsDead()
{
	if (bIsDead)
	{
		ApplyDeathState();
	}
}

void AMainCharacterBase::OnRep_RightWeapon()
{
	if (RightWeapon)
	{
		BindEquipmentDelegates(RightWeapon);
	}

	UpdateEquipmentStateFromWeapons();
}

void AMainCharacterBase::OnRep_LeftWeapon()
{
	if (LeftWeapon)
	{
		BindEquipmentDelegates(LeftWeapon);
	}

	UpdateEquipmentStateFromWeapons();
}

void AMainCharacterBase::ApplyDeathState()
{
	if (bDeathStateApplied)
	{
		return;
	}

	bDeathStateApplied = true;
	ExitLockOn();
	SetCharacterState(ECharacterActionState::Dead);
	ActiveMontageLockCount = 0;
	SetCanAction(false);
	bCanRun = false;
	bIsAiming = false;
	bIsGuarding = false;
	bIsBlocking = false;
	bCanParry = false;
	bIsInvincible = false;
	InvincibleWindowCount = 0;
	ParryWindowCount = 0;
	BlockWindowCount = 0;
	bCanMove = false;
	bSmoothRotating = false;
	FacingSource = EFacingSource::None;
	Gait = EGait::Walk;
	QueuedActionHandledMontage = nullptr;
	PendingUnequipEquipment = nullptr;
	PendingEquipEquipment = nullptr;
	PendingEquipmentItemID = NAME_None;
	PendingEquipmentClass = nullptr;
	ClearBufferedInputs();
	ClearMoveInput();

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(RetriggerableDelayHandle);
		GetWorld()->GetTimerManager().ClearTimer(RespawnTimerHandle);
	}

	RestoreMontageHitStop();
	RestoreAbilityRootMotionControl();
	StopJumping();

	if (MovementComp)
	{
		MovementComp->StopMovementImmediately();
		MovementComp->DisableMovement();
	}

	CancelEquippedActions();

	if (HasAuthority())
	{
		ScheduleRespawn();

		if (DeathMontage)
		{
			const float DeathMontageDuration = PlayNetworkedMontage(
				DeathMontage,
				DeathMontagePlayRate,
				NAME_None,
				false,
				false,
				true);

			if (DeathMontageDuration <= 0.f)
			{
				MulticastTriggerDeathRagdoll();
			}
		}
		else
		{
			MulticastTriggerDeathRagdoll();
		}
	}
}

void AMainCharacterBase::ClearBufferedInputs()
{
	auto ResetInputState = [](FBufferedInputState& InputState)
	{
		InputState.bPressed = false;
		InputState.bConsumed = false;
		InputState.BufferUntilTime = 0.0;
	};

	ResetInputState(PrimaryInputState);
	ResetInputState(AbilityInputState);
	ResetInputState(SecondaryInputState);
	ResetInputState(RunInputState);
	ResetInputState(JumpInputState);
	ResetInputState(RollInputState);
}

void AMainCharacterBase::CleanupOwnedEquipment(float CleanupDelay)
{
	TSet<ACharacterEquipmentBase*> OwnedEquipmentActors;

	auto CollectEquipment = [this, &OwnedEquipmentActors](ACharacterEquipmentBase* EquipmentActor)
	{
		if (!IsValid(EquipmentActor) || EquipmentActor->GetOwner() != this)
		{
			return;
		}

		OwnedEquipmentActors.Add(EquipmentActor);
	};

	CollectEquipment(RightWeapon);
	CollectEquipment(LeftWeapon);
	CollectEquipment(PendingEquipEquipment);
	CollectEquipment(PendingUnequipEquipment);

	for (ACharacterEquipmentBase* EquipmentActor : OwnedEquipmentActors)
	{
		if (CleanupDelay <= 0.f)
		{
			EquipmentActor->Destroy();
		}
		else
		{
			EquipmentActor->SetLifeSpan(CleanupDelay);
		}
	}
}

void AMainCharacterBase::ScheduleRespawn()
{
	if (!HasAuthority())
	{
		return;
	}

	GetWorldTimerManager().SetTimer(
		RespawnTimerHandle,
		this,
		&AMainCharacterBase::HandleRespawnTimerElapsed,
		FMath::Max(0.f, RespawnDelay),
		false);
}

void AMainCharacterBase::HandleRespawnTimerElapsed()
{
	if (!HasAuthority())
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(RespawnTimerHandle);

	AController* OwningController = Controller;
	if (!OwningController)
	{
		Destroy();
		return;
	}

	AGameModeBase* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode() : nullptr;
	if (!GameMode)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PlayerDeath] Failed to respawn %s because GameMode was not available"), *GetNameSafe(this));
		return;
	}

	CleanupOwnedEquipment(CorpseDestroyDelayAfterRespawn);

	OwningController->UnPossess();
	GameMode->RestartPlayer(OwningController);

	//KDH 부활 후 HP 갱신 추가
	if (AVL_PlayerController* PC = Cast<AVL_PlayerController>(OwningController))
	{
		PC->ClientRefreshPlayerInfoUI();
	}

	if (CorpseDestroyDelayAfterRespawn <= 0.f)
	{
		Destroy();
	}
	else
	{
		SetLifeSpan(CorpseDestroyDelayAfterRespawn);
	}
}

void AMainCharacterBase::ConfigureMeshForDeathRagdoll(USkeletalMeshComponent* MeshComponent, bool bDetachFromLeaderPose, bool bSimulatePhysics)
{
	if (!MeshComponent)
	{
		return;
	}

	if (bDetachFromLeaderPose)
	{
		MeshComponent->SetLeaderPoseComponent(nullptr, true, true);
	}

	MeshComponent->bPauseAnims = true;
	MeshComponent->SetAnimInstanceClass(nullptr);

	if (bSimulatePhysics && MeshComponent->GetPhysicsAsset())
	{
		MeshComponent->SetCollisionProfileName(TEXT("Ragdoll"));
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		MeshComponent->SetAllBodiesSimulatePhysics(true);
		MeshComponent->SetAllBodiesPhysicsBlendWeight(1.f);
		MeshComponent->SetSimulatePhysics(true);
		MeshComponent->WakeAllRigidBodies();
		MeshComponent->bBlendPhysics = true;
		return;
	}

	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComponent->SetAllBodiesSimulatePhysics(false);
	MeshComponent->SetAllBodiesPhysicsBlendWeight(0.f);
	MeshComponent->SetSimulatePhysics(false);
	MeshComponent->bBlendPhysics = false;
}

void AMainCharacterBase::TriggerDeathRagdoll()
{
	if (bDeathRagdollStarted || !GetMesh())
	{
		return;
	}

	bDeathRagdollStarted = true;

	StopAnimMontage(DeathMontage);

	if (MovementComp)
	{
		MovementComp->StopMovementImmediately();
		MovementComp->DisableMovement();
	}

	if (UCapsuleComponent* CharacterCapsule = GetCapsuleComponent())
	{
		CharacterCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	TArray<USkeletalMeshComponent*> SkeletalMeshComponents;
	GetComponents(SkeletalMeshComponents);

	USkeletalMeshComponent* MainMeshComponent = GetMesh();
	TArray<USkeletalMeshComponent*> ChildMeshComponents;

	for (USkeletalMeshComponent* SkeletalMeshComponent : SkeletalMeshComponents)
	{
		if (!SkeletalMeshComponent || SkeletalMeshComponent == MainMeshComponent)
		{
			continue;
		}

		ChildMeshComponents.Add(SkeletalMeshComponent);
	}

	const bool bHasChildMeshForRagdoll = ChildMeshComponents.ContainsByPredicate([](USkeletalMeshComponent* SkeletalMeshComponent)
	{
		return SkeletalMeshComponent && SkeletalMeshComponent->GetPhysicsAsset();
	});
	ConfigureMeshForDeathRagdoll(MainMeshComponent, false, !bHasChildMeshForRagdoll);

	for (USkeletalMeshComponent* SkeletalMeshComponent : ChildMeshComponents)
	{
		ConfigureMeshForDeathRagdoll(SkeletalMeshComponent, true, true);
	}
}

void AMainCharacterBase::MulticastTriggerDeathRagdoll_Implementation()
{
	TriggerDeathRagdoll();
}

void AMainCharacterBase::PlayHitReaction()
{
	if (!HasAuthority() || bIsDead || !HitReactMontage)
	{
		return;
	}

	if (ActionState == ECharacterActionState::Dead ||
		ActionState == ECharacterActionState::Rolling ||
		ActionState == ECharacterActionState::InAir)
	{
		return;
	}

	if (ActionState != ECharacterActionState::Stunned)
	{
		CancelEquippedActions();
		SetCharacterState(ECharacterActionState::Stunned);
	}
	else
	{
		// Restart the hit react cleanly so repeated hits can replay the montage.
		StopNetworkedMontage(HitReactMontage, 0.f);
	}

	PlayNetworkedMontage(HitReactMontage, HitReactPlayRate, NAME_None, false, false);
}

AVL_AICharacterBase* AMainCharacterBase::FindParriedMonster(AController* EventInstigator, AActor* DamageCauser) const
{
	if (AVL_AICharacterBase* Monster = Cast<AVL_AICharacterBase>(DamageCauser))
	{
		return Monster;
	}

	if (DamageCauser)
	{
		if (AVL_AICharacterBase* Monster = Cast<AVL_AICharacterBase>(DamageCauser->GetOwner()))
		{
			return Monster;
		}

		if (AVL_AICharacterBase* Monster = Cast<AVL_AICharacterBase>(DamageCauser->GetInstigator()))
		{
			return Monster;
		}
	}

	if (EventInstigator)
	{
		return Cast<AVL_AICharacterBase>(EventInstigator->GetPawn());
	}

	return nullptr;
}

void AMainCharacterBase::ApplyParryGroggyToAttacker(AController* EventInstigator, AActor* DamageCauser)
{
	AVL_AICharacterBase* ParriedMonster = FindParriedMonster(EventInstigator, DamageCauser);
	if (!ParriedMonster)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PlayerDamage] Character=%s parried but attacker was not found. DamageCauser=%s EventInstigator=%s"),
			*GetNameSafe(this),
			*GetNameSafe(DamageCauser),
			*GetNameSafe(EventInstigator));
		return;
	}

	ParriedMonster->ApplyGroggy(ParryGroggyAmount);
}

void AMainCharacterBase::HandleStatDeath()
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;
	ApplyDeathState();
}

void AMainCharacterBase::ApplyKnockback(FVector LaunchVelocity)
{
	if (HasAuthority())
	{
		LaunchCharacter(LaunchVelocity, true, true);
		return;
	}

	Server_ApplyKnockback(LaunchVelocity);
}

void AMainCharacterBase::Server_ApplyKnockback_Implementation(FVector LaunchVelocity)
{
	ApplyKnockback(LaunchVelocity);
}

void AMainCharacterBase::Client_ApplyKnockback_Implementation(FVector LaunchVelocity)
{
	ApplyKnockback(LaunchVelocity);
}

bool AMainCharacterBase::HandleEquipmentItem(FName ItemID, TSubclassOf<ACharacterEquipmentBase> EquipmentClass)
{
	if (bIsDead)
	{
		return false;
	}

	if (ItemID.IsNone() || !EquipmentClass)
	{
		return false;
	}

	if (!bCanAction)
	{
		QueuePendingEquipmentItem(ItemID, EquipmentClass);
		return true;
	}

	PendingEquipmentItemID = NAME_None;
	PendingEquipmentClass = nullptr;

	if (!HasAuthority())
	{
		ServerHandleEquipmentItem(ItemID, EquipmentClass);
		return true;
	}

	if (RightWeapon && RightWeapon->EquippedItemID == ItemID)
	{
		Unequipment(RightWeapon);
		return true;
	}

	if (LeftWeapon && LeftWeapon->EquippedItemID == ItemID)
	{
		Unequipment(LeftWeapon);
		return true;
	}

	ACharacterEquipmentBase* Equipment = GetOrCreateEquipment(EquipmentClass);
	if (!Equipment)
	{
		return false;
	}

	Equipment->EquippedItemID = ItemID;
	EquipItem(Equipment);
	return true;
}

void AMainCharacterBase::ServerHandleEquipmentItem_Implementation(FName ItemID, TSubclassOf<ACharacterEquipmentBase> EquipmentClass)
{
	HandleEquipmentItem(ItemID, EquipmentClass);
}

EWeaponAnimType AMainCharacterBase::GetWeaponAnimType() const
{
	return CurrentWeaponType;
}

bool AMainCharacterBase::ShouldUseLeftHandIK() const
{
	return LeftHandIKStateCount > 0;
}

bool AMainCharacterBase::ShouldUseRightHandIK() const
{
	return RightHandIKStateCount > 0;
}

FTransform AMainCharacterBase::GetLeftHandIKTransformCS() const
{
	return GetLeftHandIKTransformCSForMesh(GetMesh());
}

FTransform AMainCharacterBase::GetRightHandIKTransformCS() const
{
	return GetRightHandIKTransformCSForMesh(GetMesh());
}

FTransform AMainCharacterBase::GetLeftHandIKTransformCSForMesh(const USkeletalMeshComponent* MeshComponent) const
{
	if (!MeshComponent || LeftHandIKStateCount <= 0)
	{
		return FTransform::Identity;
	}

	FTransform LeftHandWorldTransform = LeftHandIKTransformWS;
	if (const ACharacterWeaponBase* RightHandWeapon = Cast<ACharacterWeaponBase>(RightWeapon);
		RightHandWeapon && RightWeapon->bEquipmentVisible)
	{
		RightHandWeapon->GetLeftHandIKWorldTransform(LeftHandWorldTransform);
	}
	else if (const ACharacterWeaponBase* LeftHandWeapon = Cast<ACharacterWeaponBase>(LeftWeapon);
		LeftHandWeapon && LeftWeapon->bEquipmentVisible)
	{
		LeftHandWeapon->GetLeftHandIKWorldTransform(LeftHandWorldTransform);
	}

	FTransform MeshWorldTransform = MeshComponent->GetComponentTransform();
	LeftHandWorldTransform.NormalizeRotation();
	MeshWorldTransform.NormalizeRotation();
	const FTransform LeftHandComponentTransform = LeftHandWorldTransform.GetRelativeTransform(MeshWorldTransform);
	return LeftHandComponentTransform;
}

FTransform AMainCharacterBase::GetRightHandIKTransformCSForMesh(const USkeletalMeshComponent* MeshComponent) const
{
	if (!MeshComponent || RightHandIKStateCount <= 0)
	{
		return FTransform::Identity;
	}

	FTransform RightHandWorldTransform = RightHandIKTransformWS;
	if (const ACharacterWeaponBase* RightHandWeapon = Cast<ACharacterWeaponBase>(RightWeapon);
		RightHandWeapon && RightWeapon->bEquipmentVisible)
	{
		RightHandWeapon->GetRightHandIKWorldTransform(RightHandWorldTransform);
	}
	else if (const ACharacterWeaponBase* LeftHandWeapon = Cast<ACharacterWeaponBase>(LeftWeapon);
		LeftHandWeapon && LeftWeapon->bEquipmentVisible)
	{
		LeftHandWeapon->GetRightHandIKWorldTransform(RightHandWorldTransform);
	}

	FTransform MeshWorldTransform = MeshComponent->GetComponentTransform();
	RightHandWorldTransform.NormalizeRotation();
	MeshWorldTransform.NormalizeRotation();
	const FTransform RightHandComponentTransform = RightHandWorldTransform.GetRelativeTransform(MeshWorldTransform);
	return RightHandComponentTransform;
}

void AMainCharacterBase::SetWeaponIKState(EWeaponTarget Target, bool bEnabled)
{
	ApplyWeaponIKState(Target, bEnabled ? 1 : -1);
}

void AMainCharacterBase::SetWeaponTrailState(EWeaponTarget Target, bool bEnabled)
{
	auto ApplyTrailState = [bEnabled](ACharacterEquipmentBase* Equipment)
	{
		if (ACharacterWeaponBase* Weapon = Cast<ACharacterWeaponBase>(Equipment))
		{
			Weapon->SetWeaponTrailEnabled(bEnabled);
		}
	};

	switch (Target)
	{
	case EWeaponTarget::Left:
		ApplyTrailState(LeftWeapon);
		break;
	case EWeaponTarget::Right:
		ApplyTrailState(RightWeapon);
		break;
	case EWeaponTarget::Both:
		ApplyTrailState(RightWeapon);
		ApplyTrailState(LeftWeapon);
		break;
	default:
		break;
	}
}

void AMainCharacterBase::UpdateEquipmentStateFromWeapons()
{
	bIsWeaponEquip = (RightWeapon != nullptr) || (LeftWeapon != nullptr);

	ACharacterWeaponBase* Weapon = Cast<ACharacterWeaponBase>(RightWeapon);
	CurrentWeaponType = Weapon ? Weapon->GetWeaponAnimType() : EWeaponAnimType::None;
}

void AMainCharacterBase::UpdateWeaponIKData()
{
	const bool bRequestedLeftHandIK = LeftHandIKStateCount > 0;
	const bool bRequestedRightHandIK = RightHandIKStateCount > 0;

	bUseLeftHandIK = false;
	bUseRightHandIK = false;
	LeftHandIKTransformWS = FTransform::Identity;
	RightHandIKTransformWS = FTransform::Identity;

	USkeletalMeshComponent* CharacterMesh = GetMesh();
	if (!CharacterMesh)
	{
		return;
	}

	const ACharacterWeaponBase* IKSourceWeapon = nullptr;
	if (const ACharacterWeaponBase* RightHandWeapon = Cast<ACharacterWeaponBase>(RightWeapon);
		RightHandWeapon && RightWeapon->bEquipmentVisible)
	{
		IKSourceWeapon = RightHandWeapon;
	}
	else if (const ACharacterWeaponBase* LeftHandWeapon = Cast<ACharacterWeaponBase>(LeftWeapon);
		LeftHandWeapon && LeftWeapon->bEquipmentVisible)
	{
		IKSourceWeapon = LeftHandWeapon;
	}

	if (!IKSourceWeapon)
	{
		return;
	}

	FTransform SocketWorldTransform;
	bool bHasLeftHandIKTarget = false;
	bool bHasRightHandIKTarget = false;

	if (bRequestedLeftHandIK && IKSourceWeapon->GetLeftHandIKWorldTransform(SocketWorldTransform))
	{
		LeftHandIKTransformWS = SocketWorldTransform;
		bHasLeftHandIKTarget = true;
	}

	if (bRequestedRightHandIK && IKSourceWeapon->GetRightHandIKWorldTransform(SocketWorldTransform))
	{
		RightHandIKTransformWS = SocketWorldTransform;
		bHasRightHandIKTarget = true;
	}

	bUseLeftHandIK = bRequestedLeftHandIK && bHasLeftHandIKTarget;
	bUseRightHandIK = bRequestedRightHandIK && bHasRightHandIKTarget;
}

void AMainCharacterBase::ApplyWeaponIKState(EWeaponTarget Target, int32 Delta)
{
	switch (Target)
	{
	case EWeaponTarget::Left:
		LeftHandIKStateCount = FMath::Max(0, LeftHandIKStateCount + Delta);
		break;
	case EWeaponTarget::Right:
		RightHandIKStateCount = FMath::Max(0, RightHandIKStateCount + Delta);
		break;
	case EWeaponTarget::Both:
		LeftHandIKStateCount = FMath::Max(0, LeftHandIKStateCount + Delta);
		RightHandIKStateCount = FMath::Max(0, RightHandIKStateCount + Delta);
		break;
	default:
		break;
	}

	UpdateWeaponIKData();
}

void AMainCharacterBase::UpdateCameraSocketOffset(float DeltaTime)
{
	if (!SpringArm)
	{
		return;
	}

	CurrentCameraSocketOffset = FMath::VInterpTo(
		CurrentCameraSocketOffset,
		TargetCameraSocketOffset,
		DeltaTime,
		CameraSocketOffsetInterpSpeed);

	if (CurrentCameraSocketOffset.Equals(TargetCameraSocketOffset, 0.1f))
	{
		CurrentCameraSocketOffset = TargetCameraSocketOffset;
	}

	SpringArm->SetRelativeLocation(FVector::ZeroVector);
	SpringArm->SocketOffset = CurrentCameraSocketOffset;

	// 활 조준 중이면 AimArmLength, 평상시엔 마우스 휠로 조절한 TargetArmLength 사용
	const float DesiredArmLength = bIsAiming ? AimArmLength : TargetArmLength;
	SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, DesiredArmLength, DeltaTime, ArmLengthInterpSpeed);
}

void AMainCharacterBase::UpdateLockOn(float DeltaTime)
{
	if (!IsValid(LockOnTarget))
	{
		return;
	}

	if (!IsLockOnTargetValid(LockOnTarget))
	{
		ExitLockOn();
		return;
	}

	AController* CurrentController = GetController();
	if (!CurrentController)
	{
		ExitLockOn();
		return;
	}

	if (RotationMode != ERotationMode::Strafe)
	{
		SetRotationMode(ERotationMode::Strafe);
	}

	TargetCameraSocketOffset = LockOnCameraSocketOffset;

	// SpringArm 베이스(=캐릭터 루트) + 높이 오프셋으로 계산해 카메라 SocketOffset 보간에 의한 피드백 루프를 제거
	const FVector ViewOrigin = (SpringArm ? SpringArm->GetComponentLocation() : GetActorLocation()) + FVector(0.f, 0.f, 50.f);
	const FVector FocusLocation = GetLockOnFocusLocation(LockOnTarget);
	TargetControlRotation = UKismetMathLibrary::FindLookAtRotation(ViewOrigin, FocusLocation);

	const FRotator NewControlRotation = FMath::RInterpTo(
		CurrentController->GetControlRotation(),
		TargetControlRotation,
		DeltaTime,
		LockOnInterpSpeed);

	CurrentController->SetControlRotation(NewControlRotation);
}

void AMainCharacterBase::UpdateCharacterFacing(float DeltaTime)
{
	if (RotationMode != ERotationMode::Strafe &&
		!bIsAiming &&
		ActionState != ECharacterActionState::Rolling &&
		FacingSource != EFacingSource::RollDirection &&
		FacingSource != EFacingSource::RollRecovery)
	{
		return;
	}

	AController* CurrentController = GetController();
	if (!CurrentController)
	{
		return;
	}

	FacingSource = DetermineFacingSource();

	if (FacingSource == EFacingSource::None)
	{
		return;
	}

	if (FacingSource == EFacingSource::RollDirection)
	{
		const bool bRollMontagePlaying =
			RollMontage &&
			GetMesh() &&
			GetMesh()->GetAnimInstance() &&
			GetMesh()->GetAnimInstance()->Montage_IsPlaying(RollMontage);

		if (ActionState == ECharacterActionState::Rolling || bRollMontagePlaying)
		{
			if (bSmoothRotating || IsValid(LockOnTarget))
			{
				const float RollInterpSpeed = bSmoothRotating
					? FMath::Max(0.f, InputRotationInterpSpeed)
					: StrafeFacingInterpSpeed;

				const FRotator NewRotation = FMath::RInterpTo(
					GetActorRotation(),
					TargetRotation,
					DeltaTime,
					RollInterpSpeed
				);

				SetActorRotation(NewRotation);

				if (FMath::Abs(FRotator::NormalizeAxis(NewRotation.Yaw - TargetRotation.Yaw)) < 1.f)
				{
					SetActorRotation(TargetRotation);

					if (bSmoothRotating)
					{
						bSmoothRotating = false;
					}
				}
			}
			else
			{
				SetActorRotation(TargetRotation);
			}

			return;
		}

		// 몽타주 끝났다고 여기서 RollRecovery 시작하지 않음.
		return;
	}

	if (ActionState == ECharacterActionState::Rolling && bDeferRollRecoveryUntilRollEnd && FacingSource == EFacingSource::RollRecovery)
	{
		SetActorRotation(TargetRotation);
		return;
	}

	FRotator DesiredRotation = GetActorRotation();
	float InterpSpeed = StrafeFacingInterpSpeed;

	switch (FacingSource)
	{
	case EFacingSource::InputDirection:
		DesiredRotation = TargetRotation;
		InterpSpeed = FMath::Max(0.f, InputRotationInterpSpeed);
		break;
	case EFacingSource::RollRecovery:
		if (!IsValid(LockOnTarget))
		{
			if (HasAuthority())
			{
				DesiredRotation = TargetRotation;
				InterpSpeed = 0.f;
				break;
			}

			FacingSource = EFacingSource::ControllerRotation;
			return;
		}

		DesiredRotation = FRotator(0.f, CurrentController->GetControlRotation().Yaw, 0.f);
		InterpSpeed = LockOnRollRecoveryInterpSpeed;
		break;
	case EFacingSource::ControllerRotation:
	default:
		DesiredRotation = FRotator(0.f, CurrentController->GetControlRotation().Yaw, 0.f);
		break;
	}

	const FRotator NewRotation = FMath::RInterpTo(
		GetActorRotation(),
		DesiredRotation,
		DeltaTime,
		InterpSpeed);

	SetActorRotation(NewRotation);

	if (FMath::Abs(FRotator::NormalizeAxis(NewRotation.Yaw - DesiredRotation.Yaw)) < 1.f)
	{
		SetActorRotation(DesiredRotation);

		if (FacingSource == EFacingSource::InputDirection)
		{
			bSmoothRotating = false;
			FacingSource = EFacingSource::ControllerRotation;
		}
		else if (FacingSource == EFacingSource::RollRecovery)
		{
			FacingSource = EFacingSource::ControllerRotation;
		}
	}
}

EFacingSource AMainCharacterBase::DetermineFacingSource() const
{
	if (RotationMode != ERotationMode::Strafe)
	{
		if (FacingSource == EFacingSource::InputDirection &&
			bSmoothRotating &&
			!IsValid(LockOnTarget) &&
			!bIsAiming)
		{
			return EFacingSource::InputDirection;
		}

		return EFacingSource::None;
	}

	const double Now = FPlatformTime::Seconds();
	const bool bHasQueuedRoll = IsBufferedInputActive(EPendingInputAction::Roll, Now);

	if (FacingSource == EFacingSource::RollDirection)
	{
		if (ActionState == ECharacterActionState::Rolling)
		{
			return EFacingSource::RollDirection;
		}

		// RollRecovery는 BeginLockOnRollRecovery()에서만 시작한다.
		return EFacingSource::None;
	}

	if (FacingSource == EFacingSource::RollRecovery)
	{
		return FacingSource;
	}

	if (bSmoothRotating && !IsValid(LockOnTarget) && !bIsAiming)
	{
		return EFacingSource::InputDirection;
	}

	return EFacingSource::ControllerRotation;
}

void AMainCharacterBase::ToggleLockOn()
{
	if (IsValid(LockOnTarget))
	{
		ExitLockOn();
		return;
	}

	if (bIsDead || bIsAiming || !Controller)
	{
		return;
	}

	if (AActor* NewTarget = FindBestLockOnTarget())
	{
		EnterLockOn(NewTarget);
	}
}

void AMainCharacterBase::EnterLockOn(AActor* NewTarget)
{
	if (!IsLockOnTargetValid(NewTarget))
	{
		return;
	}

	if (!IsValid(LockOnTarget))
	{
		RotationModeBeforeLockOn = RotationMode;
	}

	LockOnTarget = NewTarget;
	LockOnSwitchAccumulatedInput = FVector2D::ZeroVector;
	bLockOnSwitchInputWaitingForNeutral = false;
	SetRotationMode(ERotationMode::Strafe);
	TargetCameraSocketOffset = LockOnCameraSocketOffset;

	if (Controller)
	{
		TargetControlRotation = Controller->GetControlRotation();
	}
}

void AMainCharacterBase::ExitLockOn()
{
	if (!IsValid(LockOnTarget))
	{
		return;
	}

	LockOnTarget = nullptr;
	LockOnSwitchAccumulatedInput = FVector2D::ZeroVector;
	bLockOnSwitchInputWaitingForNeutral = false;

	if (Controller)
	{
		TargetControlRotation = Controller->GetControlRotation();
	}

	SetRotationMode(RotationModeBeforeLockOn);
}

AActor* AMainCharacterBase::FindBestLockOnTarget() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	const FVector Origin = GetActorLocation();
	const FVector ViewLocation = Camera ? Camera->GetComponentLocation() : Origin;
	const FVector ViewForward = Camera ? Camera->GetForwardVector() : GetActorForwardVector();
	const APlayerController* PlayerController = Cast<APlayerController>(Controller);

	int32 ViewportX = 0;
	int32 ViewportY = 0;
	if (PlayerController)
	{
		PlayerController->GetViewportSize(ViewportX, ViewportY);
	}

	const FVector2D ViewportSize(static_cast<float>(ViewportX), static_cast<float>(ViewportY));
	const bool bCanProjectToScreen = PlayerController && ViewportX > 0 && ViewportY > 0;
	const FVector2D ScreenCenter = ViewportSize * 0.5f;

	AActor* BestTarget = nullptr;
	float BestScore = TNumericLimits<float>::Max();

	for (TActorIterator<AVL_AICharacterBase> It(World); It; ++It)
	{
		AVL_AICharacterBase* Candidate = *It;
		if (!IsLockOnTargetValid(Candidate))
		{
			continue;
		}

		const FVector ToCandidate = GetLockOnFocusLocation(Candidate) - ViewLocation;
		const float Distance = ToCandidate.Size();
		if (Distance > LockOnSearchRadius || Distance <= KINDA_SMALL_NUMBER)
		{
			continue;
		}

		const FVector ToCandidateDir = ToCandidate / Distance;
		const float ViewDot = FVector::DotProduct(ViewForward, ToCandidateDir);
		if (ViewDot < LockOnMinViewDot)
		{
			continue;
		}

		float ScreenScore = 0.f;
		if (bCanProjectToScreen)
		{
			FVector2D ScreenPosition;
			if (PlayerController->ProjectWorldLocationToScreen(GetLockOnFocusLocation(Candidate), ScreenPosition))
			{
				const float MaxScreenDistance = ScreenCenter.Size();
				ScreenScore = MaxScreenDistance > KINDA_SMALL_NUMBER
					? FVector2D::Distance(ScreenPosition, ScreenCenter) / MaxScreenDistance
					: 0.f;
			}
			else
			{
				ScreenScore = 1.f;
			}
		}

		const float DistanceScore = Distance / FMath::Max(LockOnSearchRadius, 1.f);
		const float AlignmentScore = 1.f - ViewDot;
		const float TotalScore = (DistanceScore * 0.35f) + (AlignmentScore * 0.45f) + (ScreenScore * 0.20f);

		if (TotalScore < BestScore)
		{
			BestScore = TotalScore;
			BestTarget = Candidate;
		}
	}

	return BestTarget;
}

AActor* AMainCharacterBase::FindNextLockOnTarget(const FVector2D& InputDirection) const
{
	if (!IsValid(LockOnTarget))
	{
		return nullptr;
	}

	const float AbsX = FMath::Abs(InputDirection.X);
	const float AbsY = FMath::Abs(InputDirection.Y);
	if (FMath::Max(AbsX, AbsY) < LockOnSwitchInputDeadZone)
	{
		return nullptr;
	}

	const bool bUseHorizontal = AbsX >= AbsY;
	const float DesiredSign = bUseHorizontal ? FMath::Sign(InputDirection.X) : FMath::Sign(InputDirection.Y);
	if (FMath::IsNearlyZero(DesiredSign))
	{
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	const APlayerController* PlayerController = Cast<APlayerController>(Controller);
	const FVector ViewLocation = Camera ? Camera->GetComponentLocation() : GetActorLocation();
	const FVector CameraRight = Camera ? Camera->GetRightVector() : GetActorRightVector();
	const FVector CameraUp = Camera ? Camera->GetUpVector() : GetActorUpVector();
	const FVector CurrentFocus = GetLockOnFocusLocation(LockOnTarget);

	FVector2D CurrentScreenPosition = FVector2D::ZeroVector;
	bool bHasCurrentScreenPosition = false;
	if (PlayerController)
	{
		bHasCurrentScreenPosition = PlayerController->ProjectWorldLocationToScreen(CurrentFocus, CurrentScreenPosition);
	}

	AActor* BestTarget = nullptr;
	float BestScore = TNumericLimits<float>::Max();

	for (TActorIterator<AVL_AICharacterBase> It(World); It; ++It)
	{
		AVL_AICharacterBase* Candidate = *It;
		if (Candidate == LockOnTarget || !IsLockOnTargetValid(Candidate))
		{
			continue;
		}

		const FVector CandidateFocus = GetLockOnFocusLocation(Candidate);
		const FVector RelativeFromCurrent = CandidateFocus - CurrentFocus;
		const FVector RelativeDirection = RelativeFromCurrent.GetSafeNormal();
		const float PrimaryAxisValue = bUseHorizontal
			? FVector::DotProduct(CameraRight, RelativeDirection)
			: FVector::DotProduct(CameraUp, RelativeDirection);

		if (DesiredSign > 0.f && PrimaryAxisValue <= 0.f)
		{
			continue;
		}
		if (DesiredSign < 0.f && PrimaryAxisValue >= 0.f)
		{
			continue;
		}

		float CandidateScore = RelativeFromCurrent.Size();

		if (PlayerController && bHasCurrentScreenPosition)
		{
			FVector2D CandidateScreenPosition;
			if (PlayerController->ProjectWorldLocationToScreen(CandidateFocus, CandidateScreenPosition))
			{
				const float DeltaX = CandidateScreenPosition.X - CurrentScreenPosition.X;
				const float DeltaY = CandidateScreenPosition.Y - CurrentScreenPosition.Y;
				const float PrimaryScreenDelta = bUseHorizontal ? DeltaX : -DeltaY;
				const float SecondaryScreenDelta = bUseHorizontal ? DeltaY : DeltaX;

				if ((DesiredSign > 0.f && PrimaryScreenDelta <= 0.f) || (DesiredSign < 0.f && PrimaryScreenDelta >= 0.f))
				{
					continue;
				}

				CandidateScore = FMath::Abs(PrimaryScreenDelta) + (FMath::Abs(SecondaryScreenDelta) * 0.35f);
			}
		}

		const FVector ToCandidate = CandidateFocus - ViewLocation;
		const float Distance = ToCandidate.Size();
		if (Distance > LockOnSearchRadius || Distance <= KINDA_SMALL_NUMBER)
		{
			continue;
		}

		const float ViewDot = FVector::DotProduct((Camera ? Camera->GetForwardVector() : GetActorForwardVector()), ToCandidate / Distance);
		if (ViewDot < LockOnMinViewDot)
		{
			continue;
		}

		if (CandidateScore < BestScore)
		{
			BestScore = CandidateScore;
			BestTarget = Candidate;
		}
	}

	return BestTarget;
}

FVector AMainCharacterBase::GetLockOnFocusLocation(const AActor* Target) const
{
	if (!IsValid(Target))
	{
		return FVector::ZeroVector;
	}

	if (const ACharacter* TargetCharacter = Cast<ACharacter>(Target))
	{
		if (const UCapsuleComponent* TargetCapsule = TargetCharacter->GetCapsuleComponent())
		{
			return TargetCharacter->GetActorLocation() + FVector(0.f, 0.f, TargetCapsule->GetScaledCapsuleHalfHeight() * LockOnTargetHeightRatio);
		}
	}

	return Target->GetActorLocation();
}

bool AMainCharacterBase::IsLockOnTargetValid(const AActor* Target) const
{
	const AVL_AICharacterBase* MonsterTarget = Cast<AVL_AICharacterBase>(Target);
	if (!IsValid(MonsterTarget) || Target == this)
	{
		return false;
	}

	if (MonsterTarget->GetCurrentHP() <= 0.f)
	{
		return false;
	}

	const float DistanceSquared = FVector::DistSquared(GetActorLocation(), MonsterTarget->GetActorLocation());
	return DistanceSquared <= FMath::Square(LockOnBreakDistance);
}

void AMainCharacterBase::HandleLockOnSwitchInput(const FVector2D& SwitchInput)
{
	if (!IsValid(LockOnTarget))
	{
		return;
	}

	if (SwitchInput.IsNearlyZero())
	{
		LockOnSwitchAccumulatedInput = FVector2D::ZeroVector;
		bLockOnSwitchInputWaitingForNeutral = false;
		return;
	}

	if (bLockOnSwitchInputWaitingForNeutral)
	{
		return;
	}

	if (!FMath::IsNearlyZero(SwitchInput.X) &&
		!FMath::IsNearlyZero(LockOnSwitchAccumulatedInput.X) &&
		FMath::Sign(SwitchInput.X) != FMath::Sign(LockOnSwitchAccumulatedInput.X))
	{
		LockOnSwitchAccumulatedInput.X = 0.f;
	}

	if (!FMath::IsNearlyZero(SwitchInput.Y) &&
		!FMath::IsNearlyZero(LockOnSwitchAccumulatedInput.Y) &&
		FMath::Sign(SwitchInput.Y) != FMath::Sign(LockOnSwitchAccumulatedInput.Y))
	{
		LockOnSwitchAccumulatedInput.Y = 0.f;
	}

	LockOnSwitchAccumulatedInput += SwitchInput;

	const float AccumulatedAbsX = FMath::Abs(LockOnSwitchAccumulatedInput.X);
	const float AccumulatedAbsY = FMath::Abs(LockOnSwitchAccumulatedInput.Y);
	if (FMath::Max(AccumulatedAbsX, AccumulatedAbsY) < LockOnSwitchInputDeadZone)
	{
		return;
	}

	if (AActor* NextTarget = FindNextLockOnTarget(LockOnSwitchAccumulatedInput))
	{
		EnterLockOn(NextTarget);
		bLockOnSwitchInputWaitingForNeutral = true;
	}

	LockOnSwitchAccumulatedInput = FVector2D::ZeroVector;
}

void AMainCharacterBase::HandleAnimAction(EEquipMentHandleAction Action, EWeaponTarget target)
{
	if (Action == EEquipMentHandleAction::CanAction)
	{
		UAnimInstance* DebugAnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
		UAnimMontage* DebugActiveMontage = DebugAnimInstance ? DebugAnimInstance->GetCurrentActiveMontage() : nullptr;
		UE_LOG(LogTemp, Warning, TEXT("[PrimaryDebug][CanActionNotify] Char=%s Role=%s HasAuthority=%d Local=%d Target=%d ActiveMontage=%s bCanActionBefore=%d ActiveLocks=%d"),
			*GetNameSafe(this),
			GetRoleText(this),
			HasAuthority() ? 1 : 0,
			IsLocallyControlled() ? 1 : 0,
			static_cast<int32>(target),
			*GetNameSafe(DebugActiveMontage),
			bCanAction ? 1 : 0,
			ActiveMontageLockCount);

		SetCanMove(true);
		SetCanAction(true);
		bCanRun = false;
		SetGaitState(EGait::Walk);

		switch (target)
		{
		case EWeaponTarget::Right:
			if (RightWeapon)
			{
				RightWeapon->HandleAction(Action);
			}
			break;
		case EWeaponTarget::Left:
			if (LeftWeapon)
			{
				LeftWeapon->HandleAction(Action);
			}
			break;
		case EWeaponTarget::Both:
			if (RightWeapon)
			{
				RightWeapon->HandleAction(Action);
			}
			if (LeftWeapon)
			{
				LeftWeapon->HandleAction(Action);
			}
			break;
		}

		if (IsCurrentAbilityMontageActive())
		{
			ReleaseAbilityRootMotionControl();
		}

		UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
		UAnimMontage* QueuedActionSourceMontage = AnimInstance ? AnimInstance->GetCurrentActiveMontage() : nullptr;

		if (TryConsumeQueuedCharacterAction())
		{
			QueuedActionHandledMontage = QueuedActionSourceMontage;
			return;
		}

		BeginLockOnRollRecovery();

		return;
	}

	if (Action == EEquipMentHandleAction::LockMove)
	{
		SetCanMove(false);
		return;
	}

	if (Action == EEquipMentHandleAction::UnlockMove)
	{
		SetCanMove(true);
		return;
	}

	if (Action == EEquipMentHandleAction::UnequipComplete)
	{
		FinalizePendingUnequipEquipment();
		SetCanAction(true);
		return;
	}

	if (Action == EEquipMentHandleAction::AttachPendingEquipment)
	{
		FinalizePendingEquipEquipment();
		return;
	}

	if (Action == EEquipMentHandleAction::DeathRagdoll)
	{
		TriggerDeathRagdoll();
		return;
	}

	switch (target)
	{
	case EWeaponTarget::Right:
		if (RightWeapon)
		{
			RightWeapon->HandleAction(Action);
		}
		break;

	case EWeaponTarget::Left:
		if (LeftWeapon)
		{
			LeftWeapon->HandleAction(Action);
		}
		break;

	case EWeaponTarget::Both:
		if (RightWeapon)
		{
			RightWeapon->HandleAction(Action);
		}
		if (LeftWeapon)
		{
			LeftWeapon->HandleAction(Action);
		}
		break;
	}
}

// Interaction
void AMainCharacterBase::SetCurrentInteractorActor(AActor* NewActor)
{
	CurrentInteractActor = NewActor;
}

AActor* AMainCharacterBase::GetCurrentInteractActor() const
{
	return CurrentInteractActor;
}

void AMainCharacterBase::Interaction(const FInputActionValue& Value)
{
	if (bIsDead)
	{
		return;
	}

	if (!CurrentInteractActor)
	{
		return;
	}

	if (AVL_InteractActorBase* InteractActor = Cast<AVL_InteractActorBase>(CurrentInteractActor))
	{
		InteractActor->OnInteracted(this);
	}
}

// Action state and input buffering
bool AMainCharacterBase::CanDoAction() const
{
	if (bIsDead)
	{
		return false;
	}

	// 전신 차단 상태면 무기 액션 불가
	if (ActionState == ECharacterActionState::Stunned ||
		ActionState == ECharacterActionState::Dead  ||
		ActionState == ECharacterActionState::InAir)
	{
		return false;
	}
	// 무기 공격 윈도우 확인 (선딜 구간이면 false)
	return bCanAction;
}

bool AMainCharacterBase::CanBufferServerPrimaryActionRequest() const
{
	if (!HasAuthority() || !StatComponent || bIsDead)
	{
		return false;
	}

	if (ActionState == ECharacterActionState::Stunned ||
		ActionState == ECharacterActionState::Dead ||
		ActionState == ECharacterActionState::InAir)
	{
		return false;
	}

	return !bCanAction &&
		RightWeapon &&
		RightWeapon->CanStartAction(EEquipmentActionType::Primary);
}

bool AMainCharacterBase::BufferServerPrimaryActionRequest()
{
	if (!CanBufferServerPrimaryActionRequest())
	{
		UE_LOG(LogTemp, Warning, TEXT("[PrimaryDebug][ServerBufferPrimarySkipped] Char=%s Role=%s ActionState=%d bCanAction=%d ActiveLocks=%d RightWeapon=%s"),
			*GetNameSafe(this),
			GetRoleText(this),
			static_cast<int32>(ActionState),
			bCanAction ? 1 : 0,
			ActiveMontageLockCount,
			*GetNameSafe(RightWeapon));
		return false;
	}

	FBufferedInputState& InputState = GetBufferedInputState(EPendingInputAction::Primary);
	InputState.bPressed = false;
	InputState.bConsumed = false;
	InputState.BufferUntilTime = FPlatformTime::Seconds() + InputGracePeriod;

	UE_LOG(LogTemp, Warning, TEXT("[PrimaryDebug][ServerBufferPrimary] Char=%s Role=%s ComboBufferedUntil=%.3f ActionState=%d bCanAction=%d ActiveLocks=%d RightWeapon=%s"),
		*GetNameSafe(this),
		GetRoleText(this),
		InputState.BufferUntilTime,
		static_cast<int32>(ActionState),
		bCanAction ? 1 : 0,
		ActiveMontageLockCount,
		*GetNameSafe(RightWeapon));

	return true;
}

void AMainCharacterBase::CancelEquippedActions()
{
	if (!HasAuthority() && Cast<AShieldBase>(LeftWeapon))
	{
		ServerSetShieldGuarding(false);
	}

	if (RightWeapon) RightWeapon->CancelAction();
	if (LeftWeapon) LeftWeapon->CancelAction();
}

bool AMainCharacterBase::TryConsumeQueuedCharacterAction()
{
	const double Now = FPlatformTime::Seconds();
	const EPendingInputAction PendingAction = GetBufferedActionToExecute(Now);
	if (PendingAction == EPendingInputAction::Roll)
	{
		bSmoothRotating = false;
		bDeferRollRecoveryUntilRollEnd = false;

		if (FacingSource == EFacingSource::RollRecovery ||
			FacingSource == EFacingSource::ControllerRotation)
		{
			FacingSource = EFacingSource::RollDirection;
		}

		PrepareQueuedRollFacing();
	}

	if (ConsumePendingEquipmentItem())
	{
		return true;
	}

	if (PendingAction == EPendingInputAction::None)
	{
		return false;
	}

	ProcessPendingInputs();
	return true;
}

FBufferedInputState& AMainCharacterBase::GetBufferedInputState(EPendingInputAction Action)
{
	switch (Action)
	{
	case EPendingInputAction::Ability:   return AbilityInputState;
	case EPendingInputAction::Run:       return RunInputState;
	case EPendingInputAction::Secondary: return SecondaryInputState;
	case EPendingInputAction::Jump:      return JumpInputState;
	case EPendingInputAction::Roll:      return RollInputState;
	case EPendingInputAction::Primary:   return PrimaryInputState;
	default:                             return PrimaryInputState;
	}
}

const FBufferedInputState& AMainCharacterBase::GetBufferedInputState(EPendingInputAction Action) const
{
	switch (Action)
	{
	case EPendingInputAction::Ability:   return AbilityInputState;
	case EPendingInputAction::Run:       return RunInputState;
	case EPendingInputAction::Secondary: return SecondaryInputState;
	case EPendingInputAction::Jump:      return JumpInputState;
	case EPendingInputAction::Roll:      return RollInputState;
	case EPendingInputAction::Primary:   return PrimaryInputState;
	default:                             return PrimaryInputState;
	}
}

bool AMainCharacterBase::ShouldRepeatBufferedInputWhilePressed(EPendingInputAction Action)
{
	return Action == EPendingInputAction::Primary ||
		Action == EPendingInputAction::Secondary ||
		Action == EPendingInputAction::Run ||
		Action == EPendingInputAction::Roll;
}

void AMainCharacterBase::PressBufferedInput(EPendingInputAction Action)
{
	FBufferedInputState& InputState = GetBufferedInputState(Action);
	InputState.bPressed = true;
	InputState.bConsumed = false;
	InputState.BufferUntilTime = 0.0;
}

void AMainCharacterBase::ReleaseBufferedInput(EPendingInputAction Action)
{
	FBufferedInputState& InputState = GetBufferedInputState(Action);
	InputState.bPressed = false;

	if (ShouldRepeatBufferedInputWhilePressed(Action) || !InputState.bConsumed)
	{
		InputState.BufferUntilTime = FPlatformTime::Seconds() + InputGracePeriod;
	}
	else
	{
		InputState.BufferUntilTime = 0.0;
	}
}

void AMainCharacterBase::ConsumeBufferedInput(EPendingInputAction Action)
{
	FBufferedInputState& InputState = GetBufferedInputState(Action);

	if (ShouldRepeatBufferedInputWhilePressed(Action))
	{
		if (!InputState.bPressed)
		{
			InputState.BufferUntilTime = 0.0;
		}
		return;
	}

	InputState.bConsumed = true;
	InputState.BufferUntilTime = 0.0;
}

bool AMainCharacterBase::IsBufferedInputPressed(EPendingInputAction Action) const
{
	return GetBufferedInputState(Action).bPressed;
}

bool AMainCharacterBase::IsBufferedInputActive(EPendingInputAction Action, double Now) const
{
	const FBufferedInputState& InputState = GetBufferedInputState(Action);

	if (InputState.bPressed)
	{
		return ShouldRepeatBufferedInputWhilePressed(Action) || !InputState.bConsumed;
	}

	return InputState.BufferUntilTime > Now;
}

EPendingInputAction AMainCharacterBase::GetBufferedActionToExecute(double Now) const
{
	if (IsBufferedInputActive(EPendingInputAction::Primary, Now))
	{
		return EPendingInputAction::Primary;
	}

	if (IsBufferedInputActive(EPendingInputAction::Ability, Now))
	{
		return EPendingInputAction::Ability;
	}

	if (IsBufferedInputActive(EPendingInputAction::Roll, Now))
	{
		return EPendingInputAction::Roll;
	}

	if (IsBufferedInputActive(EPendingInputAction::Jump, Now))
	{
		return EPendingInputAction::Jump;
	}

	if (IsBufferedInputActive(EPendingInputAction::Secondary, Now))
	{
		return EPendingInputAction::Secondary;
	}

	return EPendingInputAction::None;
}

void AMainCharacterBase::RecoverActionWindowIfIdle()
{
	if (bIsDead)
	{
		return;
	}

	if (ActionState != ECharacterActionState::Idle)
	{
		return;
	}

	UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (AnimInstance && AnimInstance->IsAnyMontagePlaying())
	{
		return;
	}

	const bool bNeedsRecovery = !bCanAction || !bCanMove || ActiveMontageLockCount != 0 || !bCanRun;
	if (!bNeedsRecovery)
	{
		return;
	}

	ActiveMontageLockCount = 0;
	SetCanMove(true);
	SetCanAction(true);
	bCanRun = true;
	PrepareQueuedRollFacing();

	if (IsBufferedInputPressed(EPendingInputAction::Run))
	{
		StartRun();
	}
}

bool AMainCharacterBase::CanUseRequestedGait(EGait DesiredGait) const
{
	if (DesiredGait != EGait::Run && DesiredGait != EGait::Sprint)
	{
		return true;
	}

	if (bIsDead || !bCanRun || bIsAiming || bIsGuarding || bIsBlocking)
	{
		return false;
	}

	if (ActionState == ECharacterActionState::Dead || ActionState == ECharacterActionState::InAir)
	{
		return false;
	}

	return StatComponent && StatComponent->CanSpendStamina(KINDA_SMALL_NUMBER);
}

void AMainCharacterBase::RestorePostAirborneState()
{
	if (bIsDead)
	{
		return;
	}

	ExitState(ECharacterActionState::InAir);
	ActiveMontageLockCount = 0;
	SetCanAction(true);
	SetCanMove(true);
	bCanRun = true;

	if (IsBufferedInputPressed(EPendingInputAction::Run))
	{
		StartRun();
	}
	else
	{
		SetGaitState(EGait::Walk);
	}
}

void AMainCharacterBase::PrepareQueuedRollFacing()
{
	if (RotationMode != ERotationMode::Strafe || !IsValid(LockOnTarget))
	{
		return;
	}

	if (GetBufferedActionToExecute(FPlatformTime::Seconds()) != EPendingInputAction::Roll)
	{
		return;
	}

	FRotator DesiredRollRotation = FRotator::ZeroRotator;
	if (!GetDesiredMoveInputRotation(DesiredRollRotation))
	{
		return;
	}

	QueuedRollRotation = FRotator(0.f, DesiredRollRotation.Yaw, 0.f);
	bHasQueuedRollRotation = true;
}

void AMainCharacterBase::BeginLockOnRollRecovery()
{
	if (RotationMode != ERotationMode::Strafe || !IsValid(LockOnTarget))
	{
		return;
	}

	if (!HasAuthority() && !IsLocallyControlled())
	{
		return;
	}

	FVector ToTarget = GetLockOnFocusLocation(LockOnTarget) - GetActorLocation();
	ToTarget.Z = 0.f;

	if (ToTarget.IsNearlyZero())
	{
		return;
	}

	TargetRotation = ToTarget.Rotation();

	bSmoothRotating = false;
	FacingSource = EFacingSource::RollRecovery;

	ForceNetUpdate();
}

void AMainCharacterBase::ProcessPendingInputs()
{
	if (bIsDead)
	{
		return;
	}

	const double Now = FPlatformTime::Seconds();
	const EPendingInputAction ActionToExecute = GetBufferedActionToExecute(Now);

	switch (ActionToExecute)
	{
	case EPendingInputAction::Primary:
		StartPrimaryAction();
		if (!IsBufferedInputPressed(EPendingInputAction::Primary)) StopPrimaryAction();
		break;
	case EPendingInputAction::Ability:
		StartAbilityAction();
		if (!IsBufferedInputPressed(EPendingInputAction::Ability)) StopAbilityAction();
		break;
	case EPendingInputAction::Secondary:
		StartSecondaryAction();
		if (!IsBufferedInputPressed(EPendingInputAction::Secondary)) StopSecondaryAction();
		break;
	case EPendingInputAction::Jump:
		StartJump();
		if (!IsBufferedInputPressed(EPendingInputAction::Jump)) StopJump();
		break;
	case EPendingInputAction::Roll:
		Roll();
		break;
	default:
		break;
	}
}

void AMainCharacterBase::SetCharacterState(ECharacterActionState NewState)
{
	ActionState = NewState;
}

void AMainCharacterBase::OnGlobalMontageStarted(UAnimMontage* Montage)
{
	if (ActiveHitStopMontage && ActiveHitStopMontage != Montage)
	{
		RestoreMontageHitStop();
	}

	if (Montage == RollMontage)
	{
		RollDebugLastLocation = GetActorLocation();
		RollDebugFrameCount = 12;

		const FRotator ControlRot = Controller ? Controller->GetControlRotation() : FRotator::ZeroRotator;
		UE_LOG(LogTemp, Warning, TEXT("[RollDebug][MontageStarted] Char=%s Role=%s HasAuthority=%d Local=%d ActorYaw=%.2f TargetYaw=%.2f ControlYaw=%.2f Facing=%s ActionState=%d"),
			*GetNameSafe(this),
			GetRoleText(this),
			HasAuthority() ? 1 : 0,
			IsLocallyControlled() ? 1 : 0,
			GetActorRotation().Yaw,
			TargetRotation.Yaw,
			ControlRot.Yaw,
			GetFacingSourceText(FacingSource),
			static_cast<int32>(ActionState));
	}

	if (Montage != RollMontage && (FacingSource == EFacingSource::RollRecovery || FacingSource == EFacingSource::RollDirection))
	{
		FacingSource = RotationMode == ERotationMode::Strafe
			? EFacingSource::ControllerRotation
			: EFacingSource::None;
	}

	ActiveMontageLockCount++;
	SetCanAction(false);
	SetCanMove(false);
	bCanRun = false;
	SetGaitState(EGait::Walk);
}


void AMainCharacterBase::OnGlobalMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == ActiveHitStopMontage)
	{
		RestoreMontageHitStop();
	}

	RestoreAbilityRootMotionControl();

	const bool bRollRestartInProgress = Montage == RollMontage && bStartingRollMontage && ActionState == ECharacterActionState::Rolling;

	if (Montage == RollMontage)
	{
		RollDebugFrameCount = 0;

	}

	if (Montage == RollMontage && !bInterrupted && !bRollRestartInProgress)
	{
		ExitState(ECharacterActionState::Rolling);
	}

	if (Montage == HitReactMontage && !bInterrupted && ActionState == ECharacterActionState::Stunned && !bIsDead)
	{
		ExitState(ECharacterActionState::Stunned);
	}

	if (PendingUnequipEquipment && Montage == PendingUnequipEquipment->UnequipMontage)
	{
		FinalizePendingUnequipEquipment();
	}

	if (PendingEquipEquipment && Montage == PendingEquipEquipment->EquipMontage)
	{
		FinalizePendingEquipEquipment();
	}

	if (Montage == DeathMontage && !bDeathRagdollStarted)
	{
		TriggerDeathRagdoll();
	}

	const bool bQueuedActionAlreadyHandled = QueuedActionHandledMontage == Montage;
	if (bQueuedActionAlreadyHandled)
	{
		QueuedActionHandledMontage = nullptr;
	}

	ActiveMontageLockCount = FMath::Max(0, ActiveMontageLockCount - 1);
	if (bIsDead)
	{
		ActiveMontageLockCount = 0;
		return;
	}

	if (bRollRestartInProgress)
	{
		return;
	}

	if (Montage == RollMontage)
	{
		bDeferRollRecoveryUntilRollEnd = false;

		const bool bQueuedRoll = IsBufferedInputActive(
			EPendingInputAction::Roll,
			FPlatformTime::Seconds()
		);

		if (!bInterrupted &&
			!bQueuedRoll &&
			!bIsAiming &&
			!bIsGuarding &&
			!IsValid(LockOnTarget))
		{
			SetRotationMode(ERotationMode::OrientToMovement);

			if (IsBufferedInputPressed(EPendingInputAction::Run))
			{
				SetGaitState(EGait::Run);
			}
			else
			{
				SetGaitState(EGait::Walk);
			}
		}
	}

	if (ActiveMontageLockCount == 0)
	{
		bCanRun = true;
		SetCanMove(true);
		SetCanAction(true);

		if (!bQueuedActionAlreadyHandled && TryConsumeQueuedCharacterAction())
		{
			return;
		}

		if (IsBufferedInputPressed(EPendingInputAction::Run))
		{
			StartRun();
		}
	}
}

void AMainCharacterBase::SetCanAction(bool bCan)
{
	bCanAction = bCan;
}

void AMainCharacterBase::ExitState(ECharacterActionState ExitingState)
{
	if (ActionState == ExitingState)
	{
		ActionState = ECharacterActionState::Idle;
	}
}

// Jump and airborne actions
void AMainCharacterBase::OnJumpPressed()
{
	PressBufferedInput(EPendingInputAction::Jump);
	StartJump();
}

void AMainCharacterBase::OnJumpReleased()
{
	ReleaseBufferedInput(EPendingInputAction::Jump);
	StopJump();
}

void AMainCharacterBase::StartJump()
{
	if (!CanDoAction() || !StatComponent || bIsDead)
	{
		return;
	}

	if (!StatComponent->ConsumeStamina(JumpStaminaCost))
	{
		return;
	}

	ConsumeBufferedInput(EPendingInputAction::Jump);
	CancelEquippedActions();

	Super::Jump();

	// 스태미나 소모는 서버에 요청
	if (!HasAuthority())
	{
		ServerConsumeJumpStamina();
		return;
	}

	StatComponent->ConsumeStamina(JumpStaminaCost);
}

void AMainCharacterBase::StopJump()
{
	Super::StopJumping();
}

// Movement and rotation
void AMainCharacterBase::UpdateSmoothRotation(float DeltaTime)
{
	if (RotationMode == ERotationMode::Strafe || IsValid(LockOnTarget))
	{
		bSmoothRotating = false;
		return;
	}

	if (!bSmoothRotating || bIsAiming)
	{
		return;
	}

	const FRotator CurrentRotation = GetActorRotation();
	const FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, FMath::Max(0.f, InputRotationInterpSpeed));
	SetActorRotation(NewRotation);

	if (FMath::Abs(FRotator::NormalizeAxis(NewRotation.Yaw - TargetRotation.Yaw)) < 1.f)
	{
		bSmoothRotating = false;
	}
}


void AMainCharacterBase::Rotation(const struct FInputActionValue& Value)
{
	const FVector2D LookAxis = Value.Get<FVector2D>();

	if (IsValid(LockOnTarget))
	{
		HandleLockOnSwitchInput(LookAxis);
		return;
	}

	const float YawInput = LookAxis.X; // 마우스 좌우
	const float PitchInput = LookAxis.Y; // 마우스 상하

	AddControllerYawInput(YawInput);
	AddControllerPitchInput(PitchInput);
	SendStrafeFacingToServer();
}

void AMainCharacterBase::OnLockOnPressed()
{
	ToggleLockOn();
}

void AMainCharacterBase::OnCameraZoom(const FInputActionValue& Value)
{
	// 활 조준 중에는 줌 입력 무시 (AimArmLength 시스템이 암 길이 담당)
	if (bIsAiming)
	{
		return;
	}

	const float ScrollValue = Value.Get<float>();
	if (FMath::IsNearlyZero(ScrollValue))
	{
		return;
	}

	// 휠 위 = 줌인(암 짧아짐), 휠 아래 = 줌아웃(암 길어짐)
	TargetArmLength = FMath::Clamp(TargetArmLength - ScrollValue * ZoomStep, MinArmLength, MaxArmLength);
}

void AMainCharacterBase::Move(const struct FInputActionValue& Value)
{
	const FVector2D MoveVector = Value.Get<FVector2D>();
	CachedMoveInput = MoveVector;

	if (bIsDead || !bCanMove)
	{
		// 이동 잠금 중 - 실제 이동만 막고 입력 방향은 유지한다.
		return;
	}

	if (MoveVector.IsNearlyZero()) return;

	const FRotator ControlRot = Controller ? Controller->GetControlRotation() : FRotator::ZeroRotator;
	const FRotator YawRot(0.f, ControlRot.Yaw, 0.f);

	const FVector Forward = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);

	AddMovementInput(Forward, MoveVector.Y);
	AddMovementInput(Right, MoveVector.X);
}

void AMainCharacterBase::ClearMoveInput()
{
	CachedMoveInput = FVector2D::ZeroVector;
}

void AMainCharacterBase::OnRunPressed()
{
	PressBufferedInput(EPendingInputAction::Run);
	StartRun();
}

void AMainCharacterBase::OnRunReleased()
{
	ReleaseBufferedInput(EPendingInputAction::Run);
	StopRun();
}

void AMainCharacterBase::StartRun()
{
	if (!CanUseRequestedGait(EGait::Run))
	{
		SetGaitState(EGait::Walk);
		return;
	}

	ConsumeBufferedInput(EPendingInputAction::Run);

	if (HasAuthority())
	{
		SetGaitState(EGait::Run, false);
	}
	else
	{
		ServerSetGait(EGait::Run);
	}
}

void AMainCharacterBase::StopRun()
{
	SetGaitState(EGait::Walk);
}

void AMainCharacterBase::StartSprint()
{
	if (!CanUseRequestedGait(EGait::Sprint))
	{
		SetGaitState(EGait::Walk);
		return;
	}

	if (HasAuthority())
	{
		SetGaitState(EGait::Sprint, false);
	}
	else
	{
		ServerSetGait(EGait::Sprint);
	}
}

void AMainCharacterBase::StopSprint()
{
	SetGaitState(EGait::Walk);
}

void AMainCharacterBase::OnBowZoomChanged(bool bZooming)
{

	SetAimingState(bZooming);

	const bool bRolling =
		ActionState == ECharacterActionState::Rolling ||
		(
			RollMontage &&
			GetMesh() &&
			GetMesh()->GetAnimInstance() &&
			GetMesh()->GetAnimInstance()->Montage_IsPlaying(RollMontage)
			);

	if (bRolling)
	{
		if (bZooming)
		{
			// CanAction 타이밍에 활 시위 당기는 경우는 Strafe로 전환해야 함
			ExitLockOn();
			SetRotationMode(ERotationMode::Strafe);
			SetGaitState(EGait::Walk);
			return;
		}

		// 활 공격이 구르기 CanAction 타이밍에 끝나면 false 이벤트가 롤 중에 들어온다.
		// 이때 조준/가드/락온이 아니면 여기서 Strafe를 풀어야 이후 회전 모드가 남지 않는다.
		if (!bIsGuarding && !IsValid(LockOnTarget))
		{
			SetRotationMode(ERotationMode::OrientToMovement);
		}
		return;
	}

	if (bZooming)
	{
		ExitLockOn();
		SetRotationMode(ERotationMode::Strafe);
		SetGaitState(EGait::Walk);
		return;
	}

	if (!bIsGuarding)
	{
		SetRotationMode(ERotationMode::OrientToMovement);
	}

	if (!bIsGuarding && IsBufferedInputPressed(EPendingInputAction::Run))
	{
		SetGaitState(EGait::Run);
	}
}

void AMainCharacterBase::OnShieldBlockChanged(bool bBlocking)
{
	bIsBlocking = bBlocking;

	if (bBlocking)
	{
		SetGaitState(EGait::Walk);
	}
	else if (!bIsGuarding)
	{
		if (IsBufferedInputPressed(EPendingInputAction::Run))
			SetGaitState(EGait::Run);
	}
}

void AMainCharacterBase::OnShieldGuardChanged(bool bGuarding)
{
	bIsGuarding = bGuarding;

	if (bGuarding)
	{
		ExitLockOn();
		SetRotationMode(ERotationMode::Strafe);
		SetGaitState(EGait::Walk);
		return;
	}

	bCanParry = false;

	if (!bIsAiming)
	{
		SetRotationMode(ERotationMode::OrientToMovement);
	}

	if (IsBufferedInputPressed(EPendingInputAction::Run))
	{
		SetGaitState(EGait::Run);
	}
}

void AMainCharacterBase::SetDefenseWindowState(EDefenseWindowType WindowType, bool bEnabled)
{
	auto UpdateCount = [bEnabled](int32& Count)
	{
		Count = bEnabled ? Count + 1 : FMath::Max(0, Count - 1);
		return Count > 0;
	};

	switch (WindowType)
	{
	case EDefenseWindowType::Invincible:
		bIsInvincible = UpdateCount(InvincibleWindowCount);
		break;
	case EDefenseWindowType::Parry:
	{
		const bool bParryWindowActive = UpdateCount(ParryWindowCount);
		if (AShieldBase* ShieldEquipment = Cast<AShieldBase>(LeftWeapon))
		{
			bCanParry = bParryWindowActive && ShieldEquipment->IsBlockInputHeld();
			ShieldEquipment->SetParryWindowActive(bCanParry);
		}
		else
		{
			bCanParry = bParryWindowActive;
		}
		break;
	}
	case EDefenseWindowType::Block:
	{
		const bool bBlockWindowActive = UpdateCount(BlockWindowCount);
		if (AShieldBase* ShieldEquipment = Cast<AShieldBase>(LeftWeapon))
		{
			ShieldEquipment->SetBlockWindowActive(bBlockWindowActive);
		}
		else
		{
			OnShieldBlockChanged(bBlockWindowActive);
		}
		break;
	}
	default:
		break;
	}
}

void AMainCharacterBase::SetAimingState(bool bNewAiming, bool bRequestServer)
{
	if (bIsAiming == bNewAiming)
	{
		return;
	}

	bIsAiming = bNewAiming;

	if (!HasAuthority() && bRequestServer)
	{
		ServerSetAiming(bNewAiming);
	}
}

bool AMainCharacterBase::ShouldRotateToInputForWeaponAction() const
{
	if (IsValid(LockOnTarget) || bIsAiming)
	{
		return false;
	}

	return !GetEquippedBow();
}

bool AMainCharacterBase::GetDesiredMoveInputRotation(FRotator& OutRotation) const
{
	const FRotator ControlRot = Controller ? Controller->GetControlRotation() : GetActorRotation();
	const FRotator YawRot(0.f, ControlRot.Yaw, 0.f);
	const FVector Forward = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);
	const FVector DesiredDirection = (Forward * CachedMoveInput.Y + Right * CachedMoveInput.X).GetSafeNormal2D();

	if (DesiredDirection.IsNearlyZero())
	{
		return false;
	}

	OutRotation = FRotator(0.f, DesiredDirection.Rotation().Yaw, 0.f);
	return true;
}

void AMainCharacterBase::ApplyRollFacing(const FRotator& DesiredRotation, bool bHasDesiredRotation, bool bUseImmediateRotation, bool bUseSmoothRotation, EFacingSource NewFacingSource)
{
	FacingSource = NewFacingSource;

	if (!bHasDesiredRotation)
	{
		return;
	}

	TargetRotation = FRotator(0.f, DesiredRotation.Yaw, 0.f);

	if (bUseImmediateRotation)
	{
		bSmoothRotating = false;
		SetActorRotation(TargetRotation);
		return;
	}

	if (bUseSmoothRotation)
	{
		InputRotationInterpSpeed = 10.f;
		bSmoothRotating = true;
	}
}

void AMainCharacterBase::ApplyInputRotation(const FRotator& DesiredRotation, float RotationInterpSpeed)
{
	TargetRotation = FRotator(0.f, DesiredRotation.Yaw, 0.f);
	InputRotationInterpSpeed = FMath::Max(0.f, RotationInterpSpeed);
	bSmoothRotating = true;
}

void AMainCharacterBase::SetGaitState(EGait NewGait, bool bRequestServer)
{
	EGait SanitizedGait = NewGait;
	const bool bNeedsRunPermissions = SanitizedGait == EGait::Run || SanitizedGait == EGait::Sprint;

	if (bIsDead)
	{
		SanitizedGait = EGait::Walk;
	}
	else if (bNeedsRunPermissions && !CanUseRequestedGait(SanitizedGait))
	{
		SanitizedGait = EGait::Walk;
	}

	if (!HasAuthority() && bRequestServer && bNeedsRunPermissions)
	{
		ServerSetGait(SanitizedGait);

		if (SanitizedGait == EGait::Walk && Gait != EGait::Walk)
		{
			Gait = EGait::Walk;
		}

		return;
	}

	if (Gait == SanitizedGait)
	{
		return;
	}

	Gait = SanitizedGait;

	if (!HasAuthority() && bRequestServer)
	{
		ServerSetGait(SanitizedGait);
	}
}

void AMainCharacterBase::ServerSetGait_Implementation(EGait NewGait)
{
	SetGaitState(NewGait, false);
}

void AMainCharacterBase::ServerSetAiming_Implementation(bool bNewAiming)
{
	SetAimingState(bNewAiming, false);
}

void AMainCharacterBase::ServerSetRotationMode_Implementation(ERotationMode NewRotationMode, FRotator ControlRotation)
{
	SetRotationMode(NewRotationMode, false);

	if (NewRotationMode == ERotationMode::Strafe && !IsValid(LockOnTarget) && !bIsAiming)
	{
		ApplyAuthoritativeStrafeFacing(ControlRotation);
	}
}

void AMainCharacterBase::ServerSetShieldGuarding_Implementation(bool bGuarding)
{
	if (AShieldBase* ShieldEquipment = Cast<AShieldBase>(LeftWeapon))
	{
		ShieldEquipment->SetGuardInputHeld(bGuarding);
	}
}

void AMainCharacterBase::ServerApplyRollFacing_Implementation(FRotator DesiredRotation, bool bHasDesiredRotation, bool bUseImmediateRotation, bool bUseSmoothRotation, EFacingSource NewFacingSource)
{
	if (!RollMontage ||
		ActionState == ECharacterActionState::Stunned ||
		ActionState == ECharacterActionState::Dead ||
		ActionState == ECharacterActionState::InAir ||
		!StatComponent ||
		!StatComponent->ConsumeStamina(RollStaminaCost))
	{
		return;
	}

	SetCharacterState(ECharacterActionState::Rolling);
	CancelEquippedActions();
	ApplyRollFacing(DesiredRotation, bHasDesiredRotation, bUseImmediateRotation, bUseSmoothRotation, NewFacingSource);
}

void AMainCharacterBase::ServerStartRoll_Implementation(FRotator DesiredRotation, bool bHasDesiredRotation, bool bUseImmediateRotation, bool bUseSmoothRotation, EFacingSource NewFacingSource, bool bDeferRecoveryUntilRollEnd)
{
	const FRotator ControlRot = Controller ? Controller->GetControlRotation() : FRotator::ZeroRotator;
	UE_LOG(LogTemp, Warning, TEXT("[RollDebug][ServerStartRoll] Char=%s Role=%s ActorYaw=%.2f TargetYaw=%.2f ControlYaw=%.2f DesiredYaw=%.2f HasDesired=%d Immediate=%d Smooth=%d NewFacing=%s DeferRecovery=%d Stamina=%.2f"),
		*GetNameSafe(this),
		GetRoleText(this),
		GetActorRotation().Yaw,
		TargetRotation.Yaw,
		ControlRot.Yaw,
		DesiredRotation.Yaw,
		bHasDesiredRotation ? 1 : 0,
		bUseImmediateRotation ? 1 : 0,
		bUseSmoothRotation ? 1 : 0,
		GetFacingSourceText(NewFacingSource),
		bDeferRecoveryUntilRollEnd ? 1 : 0,
		StatComponent ? StatComponent->GetCurrentStamina() : -1.f);

	if (!RollMontage ||
		ActionState == ECharacterActionState::Stunned ||
		ActionState == ECharacterActionState::Dead ||
		ActionState == ECharacterActionState::InAir ||
		!StatComponent ||
		!StatComponent->ConsumeStamina(RollStaminaCost))
	{
		return;
	}

	// 서버에서도 CanAction 때 걸린 RollRecovery를 새 구르기가 물고 가지 않게 정리
	if (FacingSource == EFacingSource::RollRecovery)
	{
		FacingSource = EFacingSource::ControllerRotation;
	}

	bSmoothRotating = false;
	bPendingRollRecoveryDeferral = false;

	SetCharacterState(ECharacterActionState::Rolling);
	this->bDeferRollRecoveryUntilRollEnd = bDeferRecoveryUntilRollEnd;

	CancelEquippedActions();
	ApplyRollFacing(DesiredRotation, bHasDesiredRotation, bUseImmediateRotation, bUseSmoothRotation, NewFacingSource);

	if (bHasDesiredRotation)
	{
		ActiveRollRootMotionRotation = FRotator(0.f, DesiredRotation.Yaw, 0.f);
	}
	else
	{
		ActiveRollRootMotionRotation = FRotator(0.f, GetActorRotation().Yaw, 0.f);
	}


	UE_LOG(LogTemp, Warning, TEXT("[RollDebug][ServerBeforeMulticast] Char=%s ActorYaw=%.2f TargetYaw=%.2f Facing=%s"),
		*GetNameSafe(this),
		GetActorRotation().Yaw,
		TargetRotation.Yaw,
		GetFacingSourceText(FacingSource));

	bStartingRollMontage = true;
	MulticastPlayNetworkedMontage(RollMontage, 1.f, NAME_None, false, true, true);
	bStartingRollMontage = false;
}

void AMainCharacterBase::ServerApplyInputRotation_Implementation(FRotator DesiredRotation, float RotationInterpSpeed)
{
	if (ActionState == ECharacterActionState::Stunned ||
		ActionState == ECharacterActionState::Dead ||
		ActionState == ECharacterActionState::InAir ||
		!ShouldRotateToInputForWeaponAction())
	{
		return;
	}

	ApplyInputRotation(DesiredRotation, RotationInterpSpeed);
}

void AMainCharacterBase::SendStrafeFacingToServer(bool bForce)
{
	if (HasAuthority() ||
		RotationMode != ERotationMode::Strafe ||
		IsValid(LockOnTarget) ||
		bIsAiming)
	{
		return;
	}

	AController* CurrentController = GetController();
	if (!CurrentController)
	{
		return;
	}

	const float DesiredYaw = FRotator::NormalizeAxis(CurrentController->GetControlRotation().Yaw);
	const bool bYawChangedEnough = !bHasSentStrafeFacingYaw ||
		FMath::Abs(FRotator::NormalizeAxis(DesiredYaw - LastSentStrafeFacingYaw)) >= 1.f;

	if (!bForce && !bYawChangedEnough)
	{
		return;
	}

	bHasSentStrafeFacingYaw = true;
	LastSentStrafeFacingYaw = DesiredYaw;
	ServerApplyStrafeFacing(FRotator(0.f, DesiredYaw, 0.f));
}

void AMainCharacterBase::ApplyAuthoritativeStrafeFacing(const FRotator& DesiredRotation)
{
	const FRotator SanitizedRotation(0.f, DesiredRotation.Yaw, 0.f);

	if (AController* CurrentController = GetController())
	{
		FRotator ControlRotation = CurrentController->GetControlRotation();
		ControlRotation.Yaw = SanitizedRotation.Yaw;
		CurrentController->SetControlRotation(ControlRotation);
	}

	bSmoothRotating = false;
	FacingSource = EFacingSource::ControllerRotation;
}

FTransform AMainCharacterBase::ProcessRollRootMotionPreConvertToWorld(const FTransform& InLocalRootMotion, UCharacterMovementComponent* MovementComponent, float DeltaSeconds)
{
	if (ActionState != ECharacterActionState::Rolling ||
		!RollMontage ||
		!GetMesh())
	{
		return InLocalRootMotion;
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance || !AnimInstance->Montage_IsPlaying(RollMontage))
	{
		return InLocalRootMotion;
	}

	FTransform AdjustedRootMotion = InLocalRootMotion;

	const FQuat ActorYawQuat = FRotator(0.f, GetActorRotation().Yaw, 0.f).Quaternion();
	const FQuat RollYawQuat = FRotator(0.f, ActiveRollRootMotionRotation.Yaw, 0.f).Quaternion();

	const FVector RollWorldTranslation = RollYawQuat.RotateVector(InLocalRootMotion.GetTranslation());
	const FVector ActorLocalTranslation = ActorYawQuat.Inverse().RotateVector(RollWorldTranslation);

	AdjustedRootMotion.SetTranslation(ActorLocalTranslation);

	// 몽타주 자체 회전은 제거. 캐릭터 회전은 네 코드의 FacingSource 쪽에서만 제어.
	AdjustedRootMotion.SetRotation(FQuat::Identity);

	return AdjustedRootMotion;
}

void AMainCharacterBase::ServerApplyStrafeFacing_Implementation(FRotator DesiredRotation)
{
	if (RotationMode != ERotationMode::Strafe ||
		IsValid(LockOnTarget) ||
		bIsAiming ||
		ActionState == ECharacterActionState::Stunned ||
		ActionState == ECharacterActionState::Dead ||
		ActionState == ECharacterActionState::InAir ||
		ActionState == ECharacterActionState::Rolling)
	{
		return;
	}

	ApplyAuthoritativeStrafeFacing(DesiredRotation);
}

void AMainCharacterBase::ServerPlayNetworkedMontage_Implementation(UAnimMontage* Montage, float PlayRate, FName StartSection, bool bFreezeAtEnd, bool bSkipOwnerIfLocallyControlled, bool bStopAllMontages)
{
	MulticastPlayNetworkedMontage(Montage, PlayRate, StartSection, bFreezeAtEnd, bSkipOwnerIfLocallyControlled, bStopAllMontages);
}

void AMainCharacterBase::MulticastPlayNetworkedMontage_Implementation(UAnimMontage* Montage, float PlayRate, FName StartSection, bool bFreezeAtEnd, bool bSkipOwnerIfLocallyControlled, bool bStopAllMontages)
{
	if (bSkipOwnerIfLocallyControlled && !HasAuthority() && IsLocallyControlled())
	{
		return;
	}

	PlayMontageLocally(Montage, PlayRate, StartSection, bFreezeAtEnd, bStopAllMontages);
}

void AMainCharacterBase::ServerStopNetworkedMontage_Implementation(UAnimMontage* Montage, float BlendOutTime)
{
	MulticastStopNetworkedMontage(Montage, BlendOutTime);
}

void AMainCharacterBase::MulticastStopNetworkedMontage_Implementation(UAnimMontage* Montage, float BlendOutTime)
{
	if (!HasAuthority() && IsLocallyControlled())
	{
		return;
	}

	StopMontageLocally(Montage, BlendOutTime);
}

void AMainCharacterBase::MulticastApplyMontageHitStop_Implementation(UAnimMontage* Montage, float Duration, float PausedPlayRate)
{
	ApplyLocalMontageHitStop(Montage, Duration, PausedPlayRate);
}

void AMainCharacterBase::RotateToInput()
{
	RotateToInput(10.f);
}

void AMainCharacterBase::RotateToInput(float RotationInterpSpeed)
{
	if (IsValid(LockOnTarget))
	{
		bSmoothRotating = false;
		return;
	}

	FRotator DesiredRotation = FRotator::ZeroRotator;
	if (GetDesiredMoveInputRotation(DesiredRotation))
	{
		ApplyInputRotation(DesiredRotation, RotationInterpSpeed);

		if (!HasAuthority())
		{
			ServerApplyInputRotation(DesiredRotation, RotationInterpSpeed);
		}
	}
}

void AMainCharacterBase::RotateToControlYaw(float RotationInterpSpeed)
{
	if (IsValid(LockOnTarget) || bIsAiming)
	{
		bSmoothRotating = false;
		return;
	}

	AController* CurrentController = GetController();
	if (!CurrentController)
	{
		return;
	}

	const FRotator DesiredRotation(0.f, CurrentController->GetControlRotation().Yaw, 0.f);
	ApplyInputRotation(DesiredRotation, RotationInterpSpeed);

	if (!HasAuthority())
	{
		ServerApplyInputRotation(DesiredRotation, RotationInterpSpeed);
	}
}

// Action execution
void AMainCharacterBase::OnRollPressed()
{
	PressBufferedInput(EPendingInputAction::Roll);

	FRotator PressedRollRotation = FRotator::ZeroRotator;
	if (GetDesiredMoveInputRotation(PressedRollRotation))
	{
		QueuedRollRotation = FRotator(0.f, PressedRollRotation.Yaw, 0.f);
		bHasQueuedRollRotation = true;
	}

	if (CanDoAction())
	{
		Roll();
	}
}

void AMainCharacterBase::OnRollReleased()
{
	ReleaseBufferedInput(EPendingInputAction::Roll);
}

void AMainCharacterBase::Roll()
{
	if (!RollMontage) return;

	if (!CanDoAction())
	{
		const FRotator ControlRot = Controller ? Controller->GetControlRotation() : GetActorRotation();
		UE_LOG(LogTemp, Warning, TEXT("[RollDebug][RollBlocked] Char=%s Role=%s HasAuthority=%d Local=%d ActionState=%d CanAction=%d Input=(%.2f, %.2f) ActorYaw=%.2f TargetYaw=%.2f ControlYaw=%.2f Facing=%s"),
			*GetNameSafe(this),
			GetRoleText(this),
			HasAuthority() ? 1 : 0,
			IsLocallyControlled() ? 1 : 0,
			static_cast<int32>(ActionState),
			bCanAction ? 1 : 0,
			CachedMoveInput.X,
			CachedMoveInput.Y,
			GetActorRotation().Yaw,
			TargetRotation.Yaw,
			ControlRot.Yaw,
			GetFacingSourceText(FacingSource));
		return;
	}

	if (!StatComponent || !StatComponent->ConsumeStamina(RollStaminaCost))
	{
		bPendingRollRecoveryDeferral = false;
		return;
	}

	const bool bDeferRecoveryForThisRoll = false;
	bPendingRollRecoveryDeferral = false;
	bDeferRollRecoveryUntilRollEnd = false;

	ConsumeBufferedInput(EPendingInputAction::Roll);
	SetCharacterState(ECharacterActionState::Rolling);
	CancelEquippedActions();

	FRotator DesiredRollRotation = FRotator::ZeroRotator;
	bool bHasDesiredRollRotation = false;
	bool bUseImmediateRollRotation = false;
	bool bUseSmoothRollRotation = false;
	EFacingSource NewFacingSource = EFacingSource::InputDirection;

	if (IsValid(LockOnTarget))
	{
		if (bHasQueuedRollRotation)
		{
			DesiredRollRotation = QueuedRollRotation;
			bHasDesiredRollRotation = true;
			bUseImmediateRollRotation = true;
			NewFacingSource = EFacingSource::RollDirection;

			bHasQueuedRollRotation = false;
		}
		else if (GetDesiredMoveInputRotation(DesiredRollRotation))
		{
			bHasDesiredRollRotation = true;
			bUseImmediateRollRotation = true;
			NewFacingSource = EFacingSource::RollDirection;
		}
		else if (IsValid(LockOnTarget))
		{
			// 락온 상태에서 입력 없이 구르면 몬스터/컨트롤러 방향
			const FRotator ControlRot = Controller ? Controller->GetControlRotation() : GetActorRotation();

			DesiredRollRotation = FRotator(0.f, ControlRot.Yaw, 0.f);
			bHasDesiredRollRotation = true;
			bUseImmediateRollRotation = false;
			NewFacingSource = EFacingSource::ControllerRotation;

			bHasQueuedRollRotation = false;
		}
	}
	else
	{
		const bool bUseRollDirectionFacing = bIsAiming || RotationMode == ERotationMode::Strafe;

		if (bHasQueuedRollRotation)
		{
			DesiredRollRotation = QueuedRollRotation;
			bHasDesiredRollRotation = true;
			bUseSmoothRollRotation = true;
			NewFacingSource = bUseRollDirectionFacing ? EFacingSource::RollDirection : EFacingSource::InputDirection;

			bHasQueuedRollRotation = false;
		}
		else if (GetDesiredMoveInputRotation(DesiredRollRotation))
		{
			bHasDesiredRollRotation = true;
			bUseSmoothRollRotation = true;
			NewFacingSource = bUseRollDirectionFacing ? EFacingSource::RollDirection : EFacingSource::InputDirection;
		}
	}

	const FRotator ControlRot = Controller ? Controller->GetControlRotation() : FRotator::ZeroRotator;
	UE_LOG(LogTemp, Warning, TEXT("[RollDebug][LocalStartRoll] Char=%s Role=%s HasAuthority=%d Local=%d Input=(%.2f, %.2f) ActorYaw=%.2f TargetYaw=%.2f ControlYaw=%.2f DesiredYaw=%.2f HasDesired=%d Immediate=%d Smooth=%d NewFacing=%s DeferRecovery=%d LockOn=%s"),
		*GetNameSafe(this),
		GetRoleText(this),
		HasAuthority() ? 1 : 0,
		IsLocallyControlled() ? 1 : 0,
		CachedMoveInput.X,
		CachedMoveInput.Y,
		GetActorRotation().Yaw,
		TargetRotation.Yaw,
		ControlRot.Yaw,
		DesiredRollRotation.Yaw,
		bHasDesiredRollRotation ? 1 : 0,
		bUseImmediateRollRotation ? 1 : 0,
		bUseSmoothRollRotation ? 1 : 0,
		GetFacingSourceText(NewFacingSource),
		bDeferRecoveryForThisRoll ? 1 : 0,
		*GetNameSafe(LockOnTarget));

	ApplyRollFacing(DesiredRollRotation, bHasDesiredRollRotation, bUseImmediateRollRotation, bUseSmoothRollRotation, NewFacingSource);

	if (bHasDesiredRollRotation)
	{
		ActiveRollRootMotionRotation = FRotator(0.f, DesiredRollRotation.Yaw, 0.f);
	}
	else
	{
		ActiveRollRootMotionRotation = FRotator(0.f, GetActorRotation().Yaw, 0.f);
	}


	if (!HasAuthority())
	{
		ServerStartRoll(DesiredRollRotation, bHasDesiredRollRotation, bUseImmediateRollRotation, bUseSmoothRollRotation, NewFacingSource, bDeferRecoveryForThisRoll);
		bStartingRollMontage = true;
		PlayMontageLocally(RollMontage);
		bStartingRollMontage = false;
		return;
	}

	bStartingRollMontage = true;
	PlayNetworkedMontage(RollMontage, 1.f, NAME_None, false, true, true);
	bStartingRollMontage = false;
}



void AMainCharacterBase::OnPrimaryPressed()
{
	PressBufferedInput(EPendingInputAction::Primary);
	StartPrimaryAction();
}

void AMainCharacterBase::OnPrimaryReleased()
{
	ReleaseBufferedInput(EPendingInputAction::Primary);
	StopPrimaryAction();
}

void AMainCharacterBase::OnSecondaryPressed()
{
	PressBufferedInput(EPendingInputAction::Secondary);
	StartSecondaryAction();
}

void AMainCharacterBase::OnSecondaryReleased()
{
	ReleaseBufferedInput(EPendingInputAction::Secondary);
	StopSecondaryAction();
}

void AMainCharacterBase::StartPrimaryAction()
{
	UE_LOG(LogTemp, Warning, TEXT("[PrimaryDebug][Enter] Char=%s Role=%s HasAuthority=%d Local=%d bCanAction=%d ActiveLocks=%d RightWeapon=%s"),
		*GetNameSafe(this),
		GetRoleText(this),
		HasAuthority() ? 1 : 0,
		IsLocallyControlled() ? 1 : 0,
		bCanAction ? 1 : 0,
		ActiveMontageLockCount,
		*GetNameSafe(RightWeapon));

	if (!StatComponent || bIsDead)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PrimaryDebug][Blocked] Reason=NoStatOrDead Stat=%d Dead=%d"),
			StatComponent != nullptr,
			bIsDead ? 1 : 0);
		return;
	}

	if (!CanDoAction())
	{
		UE_LOG(LogTemp, Warning, TEXT("[PrimaryDebug][Blocked] Reason=CanDoActionFalse ActionState=%d bCanAction=%d ActiveLocks=%d"),
			static_cast<int32>(ActionState),
			bCanAction ? 1 : 0,
			ActiveMontageLockCount);
		return;
	}

	if (!RightWeapon || !RightWeapon->CanStartAction(EEquipmentActionType::Primary))
	{
		UE_LOG(LogTemp, Warning, TEXT("[PrimaryDebug][Blocked] Reason=WeaponCantStart RightWeapon=%s"),
			*GetNameSafe(RightWeapon));
		return;
	}

	// 클라: 서버에 먼저 요청
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[PrimaryDebug][ClientPredict] Sending ServerStartPrimaryAction"));
		ServerStartPrimaryAction();

		// 클라 로컬 크로스헤어/차징/연출용
		ConsumeBufferedInput(EPendingInputAction::Primary);

		if (ShouldRotateToInputForWeaponAction())
		{
			RotateToInput();
		}
		else
		{
			bSmoothRotating = false;
		}

		RightWeapon->StartAction(EEquipmentActionType::Primary);
		return;
	}

	// 서버: 실제 스태미나 검사/소모
	const float StaminaCost = RightWeapon->GetStaminaCostForAction(EEquipmentActionType::Primary);
	UE_LOG(LogTemp, Warning, TEXT("[Primary] SERVER Consume StaminaCost=%.2f"), StaminaCost);
	if (StaminaCost > KINDA_SMALL_NUMBER)
	{
		if (!StatComponent->ConsumeStamina(StaminaCost))
		{
			UE_LOG(LogTemp, Warning, TEXT("[PrimaryDebug][Blocked] Reason=NoStamina Cost=%.2f"), StaminaCost);
			return;
		}
	}

	ConsumeBufferedInput(EPendingInputAction::Primary);

	if (ShouldRotateToInputForWeaponAction())
	{
		RotateToInput();
	}
	else
	{
		bSmoothRotating = false;
	}

	RightWeapon->StartAction(EEquipmentActionType::Primary);
}

void AMainCharacterBase::StartSecondaryAction()
{
	if (!StatComponent || bIsDead)
	{
		return;
	}

	if (!CanDoAction())
	{
		return;
	}

	ACharacterEquipmentBase* ActionEquipment = LeftWeapon ? LeftWeapon : RightWeapon;
	if (!ActionEquipment || !ActionEquipment->CanStartAction(EEquipmentActionType::Secondary))
	{
		return;
	}

	// 클라: 스태미나 소모 없이 로컬 액션 실행
	if (!HasAuthority())
	{
		ConsumeBufferedInput(EPendingInputAction::Secondary);

		ActionEquipment->StartAction(EEquipmentActionType::Secondary);

		ServerStartSecondaryAction();
		return;
	}

	// 서버: 실제 스태미나 검사/소모
	const float StaminaCost = ActionEquipment->GetStaminaCostForAction(EEquipmentActionType::Secondary);
	if (StaminaCost > KINDA_SMALL_NUMBER)
	{
		if (!StatComponent->ConsumeStamina(StaminaCost))
		{
			return;
		}
	}

	ConsumeBufferedInput(EPendingInputAction::Secondary);

	ActionEquipment->StartAction(EEquipmentActionType::Secondary);
}

void AMainCharacterBase::OnAbilityPressed()
{
	PressBufferedInput(EPendingInputAction::Ability);
	StartAbilityAction();
}

void AMainCharacterBase::OnAbilityReleased()
{
	ReleaseBufferedInput(EPendingInputAction::Ability);
	StopAbilityAction();
}

void AMainCharacterBase::StartAbilityAction()
{
	if (!CanDoAction()) return;

	if (!RightWeapon || !RightWeapon->CanStartAction(EEquipmentActionType::Ability))
	{
		return;
	}

	const float StaminaCost = RightWeapon->GetStaminaCostForAction(EEquipmentActionType::Ability);
	if (StaminaCost > KINDA_SMALL_NUMBER)
	{
		if (!StatComponent || !StatComponent->ConsumeStamina(StaminaCost))
		{
			return;
		}
	}

	RestoreAbilityRootMotionControl();
	ConsumeBufferedInput(EPendingInputAction::Ability);

	if (ShouldRotateToInputForWeaponAction())
	{
		RotateToInput();
	}
	else
	{
		bSmoothRotating = false;
	}

	RightWeapon->StartAction(EEquipmentActionType::Ability);
}

void AMainCharacterBase::StopAbilityAction()
{
	if (RightWeapon)
	{
		RightWeapon->StopAction(EEquipmentActionType::Ability);
	}
}

void AMainCharacterBase::StopPrimaryAction()
{
	if (RightWeapon)
	{
		RightWeapon->StopAction(EEquipmentActionType::Primary);
	}
}

void AMainCharacterBase::StopSecondaryAction()
{
	if (LeftWeapon)
	{
		if (!HasAuthority() && Cast<AShieldBase>(LeftWeapon))
		{
			ServerSetShieldGuarding(false);
		}

		LeftWeapon->StopAction(EEquipmentActionType::Secondary);
	}
	else if (RightWeapon)
	{
		RightWeapon->StopAction(EEquipmentActionType::Secondary);
	}
}

void AMainCharacterBase::Crouching()
{
	if (bIsDead)
	{
		return;
	}

	if (!IsCrouched())
	{
		Stance = EStance::Crouching;
		Crouch();
	}
	else
	{
		Stance = EStance::Standing;
		UnCrouch();
	}
}

void AMainCharacterBase::SetRotationMode(ERotationMode BattleMode, bool bRequestServer)
{
	bool bAppliedMode = true;
	const bool bPreserveRollFacing =
		ActionState == ECharacterActionState::Rolling ||
		FacingSource == EFacingSource::RollDirection ||
		FacingSource == EFacingSource::RollRecovery ||
		(RollMontage &&
		 GetMesh() &&
		 GetMesh()->GetAnimInstance() &&
		 GetMesh()->GetAnimInstance()->Montage_IsPlaying(RollMontage));
	const EFacingSource PreviousFacingSource = FacingSource;

	switch (BattleMode)
	{
	case ERotationMode::OrientToMovement:
		GetCharacterMovement()->bOrientRotationToMovement = true;
		bUseControllerRotationYaw = false;
		FacingSource = bPreserveRollFacing ? PreviousFacingSource : EFacingSource::None;
		bHasSentStrafeFacingYaw = false;
		SpringArm->SetRelativeLocation(FVector::ZeroVector);
		TargetCameraSocketOffset = DefaultCameraSocketOffset;
		Camera->SetRelativeLocation(FVector::ZeroVector);
		RotationMode = ERotationMode::OrientToMovement;
		break;
	case ERotationMode::Strafe:
		GetCharacterMovement()->bOrientRotationToMovement = false;
		bUseControllerRotationYaw = false;
		FacingSource = bPreserveRollFacing ? PreviousFacingSource : EFacingSource::ControllerRotation;
		SpringArm->SetRelativeLocation(FVector::ZeroVector);
		TargetCameraSocketOffset = StrafeCameraSocketOffset;
		Camera->SetRelativeLocation(FVector::ZeroVector);
		RotationMode = ERotationMode::Strafe;
		break;
	default:
		bAppliedMode = false;
		break;
	}

	if (bAppliedMode && !HasAuthority() && bRequestServer)
	{
		const FRotator ControlRotation = Controller ? Controller->GetControlRotation() : GetActorRotation();
		ServerSetRotationMode(BattleMode, ControlRotation);

		if (BattleMode == ERotationMode::Strafe)
		{
			SendStrafeFacingToServer(true);
		}
	}
}

void AMainCharacterBase::ChangeRotationMode()
{
	if (bIsDead)
	{
		return;
	}

	if (IsValid(LockOnTarget))
	{
		return;
	}

	if (RotationMode == ERotationMode::OrientToMovement)
	{
		SetRotationMode(ERotationMode::Strafe);
	}
	else
	{
		SetRotationMode(ERotationMode::OrientToMovement);
	}
}

void AMainCharacterBase::CalcDirectionalValueFromVelocity() const
{
	const FVector Vel = GetVelocity();
	const FVector Vel2D(Vel.X, Vel.Y, 0.f);

	const FVector NormVel = Vel2D.GetSafeNormal();

	const FVector Fwd = GetActorForwardVector().GetSafeNormal2D();
	const FVector Right = GetActorRightVector().GetSafeNormal2D();

	const float DotForward = FVector::DotProduct(NormVel, Fwd);      // [-1, 1]
	const float DotRightAbs = FMath::Abs(FVector::DotProduct(NormVel, Right)); // [0, 1]

	FVector Speeds;
	switch (Gait)
	{
	case EGait::Walk:   Speeds = WalkSpeeds;   break;
	case EGait::Run:    Speeds = RunSpeeds;    break;
	case EGait::Sprint: Speeds = SprintSpeeds; break;
	default:            Speeds = WalkSpeeds;   break;
	}

	if (bIsCrouched)
	{
		Speeds = CrouchSpeed;
	}
	if (bIsAiming)
	{
		Speeds = AimSpeeds;
	}
	// 4) MapRange 1: 전/후 블렌딩 (InRangeA=1, InRangeB=-1, OutRangeA=X, OutRangeB=Z)
	const float BaseForwardBackward = FMath::GetMappedRangeValueClamped(FVector2D( 1.f, -1.f), FVector2D( Speeds.X, Speeds.Z), DotForward);

	// 5) MapRange 2: 측면 블렌딩 (InRange 0..1, OutRangeA=base, OutRangeB=Y)
	const float Result = FMath::GetMappedRangeValueClamped(FVector2D(0.f, 1.f), FVector2D( BaseForwardBackward, Speeds.Y), DotRightAbs);

	MovementComp->MaxWalkSpeed = Result;
	MovementComp->MaxWalkSpeedCrouched = Result;
}

void AMainCharacterBase::UpdateMovementParameters()
{
	float Speed2D = FVector::VectorPlaneProject(MovementComp->Velocity, FVector::UpVector).Size();

	float AccelerationAlpha = FMath::GetMappedRangeValueClamped(
		FVector2D(300, SprintSpeeds.X),
		FVector2D(800.f, 300.f),
		Speed2D
	);

	float GroundFrictionAlpha = FMath::GetMappedRangeValueClamped(
		FVector2D(0, SprintSpeeds.X),
		FVector2D(5, 3.f),
		Speed2D
	);
	float tempMA = MovementComp->MaxAcceleration;
	float tempGF = MovementComp->GroundFriction;
	switch (Gait)
	{
	case EGait::Walk:
		tempMA = 800.f;
		tempGF = 5.f;
		break;
	case EGait::Run:
		tempMA = 800.f;
		tempGF = 5.f;
		break;
	case EGait::Sprint:
		tempMA = AccelerationAlpha;
		tempGF = GroundFrictionAlpha;
		break;
	default:
		break;
	}
	GetPendingMovementInputVector() != FVector::ZeroVector ? MovementComp->BrakingDecelerationWalking = 500 : MovementComp->BrakingDecelerationWalking = 2000;

	MovementComp->MaxAcceleration = tempMA;
	MovementComp->GroundFriction = tempGF;

}

void AMainCharacterBase::UpdateAimOffsetValues()
{
	constexpr float AimOffsetMin = -90.f;
	constexpr float AimOffsetMax = 90.f;

	FRotator AimRotation = GetBaseAimRotation();
	if (!IsLocallyControlled())
	{
		AimRotation.Yaw = ReplicatedAimYaw;
	}

	const FRotator BodyRotation = GetActorRotation();
	const FRotator DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(AimRotation, BodyRotation);

	AOYaw = FMath::Clamp(DeltaRotation.Yaw, AimOffsetMin, AimOffsetMax);
	AOPitch = FMath::Clamp(FRotator::NormalizeAxis(DeltaRotation.Pitch), AimOffsetMin, AimOffsetMax);
}
