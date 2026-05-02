#pragma once

#include "CoreMinimal.h"
#include "Base/Character/VL_AICharacterBase.h"
#include "Base/AI/AIBossPattern.h"
#include "Base/AI/GimmickTypes.h"
#include "VL_Boss1.generated.h"

class UVL_AggroComponent;
class UVL_CooldownComponent;
class UVL_BossMonsterDataAsset;
class AVL_AICharacterBase;
class USphereComponent;
class UTimelineComponent;

DECLARE_MULTICAST_DELEGATE(FOnComboEndedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPhaseChangeFinished);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBossDeath);

UCLASS()
class VIALACTEA_API AVL_Boss1 : public AVL_AICharacterBase
{
	GENERATED_BODY()

public:
	AVL_Boss1();

	void ResetCombat();

	UVL_AggroComponent* GetAggroComponent() const { return AggroComponent; }

	UFUNCTION(BlueprintCallable)
	FText GetBossName() const;

public:
	 UVL_BossMonsterDataAsset* GetBossDataAsset() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AIComponent")
	UVL_AggroComponent* AggroComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AIComponent")
	UVL_CooldownComponent* CooldownComponent;

	UPROPERTY()
	TObjectPtr<UVL_BossMonsterDataAsset> CachedBossData;

	UPROPERTY()
	TObjectPtr<AAIController> CachedAIC;


	// 타겟 위치를 워핑 컴포넌트에 업데이트하는 함수
	/*void UpdateWarpTarget();*/


	// 약점 노출 상태를 나타내는 스위치


	UPROPERTY(ReplicatedUsing = OnRep_ActiveWeakPoint, BlueprintReadOnly, Category = "Boss Pattern")
	TEnumAsByte<EPhysicalSurface> ActiveWeakPoint = EPhysicalSurface::SurfaceType_Default;

	FTimerHandle WeakPointTimerHandle;
	FTimerHandle WeakPointDurationHandle;
	FTimerHandle MinionPenaltyTimerHandle;
	// 위치 업데이트 전용 타이머 핸들
	FTimerHandle WeakPointLocationTimerHandle;
	FTimerHandle HitFlashTimerHandle;
	FTimerHandle CombatMoveTimerHandle;


	void UpdateWeakPointLocation();

	// 머터리얼 색상 변경
	UFUNCTION()
	void OnRep_ActiveWeakPoint();

	UPROPERTY()
	class UMaterialInstanceDynamic* BossDynamicMaterial;

	// 약점 위치를 머티리얼에 업데이트하는 함수
	void UpdateMaterialVisual(FVector Location, bool bIsActive);


public:
	void StartWeakPointCycle();
	void DeactivateWeakPoint();
	void ShowWeakPointVisual();
	/////////////////////////
protected:


	void SetHitFlashValue(float Value);

	void ResetHitFlash();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayHitFlash();


	//ThrowAxe 구현
protected:
	UPROPERTY(ReplicatedUsing = OnRep_BonesHidden)
	bool bBonesHidden = false;

	UFUNCTION()
	void OnRep_BonesHidden();

	void UpdateBoneVisibility();

public:
	// 본을 숨기는 멀티캐스트 함수
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_HideBones(const TArray<FName>& BoneNames, bool bHide);

	//던지기 공격
	UFUNCTION()
	void ThrowAxe(bool bRight);


public:
	// VL_AICharacterBase TakeDamage Override
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	bool IsPatternReady(EAIBossPattern PatternType);

	void OnPatternStarted(EAIBossPattern PatternType);

	void MoveToTarget(float DeltaTime, float MoveSpeed);

	void InitializeGimmicks();

	UFUNCTION()
	void SpawnPatternVFX(FGameplayTag PatternTag, FVector Location);

	UFUNCTION()
	void SpawnMeteor(FGameplayTag PatternTag, FVector SpawnLocation);

	UFUNCTION()
	void SpawnExplosive(FGameplayTag InLoopTag, FGameplayTag InExplosionTag, FVector SpawnLocation);

	UFUNCTION()
	void SpawnLaser(FGameplayTag PatternTag, AActor* TargetActor);

	UFUNCTION()
	void SpawnLightning(FGameplayTag PatternTag, FVector SpawnLocation);


	UFUNCTION()
	void SpawnMinions(FGameplayTag SummonTag);

	UFUNCTION()
	void CheckMinionsAndApplyPenalty();

	UFUNCTION()
	void SetRushMode(bool Enable);

	UFUNCTION()
	bool GetRushMode();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayBossMontage(UAnimMontage* Montage, float InPlayRate = 1.0f);

	UFUNCTION()
	void StopBossMontage(float BlendOutTime = 0.f);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StopBossMontage(float BlendOutTime = 0.f);

	UFUNCTION(BlueprintCallable, Category = "Combat|Knockback")
	FVector CalculateRushKnockbackVelocity(ACharacter* TargetChar);



// 타임라인 보스 페이즈 전환을 위한 함수
public:
// 타임라인 재생 시작 함수

	void StartEmergeTimeline();

	FOnPhaseChangeFinished OnPhaseChangeFinished;

protected:
	
	UPROPERTY(VisibleAnywhere, Category = "Timeline")
	UTimelineComponent* EmergeTimeline;


	
	UFUNCTION()
	void HandleTimelineProgress(float Value);

	
	UFUNCTION()
	void OnTimelineFinished();

protected:
	float LerpStartZ;
	float LerpEndZ;
	float LerpStartHeight;
	float LerpEndHeight;


public:
	// BT 태스크에서 콤보를 시작할 때 부르는 함수
	void StartCombo();

	// 애님 노티파이에서 호출할 함수 (다음 콤보 진행 여부 결정)
	void CheckNextCombo();

	// 콤보가 완전히 끝났을 때 호출
	void EndCombo();

	UFUNCTION(NetMulticast, Reliable)
	void Multi_PlayComboStep(FName SectionName, bool bIsFirstStep);

	UFUNCTION(NetMulticast, Reliable)
	void Multi_EndCombo(UAnimMontage* ResetMontage);

	UPROPERTY(Replicated)
	int32 CurrentPatternIndex = 0;

	void UpdateCombatMovement();

		// 외부(ANS)에서 접근할 수 있도록 setter 제공
	void SetCanRotate(bool bValue) { bCanRotate = bValue; }

protected:
	float CurrentDashSpeed = 0.f;
	float DashSpeed = 300.f;

	bool bCanRotate = true; // 기본값은 true

	bool bCanDash = false;
	//////////////////////////////////////////////////////////////
public:
	UFUNCTION(BlueprintCallable, Category = "Boss|State")
	bool GetIsRushing() const;

	AActor* GetTargetCharacter() const;
	
	void RotateToTarget(float DeltaTime);


	FOnComboEndedDelegate OnComboEnded;

protected:

	virtual void BeginPlay() override;

	virtual  void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PossessedBy(AController* NewController);

	//virtual void Tick(float DeltaTime) override;

	void StartBossAI();

protected:
	// 델리게이트에 연결할 함수 

	// 기믹 상태변화시 델리게이트
	UFUNCTION()
	void HandleGimmickStateChanged(EGimmickState NewState);

	//기믹 파괴시 델리게이트 
	UFUNCTION()
	void HandleGimmickDestroyed(AActor* DestroyedGimmick);

	//매시의 색을 바꾸기위한 포인터
	UPROPERTY()
	class UMaterialInstanceDynamic* DynamicMaterial;


protected:
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Combat")
	int32 CurrentComboIndex = 0;

	bool bCanContinueCombo = false;

protected:
	// int32에서 EBossPhase로 변경 및 OnRep 함수 연결
	UPROPERTY(ReplicatedUsing = OnRep_CurrentPhase, BlueprintReadOnly, Category = "Boss")
	EBossPhase CurrentPhase = EBossPhase::None;

	// 클라이언트에서 페이즈 변경 시 호출될 함수
	UFUNCTION()
	void OnRep_CurrentPhase(EBossPhase OldPhase);

	UFUNCTION()
	void OnPhaseMontageEnded(UAnimMontage* Montage, bool bInterrupted);


	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<AVL_AICharacterBase>> SummonedMinions;
	
	/////////////////////////////    RUSH    /////////////////////////////
protected:
	UPROPERTY(VisibleAnywhere, Category = "Combat")
	USphereComponent* WoodSphere;


	// 돌진 시 대미지 판정을 위한 트리거 (생성자에서 생성)
	UPROPERTY(VisibleAnywhere, Category = "Combat")
	USphereComponent* RushDamageDetector;

	// 돌진 중 이미 대미지를 입은 액터 목록 (중복 대미지 방지)
	UPROPERTY()
	TArray<AActor*> HitActors;

	// 충돌 시 호출될 함수
	UFUNCTION()
	void OnRushOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

private:
	UPROPERTY(ReplicatedUsing = OnRep_IsRushing, BlueprintReadOnly, Category = "Boss|State", meta = (AllowPrivateAccess = "true"))
	bool bIsRushing = false;

	UFUNCTION()
	void OnRep_IsRushing();
	/////////////////////////////    기믹 엑터 관리    /////////////////////////////
protected:
	UPROPERTY(Transient) // 런타임 데이터
	TArray<TWeakObjectPtr<AActor>> ManagedGimmicks;


private:
	int32 DestroyedPillarCount = 0;

protected:
	virtual bool ApplyGroggy(float DamageAmount) override;

public:
	virtual void Die() override;


	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnBossDeath OnBossDeathEvent;
public:
	virtual float GetCooldownValue(FGameplayTag Tag) const override;

	UFUNCTION()
	FVector GetRandomNavLocation(float Radius);
public:
	 EBossPhase GetCurrentPhase() const { return CurrentPhase; }

	void SetCurrentPhase(EBossPhase NewPhase);

};
