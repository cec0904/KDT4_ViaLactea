#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CharacterEnums.h"
#include "MainCharacterBase.generated.h"

class ASwordBase;
class AShieldBase;
class ABowBase;
class ACharacterEquipmentBase;
class USkeletalMeshComponent;
class AVL_AICharacterBase;
class UVL_StatComponent;

struct FBufferedInputState
{
	bool bPressed = false;
	bool bConsumed = false;
	double BufferUntilTime = 0.0;
};

UCLASS()
class VIALACTEA_API AMainCharacterBase : public ACharacter
{
	GENERATED_BODY()
	
public:
	AMainCharacterBase();

	// Accessors
	FORCEINLINE EGait GetGait() const { return Gait; }
	FORCEINLINE EStance GetStance() const { return Stance; }
	FORCEINLINE EMovementState GetMovementState() const { return MovementState; }
	FORCEINLINE ERotationMode GetRotationMode() const { return RotationMode; }
	FORCEINLINE bool GetisEquip() const { return bIsWeaponEquip; }
	FORCEINLINE float GetAOYaw() const { return AOYaw; }
	FORCEINLINE float GetAOPitch() const { return AOPitch; }
	FORCEINLINE bool IsAiming() const { return bIsAiming; }
	FORCEINLINE bool IsJustLanded() const { return bJustLanded; }
	FORCEINLINE ECharacterActionState GetActionState() const { return ActionState; }
	FORCEINLINE bool IsActionWindowOpen() const { return bCanAction; }

	//[KDH] 추가 2026.04.20
	float GetCurrentHP() const;
	float GetMaxHP() const;
	FORCEINLINE bool IsDead() const { return bIsDead; }
	float GetHPRatio() const;
	float GetStaminaRatio() const;
	UFUNCTION(BlueprintCallable, Category = "Damage|Death")
	void TriggerDeathRagdoll();

	UFUNCTION(BlueprintCallable, Category = "Stat")
	UVL_StatComponent* GetStatComponent() const { return StatComponent; }
	//KDH 2026.04.23 추가
	UFUNCTION(Server, Reliable)
	void ServerStartPrimaryAction();

	UFUNCTION(Server, Reliable)
	void ServerStartSecondaryAction();

	//KDH [2026.04.14] 추가
	ABowBase* GetEquippedBow() const;
	//KDH 2026.04.29 추가
	UFUNCTION(Server, Reliable)
	void ServerConsumeJumpStamina();

	// Equipment API
	EWeaponAnimType GetWeaponAnimType() const;
	bool ShouldUseLeftHandIK() const;
	bool ShouldUseRightHandIK() const;
	FTransform GetLeftHandIKTransformCS() const;
	FTransform GetRightHandIKTransformCS() const;
	UFUNCTION(BlueprintPure, Category = "Animation|IK")
	FTransform GetLeftHandIKTransformCSForMesh(const USkeletalMeshComponent* MeshComponent) const;
	UFUNCTION(BlueprintPure, Category = "Animation|IK")
	FTransform GetRightHandIKTransformCSForMesh(const USkeletalMeshComponent* MeshComponent) const;
	void SetWeaponIKState(EWeaponTarget Target, bool bEnabled);
	UFUNCTION(BlueprintCallable, Category = "Animation|Trail")
	void SetWeaponTrailState(EWeaponTarget Target, bool bEnabled);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void EquipItem(ACharacterEquipmentBase* NewEquipment);

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void Unequipment(ACharacterEquipmentBase* Equipment);

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	bool HandleEquipmentItem(FName ItemID, TSubclassOf<ACharacterEquipmentBase> EquipmentClass);

	UFUNCTION(Server, Reliable)
	void ServerHandleEquipmentItem(FName ItemID, TSubclassOf<ACharacterEquipmentBase> EquipmentClass);

	// Interaction API
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetCurrentInteractorActor(AActor* NewActor);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	AActor* GetCurrentInteractActor() const;

	// Action API
	UFUNCTION(BlueprintCallable, Category = "Attack")
	void StartPrimaryAction();

	UFUNCTION(BlueprintCallable, Category = "Attack")
	void StartSecondaryAction();

	UFUNCTION(BlueprintCallable, Category = "Attack")
	void StopPrimaryAction();

	UFUNCTION(BlueprintCallable, Category = "Attack")
	void StopSecondaryAction();

	UFUNCTION(BlueprintCallable, Category = "Attack")
	void StartAbilityAction();

	UFUNCTION(BlueprintCallable, Category = "Attack")
	void StopAbilityAction();

	UFUNCTION(BlueprintCallable, Category = "AimOffset")
	void UpdateAimOffsetValues();

	UPROPERTY(Transient)
	FRotator ActiveRollRootMotionRotation = FRotator::ZeroRotator;

	UPROPERTY(Transient)
	FRotator QueuedRollRotation = FRotator::ZeroRotator;

	UPROPERTY(Transient)
	bool bHasQueuedRollRotation = false;


	// State / animation hooks
	bool CanDoAction() const;
	void SetCanMove(bool bCan) { bCanMove = bCan; }
	void SetCanAction(bool bCan);
	void SetCharacterState(ECharacterActionState NewState);
	void ExitState(ECharacterActionState ExitingState);
	void OnBowZoomChanged(bool bZooming);
	void OnShieldGuardChanged(bool bGuarding);
	void OnShieldBlockChanged(bool bBlocking);
	void SetDefenseWindowState(EDefenseWindowType WindowType, bool bEnabled);
	void RotateToInput();
	void RotateToInput(float RotationInterpSpeed);
	void RotateToControlYaw(float RotationInterpSpeed = 20.f);
	void HandleAnimAction(EEquipMentHandleAction Action, EWeaponTarget Target);
	float PlayNetworkedMontage(class UAnimMontage* Montage, float PlayRate = 1.f, FName StartSection = NAME_None, bool bFreezeAtEnd = false, bool bSkipOwnerIfLocallyControlled = true, bool bStopAllMontages = true);
	float PlayPredictedWeaponMontageLocally(class UAnimMontage* Montage, float PlayRate = 1.f, FName StartSection = NAME_None, bool bFreezeAtEnd = false, bool bStopAllMontages = true);
	void StopNetworkedMontage(class UAnimMontage* Montage, float BlendOutTime = 0.25f);
	void RequestMontageHitStop(class UAnimMontage* Montage, float Duration, float PausedPlayRate);

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	// Lifecycle
	virtual void BeginPlay() override;
	virtual void Destroyed() override;
	virtual void Landed(const FHitResult& Hit) override;
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;

	// Components
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	class USpringArmComponent* SpringArm;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	class UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Visual")
	USkeletalMeshComponent* VisibleMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	FVector DefaultCameraSocketOffset = FVector(0.f, 50.f, 50.f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	FVector StrafeCameraSocketOffset = FVector(100.f, 100.f, 50.f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	float CameraSocketOffsetInterpSpeed = 8.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (ClampMin = "0.0"))
	float MinArmLength = 150.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (ClampMin = "0.0"))
	float MaxArmLength = 600.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (ClampMin = "1.0"))
	float ZoomStep = 50.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (ClampMin = "0.0"))
	float ArmLengthInterpSpeed = 10.f;

	// 활 조준 시 사용할 고정 암 길이
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (ClampMin = "0.0"))
	float AimArmLength = 200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (ClampMin = "0.0"))
	float StrafeFacingInterpSpeed = 12.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LockOn")
	FVector LockOnCameraSocketOffset = FVector(100.f, 100.f, 50.f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LockOn", meta = (ClampMin = "0.0"))
	float LockOnSearchRadius = 3000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LockOn", meta = (ClampMin = "0.0"))
	float LockOnBreakDistance = 3600.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LockOn", meta = (ClampMin = "0.0"))
	float LockOnInterpSpeed = 8.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LockOn", meta = (ClampMin = "0.0"))
	float LockOnRollRecoveryInterpSpeed = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LockOn", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LockOnMinViewDot = 0.1f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LockOn", meta = (ClampMin = "0.0"))
	float LockOnSwitchInputDeadZone = 4.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LockOn", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LockOnTargetHeightRatio = 0.66f;

	UPROPERTY()
	UCharacterMovementComponent* MovementComp;

	// Equipment references
	UPROPERTY(ReplicatedUsing = OnRep_RightWeapon, VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
	ACharacterEquipmentBase* RightWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_LeftWeapon, VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
	ACharacterEquipmentBase* LeftWeapon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment|Class")
	TSubclassOf<ASwordBase> SwordClass;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Equipment|Runtime")
	ASwordBase* Sword = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment|Class")
	TSubclassOf<AShieldBase> ShieldClass;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Equipment|Runtime")
	AShieldBase* Shield = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment|Class")
	TSubclassOf<ABowBase> BowClass;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Equipment|Runtime")
	ABowBase* Bow = nullptr;

	// Input assets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	class UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	class UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	class UInputAction* RotationAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	class UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	class UInputAction* RunAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	class UInputAction* SprintAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	class UInputAction* RightClick;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	class UInputAction* LeftClick;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	class UInputAction* AbilityAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	class UInputAction* RollAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	class UInputAction* CrouchAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	class UInputAction* ChangeRotationModeAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	class UInputAction* InputActionInteraction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	class UInputAction* LockOnAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	class UInputAction* CameraZoomAction;

	// Interaction / combat references
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	AActor* LockOnTarget = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LockOn")
	FRotator TargetControlRotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	EFacingSource FacingSource = EFacingSource::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge")
	class UAnimMontage* RollMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	class UAnimMontage* HitReactMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float HitReactPlayRate = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage|Death")
	class UAnimMontage* DeathMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage|Death", meta = (ClampMin = "0.1"))
	float DeathMontagePlayRate = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage|Death", meta = (ClampMin = "0.0"))
	float RespawnDelay = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage|Death", meta = (ClampMin = "0.0"))
	float CorpseDestroyDelayAfterRespawn = 1.f;

	UPROPERTY()
	AActor* CurrentInteractActor = nullptr;

	// Character state
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "State", meta = (AllowPrivateAccess = "true"))
	EGait Gait = EGait::Walk;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = "true"))
	EStance Stance = EStance::Standing;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = "true"))
	EMovementState MovementState = EMovementState::Idle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "State", meta = (AllowPrivateAccess = "true"))
	ERotationMode RotationMode = ERotationMode::OrientToMovement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	ECharacterActionState ActionState = ECharacterActionState::Idle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bCanAction = true;

	UPROPERTY(ReplicatedUsing = OnRep_IsDead, VisibleAnywhere, BlueprintReadOnly, Category = "Stat")
	bool bIsDead = false;

	//[KDH] 2026.04.20 추가
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stat")
	TObjectPtr<UVL_StatComponent> StatComponent = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stat")
	float InitialMaxHP = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stat")
	float InitialMaxStamina = 150.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stat|Cost")
	float JumpStaminaCost = 20.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stat|Cost")
	float AttackStaminaCost = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stat|Cost")
	float SprintStaminaCostPerSecond = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stat|Cost")
	float RollStaminaCost = 20.f;
	//

	// 데미지를 받을 때 이제 hp바를 움직일것인가
	// 0.1초마다 업데이트를 해줄것인가

	int32 ActiveMontageLockCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated)
	bool bIsWeaponEquip = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Equipment|State", meta = (DisplayName = "Current Weapon Anim Type"))
	EWeaponAnimType CurrentWeaponType = EWeaponAnimType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation|IK")
	bool bUseLeftHandIK = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation|IK")
	bool bUseRightHandIK = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation|IK")
	FTransform LeftHandIKTransformWS = FTransform::Identity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation|IK")
	FTransform RightHandIKTransformWS = FTransform::Identity;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Movement|State")
	bool bCanMove = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Movement|State")
	bool bIsAiming = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|State")
	bool bIsBlocking = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|State")
	bool bIsGuarding = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|State")
	bool bCanRun = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Defense|State")
	bool bIsInvincible = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Defense|State")
	bool bCanParry = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defense|Parry")
	float ParryGroggyAmount = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defense|Stamina", meta = (ClampMin = "0.0"))
	float ParryStaminaDamageMultiplier = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defense|Stamina", meta = (ClampMin = "0.0"))
	float BlockStaminaDamageMultiplier = 1.f;

	// Movement tuning
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|State")
	FVector WalkSpeeds = FVector(200.f, 180.f, 150.f);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|State")
	FVector RunSpeeds = FVector(500, 350.f, 300.f);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|State")
	FVector SprintSpeeds = FVector(700.f);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|State")
	FVector AimSpeeds = FVector(100.f, 90.f, 75.f);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|State")
	FVector CrouchSpeed = FVector(100.f, 90.f, 75.f);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|State")
	bool bJustLanded = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|State")
	FVector LandingVelocity = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AimOffset")
	float AOYaw = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AimOffset")
	float AOPitch = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "AimOffset")
	float ReplicatedAimYaw = 0.f;

	// Runtime input state
	FVector2D CachedMoveInput = FVector2D::ZeroVector;
	FBufferedInputState PrimaryInputState;
	FBufferedInputState AbilityInputState;
	FBufferedInputState SecondaryInputState;
	FBufferedInputState RunInputState;
	FBufferedInputState JumpInputState;
	FBufferedInputState RollInputState;
	bool bAbilityRootMotionReleased = false;
	FVector RollDebugLastLocation = FVector::ZeroVector;
	int32 RollDebugFrameCount = 0;
	bool bStartingRollMontage = false;
	bool bPendingRollRecoveryDeferral = false;
	bool bDeferRollRecoveryUntilRollEnd = false;
	class UAnimMontage* QueuedActionHandledMontage = nullptr;
	int32 InvincibleWindowCount = 0;
	int32 ParryWindowCount = 0;
	int32 BlockWindowCount = 0;

	static constexpr double InputGracePeriod = 0.3;

	FRotator TargetRotation = FRotator::ZeroRotator;
	float InputRotationInterpSpeed = 10.f;
	bool bSmoothRotating = false;
	FTimerHandle RetriggerableDelayHandle;
	FTimerHandle MontageHitStopTimerHandle;
	FTimerHandle RespawnTimerHandle;
	class UAnimMontage* ActiveHitStopMontage = nullptr;
	float StoredHitStopPlayRate = 1.0f;
	bool bHitStopMontagePaused = false;
	bool bDeathStateApplied = false;
	bool bDeathRagdollStarted = false;

	// Equipment helpers
	void BindEquipmentDelegates(ACharacterEquipmentBase* Equipment);
	ACharacterEquipmentBase* GetOrCreateEquipment(TSubclassOf<ACharacterEquipmentBase> EquipmentClass);
	void FinalizePendingEquipEquipment();
	void FinalizePendingUnequipEquipment();
	void QueuePendingEquipmentItem(FName ItemID, TSubclassOf<ACharacterEquipmentBase> EquipmentClass);
	bool ConsumePendingEquipmentItem();

	UPROPERTY()
	ACharacterEquipmentBase* PendingUnequipEquipment = nullptr;

	UPROPERTY()
	ACharacterEquipmentBase* PendingEquipEquipment = nullptr;

	UPROPERTY()
	FName PendingEquipmentItemID = NAME_None;

	UPROPERTY()
	TSubclassOf<ACharacterEquipmentBase> PendingEquipmentClass;

	// Input buffer helpers
	void CancelEquippedActions();
	bool CanBufferServerPrimaryActionRequest() const;
	bool BufferServerPrimaryActionRequest();
	bool TryConsumeQueuedCharacterAction();
	void ProcessPendingInputs();
	FBufferedInputState& GetBufferedInputState(EPendingInputAction Action);
	const FBufferedInputState& GetBufferedInputState(EPendingInputAction Action) const;
	void PressBufferedInput(EPendingInputAction Action);
	void ReleaseBufferedInput(EPendingInputAction Action);
	void ConsumeBufferedInput(EPendingInputAction Action);
	bool IsBufferedInputPressed(EPendingInputAction Action) const;
	bool IsBufferedInputActive(EPendingInputAction Action, double Now) const;
	static bool ShouldRepeatBufferedInputWhilePressed(EPendingInputAction Action);
	EPendingInputAction GetBufferedActionToExecute(double Now) const;
	void PrepareQueuedRollFacing();
	void BeginLockOnRollRecovery();
	void RecoverActionWindowIfIdle();
	bool CanUseRequestedGait(EGait DesiredGait) const;
	void RestorePostAirborneState();
	bool GetDesiredMoveInputRotation(FRotator& OutRotation) const;
	EFacingSource DetermineFacingSource() const;
	void ApplyRollFacing(const FRotator& DesiredRotation, bool bHasDesiredRotation, bool bUseImmediateRotation, bool bUseSmoothRotation, EFacingSource NewFacingSource);
	void ApplyInputRotation(const FRotator& DesiredRotation, float RotationInterpSpeed);
	void SetGaitState(EGait NewGait, bool bRequestServer = true);
	void SetAimingState(bool bNewAiming, bool bRequestServer = true);
	void UpdateEquipmentStateFromWeapons();
	bool IsCurrentAbilityMontageActive() const;
	bool ShouldRotateToInputForWeaponAction() const;
	void SendStrafeFacingToServer(bool bForce = false);
	void ApplyAuthoritativeStrafeFacing(const FRotator& DesiredRotation);
	FTransform ProcessRollRootMotionPreConvertToWorld(const FTransform& InLocalRootMotion, class UCharacterMovementComponent* MovementComponent, float DeltaSeconds);
	void ReleaseAbilityRootMotionControl();
	void RestoreAbilityRootMotionControl();
	void UpdateWeaponIKData();
	void UpdateCameraSocketOffset(float DeltaTime);
	void UpdateLockOn(float DeltaTime);
	void UpdateCharacterFacing(float DeltaTime);
	void ApplyLocalMontageHitStop(class UAnimMontage* Montage, float Duration, float PausedPlayRate);
	void RestoreMontageHitStop();
	void ToggleLockOn();
	void EnterLockOn(AActor* NewTarget);
	void ExitLockOn();
	AActor* FindBestLockOnTarget() const;
	AActor* FindNextLockOnTarget(const FVector2D& InputDirection) const;
	FVector GetLockOnFocusLocation(const AActor* Target) const;
	bool IsLockOnTargetValid(const AActor* Target) const;
	void HandleLockOnSwitchInput(const FVector2D& SwitchInput);
	void ApplyWeaponIKState(EWeaponTarget Target, int32 Delta);
	float PlayMontageLocally(class UAnimMontage* Montage, float PlayRate = 1.f, FName StartSection = NAME_None, bool bFreezeAtEnd = false, bool bStopAllMontages = true);
	void StopMontageLocally(class UAnimMontage* Montage, float BlendOutTime = 0.25f);

	int32 LeftHandIKStateCount = 0;
	int32 RightHandIKStateCount = 0;
	FVector CurrentCameraSocketOffset = FVector::ZeroVector;
	FVector TargetCameraSocketOffset = FVector::ZeroVector;
	float TargetArmLength = 300.f;
	ERotationMode RotationModeBeforeLockOn = ERotationMode::OrientToMovement;
	FVector2D LockOnSwitchAccumulatedInput = FVector2D::ZeroVector;
	bool bLockOnSwitchInputWaitingForNeutral = false;
	bool bHasSentStrafeFacingYaw = false;
	float LastSentStrafeFacingYaw = 0.f;

	// Movement helpers
	void UpdateSmoothRotation(float DeltaTime);
	void Rotation(const struct FInputActionValue& Value);
	void Move(const struct FInputActionValue& Value);
	void OnLockOnPressed();
	void OnCameraZoom(const struct FInputActionValue& Value);
	void ClearMoveInput();
	void StartRun();
	void StopRun();
	void StartSprint();
	void StopSprint();

	UFUNCTION(Server, Reliable)
	void ServerSetGait(EGait NewGait);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bNewAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetRotationMode(ERotationMode NewRotationMode, FRotator ControlRotation);

	UFUNCTION(Server, Reliable)
	void ServerSetShieldGuarding(bool bGuarding);

	UFUNCTION(Server, Reliable)
	void ServerApplyRollFacing(FRotator DesiredRotation, bool bHasDesiredRotation, bool bUseImmediateRotation, bool bUseSmoothRotation, EFacingSource NewFacingSource);

	UFUNCTION(Server, Reliable)
	void ServerStartRoll(FRotator DesiredRotation, bool bHasDesiredRotation, bool bUseImmediateRotation, bool bUseSmoothRotation, EFacingSource NewFacingSource, bool bDeferRecoveryUntilRollEnd);

	UFUNCTION(Server, Reliable)
	void ServerApplyInputRotation(FRotator DesiredRotation, float RotationInterpSpeed);

	UFUNCTION(Server, Unreliable)
	void ServerApplyStrafeFacing(FRotator DesiredRotation);

	UFUNCTION(Server, Reliable)
	void ServerPlayNetworkedMontage(class UAnimMontage* Montage, float PlayRate, FName StartSection, bool bFreezeAtEnd, bool bSkipOwnerIfLocallyControlled, bool bStopAllMontages);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayNetworkedMontage(class UAnimMontage* Montage, float PlayRate, FName StartSection, bool bFreezeAtEnd, bool bSkipOwnerIfLocallyControlled, bool bStopAllMontages);

	UFUNCTION(Server, Reliable)
	void ServerStopNetworkedMontage(class UAnimMontage* Montage, float BlendOutTime);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastStopNetworkedMontage(class UAnimMontage* Montage, float BlendOutTime);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastApplyMontageHitStop(class UAnimMontage* Montage, float Duration, float PausedPlayRate);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastTriggerDeathRagdoll();
	void Crouching();
	void SetRotationMode(ERotationMode BattleMode, bool bRequestServer = true);
	void ChangeRotationMode();
	void CalcDirectionalValueFromVelocity() const;
	void UpdateMovementParameters();

	// Action helpers
	void OnJumpPressed();
	void OnJumpReleased();
	void StartJump();
	void StopJump();
	void OnRunPressed();
	void OnRunReleased();
	void Roll();
	void OnRollPressed();
	void OnRollReleased();

	UFUNCTION()
	void OnGlobalMontageStarted(UAnimMontage* Montage);

	UFUNCTION()
	void OnGlobalMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnRep_IsDead();

	UFUNCTION()
	void OnRep_RightWeapon();

	UFUNCTION()
	void OnRep_LeftWeapon();

	void OnPrimaryPressed();
	void OnPrimaryReleased();
	void OnSecondaryPressed();
	void OnSecondaryReleased();
	void OnAbilityPressed();
	void OnAbilityReleased();
	void Interaction(const struct FInputActionValue& Value);

private:
	void ApplyDeathState();
	void ConfigureMeshForDeathRagdoll(USkeletalMeshComponent* MeshComponent, bool bDetachFromLeaderPose, bool bSimulatePhysics);
	void ClearBufferedInputs();
	void CleanupOwnedEquipment(float CleanupDelay);
	void ScheduleRespawn();
	void HandleRespawnTimerElapsed();
	void PlayHitReaction();
	AVL_AICharacterBase* FindParriedMonster(AController* EventInstigator, AActor* DamageCauser) const;
	void ApplyParryGroggyToAttacker(AController* EventInstigator, AActor* DamageCauser);
	float ConsumeDefenseStaminaFromDamage(float IncomingDamage, float DamageMultiplier, const TCHAR* DefenseLabel);

protected:
	UFUNCTION()
	void HandleStatDeath();

public:
	UFUNCTION(BlueprintCallable, Category = "Combat|Knockback")
	void ApplyKnockback(FVector LaunchVelocity);

	UFUNCTION(Server, Reliable)
	void Server_ApplyKnockback(FVector LaunchVelocity);

	UFUNCTION(Client, Reliable)
	void Client_ApplyKnockback(FVector LaunchVelocity);
};
