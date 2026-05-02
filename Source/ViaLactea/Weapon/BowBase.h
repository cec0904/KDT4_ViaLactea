// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RangeweaponBase.h"
#include "BowBase.generated.h"

// 줌 상태 변경 시 캐릭터에 통보하는 델리게이트
DECLARE_MULTICAST_DELEGATE_OneParam(FOnZoomChanged, bool);

/**
 * 활 무기 클래스
 * RMB: 줌 / LMB 누르기: 시위 당기기 / LMB 떼기: 발사
 */
UCLASS()
class VIALACTEA_API ABowBase : public ARangeweaponBase
{
	GENERATED_BODY()

public:
	ABowBase();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaTime) override;
	virtual void OnEquip(ACharacter* NewOwner) override;
	virtual void OnRep_Owner() override;
	virtual bool OnUnequip() override;
	virtual void CompleteUnequip() override;

	// 점프 등 외부 중단 시 에임/드로우 정리
	virtual void CancelAction() override;

	// 활 전용 상태까지 포함해서 강제 초기화
	virtual void ForceResetState() override;

	// RMB 누르기 → 줌 시작
	virtual void StartSecondaryAction() override;

	// RMB 떼기 → 줌 해제
	virtual void StopSecondaryAction() override;

	// LMB 누르기 → 시위 당기기 시작
	virtual void StartPrimaryAction() override;

	// LMB 떼기 → 발사
	virtual void StopPrimaryAction() override;

	// 발사 몽타주 자연 종료 시 Strafe 해제
	virtual void OnNaturalMontageEnd() override;

	// 외부 몽타주 감지 — 줌 상태에서 내 몽타주가 아니면 정리
	virtual void OnMontageStarted(UAnimMontage* Montage) override;

	virtual bool CanStartAction(EEquipmentActionType ActionType) const override;
	virtual bool ShouldConsumeStaminaForAction(EEquipmentActionType ActionType) const override;

public:
	// 발사할 화살 클래스 (블루프린트에서 설정)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment|Weapon|Bow|Arrow")
	TSubclassOf<class AArrowProjectile> ArrowClass;

	// 발사 방향 계산에 사용할 소켓 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment|Weapon|Bow|Arrow")
	FName ArrowSpawnSocket = TEXT("ArrowSocket");

	// 드로우 중 화살이 부착될 소켓 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment|Weapon|Bow|Arrow")
	FName ArrowAttachSocket = TEXT("bow_string_mid_Socket");

	// 미리보기 화살이 바라볼 기준 소켓 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment|Weapon|Bow|Arrow")
	FName ArrowPreviewGuideSocket = TEXT("ArrowGuideSocket");

	// 완전히 당겼을 때 화살 발사 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment|Weapon|Bow|Arrow")
	float ArrowSpeed = 5000.f;

	// 최소 충전 상태의 화살 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment|Weapon|Bow|Arrow|Charge")
	float MinArrowSpeed = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment|Weapon|Bow|Arrow|Charge", meta = (ClampMin = "0.0"))
	float MinChargeDamageMultiplier = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment|Weapon|Bow|Arrow|Charge", meta = (ClampMin = "0.0"))
	float FullChargeDamageMultiplier = 1.5f;

	//[KDH 2026.04.08] 인벤토리에서 사용할 화살 아이템 이름
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment|Weapon|Bow|Ammo")
	FName RequiredArrowItemID = TEXT("Arrow_01");
	//[KDH 2026. 04.14] 화살 차징 계산값 가져가기
	UFUNCTION(BlueprintCallable, Category = "Equipment|Weapon|Bow|UI")
	float GetCrosshairChargeAlpha() const;

	UFUNCTION(BlueprintCallable, Category = "Equipment|Weapon|Bow|UI")
	bool ShouldShowCrosshairChargeRing() const;
	//

	// 이 시간 이상 당기면 최대 속도까지 도달
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment|Weapon|Bow|Arrow|Charge", meta = (ClampMin = "0.01"))
	float FullChargeTime = 3.0f;

	// 초반에 더 빨리 차오르고 후반에 천천히 올라가도록 하는 이징 강도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment|Weapon|Bow|Arrow|Charge", meta = (ClampMin = "1.0"))
	float ChargeEaseOutExponent = 4.0f;

	// RMB 조준 중에도 활줄/미리보기 화살이 유지되도록 하는 기본 시각 알파
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment|Weapon|Bow|Animation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ZoomPreviewDrawAlpha = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment|Weapon|Bow|Debug")
	bool bDrawPredictedPathDebug = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment|Weapon|Bow|Debug")
	FLinearColor PredictedPathDebugColor = FLinearColor(0.f, 1.f, 1.f, 0.1f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment|Weapon|Bow|Debug")
	FLinearColor PredictedPathHitDebugColor = FLinearColor(1.f, 0.f, 0.f, 0.1f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment|Weapon|Bow|Debug", meta = (ClampMin = "0.1"))
	float PredictedPathMaxSimTime = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment|Weapon|Bow|Debug", meta = (ClampMin = "1.0"))
	float PredictedPathSimFrequency = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment|Weapon|Bow|Debug", meta = (ClampMin = "0.0"))
	float PredictedPathDebugThickness = 1.5f;

	// 줌 상태 (RMB 누르는 중)
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Equipment|Weapon|Bow|State")
	bool bIsZooming = false;

	// 시위 당기는 중 (LMB 누르는 중)
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "Equipment|Weapon|Bow|State")
	bool bIsDrawing = false;

	// 마지막 발사에 사용된 드로우 시간
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Equipment|Weapon|Bow|State")
	float CurrentDrawDuration = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Equipment|Weapon|Bow|State")
	float ReplicatedDrawAlpha = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment|Weapon|Bow|State")
	bool bHasValidPullHandTarget = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment|Weapon|Bow|State")
	FVector PullHandWorldLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment|Weapon|Bow|State")
	FVector PullHandLocationCS = FVector::ZeroVector;

	// 시위 당기기 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment|Weapon|Bow|Animation")
	class UAnimMontage* DrawMontage;

	// 활 전용 AnimBP를 쓸 때 지정할 AnimInstance 클래스
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment|Weapon|Bow|Animation")
	TSubclassOf<class UBowAnimInstance> BowAnimInstanceClass;

	// 손이 시위를 잡는 기준 소켓 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment|Weapon|Bow|Animation")
	FName PullHandSocketName = TEXT("BowPullSocket");

	// 전용 소켓이 없을 때 사용할 왼손 기본 소켓/본 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment|Weapon|Bow|Animation")
	FName PullHandFallbackSocketName = TEXT("hand_l");

	// 줌 상태 변경 델리게이트 (캐릭터 bIsAiming 업데이트용)
	FOnZoomChanged OnZoomChanged;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment|Weapon|Bow|Sound")
	TObjectPtr<class USoundBase> BowDrawSoundCue = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment|Weapon|Bow|Sound")
	TObjectPtr<class USoundBase> BowReleaseSoundCue = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment|Weapon|Bow|Sound", meta = (ClampMin = "0.0"))
	float BowDrawSoundVolumeMultiplier = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment|Weapon|Bow|Sound", meta = (ClampMin = "0.01"))
	float BowDrawSoundPitchMultiplier = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment|Weapon|Bow|Sound", meta = (ClampMin = "0.0"))
	float BowDrawSoundFadeInTime = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment|Weapon|Bow|Sound", meta = (ClampMin = "0.0"))
	float BowDrawSoundFadeOutTime = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment|Weapon|Bow|Sound", meta = (ClampMin = "0.0"))
	float BowReleaseSoundVolumeMultiplier = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment|Weapon|Bow|Sound", meta = (ClampMin = "0.01"))
	float BowReleaseSoundPitchMultiplier = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment|Weapon|Bow|Sound", meta = (ClampMin = "0.0"))
	float BowReleaseSoundFadeOutTime = 0.12f;

private:
	void ConfigureBowAnimationInstance();

	// 드로우 중 활에 부착된 화살 (발사 전까지 유지)
	UPROPERTY()
	class AArrowProjectile* DrawArrow = nullptr;

	// 화살 발사 처리
	void FireArrow(float InArrowSpeed);

	float CalculateChargedArrowSpeed(float& OutHeldDuration, float& OutChargeAlpha) const;

	// 시위 당기기 중단 및 몽타주 정지
	void StopDrawing();

	// 드로우 취소 시 부착 화살 파괴
	void DestroyDrawArrow();

	// 현재 상태 기반으로 bIsAiming 브로드캐스트
	void BroadcastAimState();

	void UpdateBowAnimationData();
	void UpdatePreviewArrowTransform();
	void DrawPredictedArrowPathDebug() const;
	bool TryGetPullHandWorldTransform(FTransform& OutWorldTransform) const;
	bool TryGetPullHandWorldLocation(FVector& OutWorldLocation) const;
	bool TryGetPreviewArrowTransform(FVector& OutLocation, FRotator& OutRotation) const;
	bool GetArrowLaunchData(FVector& OutSocketLocation, FVector& OutDirection, FVector* OutTargetPoint = nullptr) const;
	float GetCurrentDrawAlpha() const;
	float GetPredictedArrowSpeed() const;
	void EnsureDrawArrowAttached();
	bool HasArrowAmmo() const;
	bool ShouldHoldBowString() const;
	virtual void HandleFireBowArrow() override;
	FVector GetBowSoundLocation() const;
	void StartBowDrawSoundLocal();
	void StopBowDrawSoundLocal();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayBowSound(class USoundBase* Sound, FVector Location, float VolumeMultiplier, float PitchMultiplier);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastStartBowDrawSound();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastStopBowDrawSound();

	UFUNCTION(Server, Reliable)
	void ServerStartSecondaryAction();

	UFUNCTION(Server, Reliable)
	void ServerStopSecondaryAction();

	UFUNCTION(Server, Reliable)
	void ServerStartPrimaryAction();

	UFUNCTION(Server, Reliable)
	void ServerStopPrimaryAction(float ClientHeldDuration, float ClientChargeAlpha, float ClientArrowSpeed, FVector ClientLaunchDirection, FVector ClientTargetPoint);

	double DrawStartTime = 0.0;
	float PendingArrowSpeed = 0.f;
	FVector PendingArrowLaunchDirection = FVector::ZeroVector;
	FVector PendingArrowTargetPoint = FVector::ZeroVector;
	UPROPERTY(Replicated)
	bool bPendingArrowFire = false;

	UPROPERTY(Transient)
	TObjectPtr<class UAudioComponent> BowDrawSoundAudioComponent = nullptr;

	float CalculateCurrentDrawAlphaLocal() const;

	protected:
		virtual void OnInterruptedMontageEnd() override;
};
