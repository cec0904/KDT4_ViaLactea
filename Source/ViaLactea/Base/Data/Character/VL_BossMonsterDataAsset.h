#pragma once
#include "CoreMinimal.h"
#include "Base/Data/Character/VL_MonsterDataAsset.h"
#include "VL_BossMonsterDataAsset.generated.h"

class UVL_VFXDataAsset;

USTRUCT(BlueprintType)
struct FBossPatternData
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern")
    FGameplayTag PatternTag;

    // 어떤 패턴인지
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern")
    EAIBossPattern PatternType = EAIBossPattern::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern")
    EPatternFamily PatternFamily = EPatternFamily::None;

    // 이 패턴을 쓸 수 있는 최소 페이즈 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern")
    EBossPhase RequiredPhase = EBossPhase::Phase_1;

    // 발동 가능한 최소 거리
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern")
    float MinRange = 0.f;

    // 발동 가능한 최대 거리
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern")
    float MaxRange = 1000.f;

    // 선택될 확률 가중치 (높을수록 자주 나옴)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern")
    float Weight = 1.0f;

    // 패턴 사용 후 다시 쓸 수 있을 때까지의 쿨타임
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern")
    float Cooldown = 3.0f;

    // 유지 시간
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pattern")
    float Duration = 2.0f;

    // VFX 스폰 수
    UPROPERTY(EditAnywhere, Category = "Pattern|VFX")
    int32 SpawnCount = 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Pattern||Montage")
    TSoftObjectPtr<UAnimMontage> PatternMontage = nullptr;
};

USTRUCT(BlueprintType)
struct FComboStepData // 각 타수별 상세 데이터
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    FName SectionName; // 몽타주 내 섹션 이름 (예: Attack1)

    UPROPERTY(EditAnywhere, Category = "Combo")
    TSoftObjectPtr<UAnimMontage> ComboResetMontage = nullptr;

    UPROPERTY(EditAnywhere)
    float DamageMultiplier = 1.0f; // 이 타수의 데미지 배율

    UPROPERTY(EditAnywhere)
    float StepForce = 600.0f; // 이 타수의 돌진 힘
};

USTRUCT(BlueprintType)
struct FComboData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    TSoftObjectPtr<UAnimMontage> ComboMontage = nullptr;

    UPROPERTY(EditAnywhere, Category = "Combo")
    TArray<FComboStepData> ComboSteps;

    // 다음 콤보를 이어나갈 '최대 거리' 
    UPROPERTY(EditAnywhere, Category = "Combo")
    float ComboAttackRange = 350.0f;

    UPROPERTY(EditAnywhere, Category = "Combo")
    EBossPhase BossPhase = EBossPhase::None;

    UPROPERTY(EditAnywhere, Category = "Combo")
    bool ComboUseRootMotion = false;
};

USTRUCT(BlueprintType)
struct FRepositionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    float StrafeDuration = 2.0f;

    UPROPERTY(EditAnywhere)
    float BackstepDuration = 1.5f;

    UPROPERTY(EditAnywhere)
    float SetLocationDuration = 1.5f;

    UPROPERTY(EditAnywhere)
    float RotateDuration = 1.f;
};

USTRUCT(BlueprintType)
struct FGimmickInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    FGameplayTag GimmickTag; // "Gimmick.Pillar", "Gimmick.Trap" 등으로 구분

    UPROPERTY(EditAnywhere)
    TSubclassOf<AActor> GimmickClass = nullptr;

    UPROPERTY(EditAnywhere)
    float ActivationDuration = 5.0f;
};

USTRUCT(BlueprintType)
struct FMinionSpawnData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTag SpawnTag;

    // 소환할 몬스터 클래스 (BP_Minion_A 등)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<class ACharacter> MinionClass = nullptr;

    // 한 번에 소환할 수
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SpawnCount = 4;

    // 소환 반경
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpawnRadius = 500.0f;

};

USTRUCT(BlueprintType)
struct FPhaseSetting
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    float CapsuleHalfHeight = 90.f;

    UPROPERTY(EditAnywhere)
    float CapsuleRadius = 20.f;

    UPROPERTY(EditAnywhere)
    float MeshZOffset = -90.f;

    UPROPERTY(EditAnywhere)
    TEnumAsByte<EMovementMode> MovementMode = MOVE_Walking;

    UPROPERTY(EditAnywhere)
    bool bUseGravity = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Phase")
    bool bUseTransitionTimeline = false;

    UPROPERTY(EditAnywhere, Category = "Phase|Animation")
    TObjectPtr<UAnimMontage> PhaseTransitionMontage = nullptr;
};

USTRUCT(BlueprintType)
struct FPhaseThreshold
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    EBossPhase TargetPhase = EBossPhase::None;

    UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float HealthRatio =0.f; // 이 체력 비율 '이하'가 되면 해당 페이즈로 전이
};

UCLASS()
class VIALACTEA_API UVL_BossMonsterDataAsset : public UVL_MonsterDataAsset
{
	GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Pattern")
    TArray<FBossPatternData> BossPatterns;

    UPROPERTY(EditAnywhere, Category = "Boss|Phase")
    TMap<EBossPhase, FPhaseSetting> PhaseSettings; // 페이즈별 세팅

    UPROPERTY(EditAnywhere, Category = "Boss|Phase")
    TArray<FPhaseThreshold> PhaseThresholds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Phase")
    bool bIsStationary = false;  // true면 고정형

    UPROPERTY(EditAnywhere, Category = "Boss|Pattern|Summon")
    TArray<FMinionSpawnData> MinionSpawnInfos;

    UPROPERTY(EditAnywhere, Category = "Boss|Pattern|Gimmick")
    TArray<FGimmickInfo> GimmickList;

    UPROPERTY(EditAnywhere, Category = "Boss|Pattern|Gimmick")
    float WeakPointDuration = 5.0f; // 5초
    UPROPERTY(EditAnywhere, Category = "Boss|Pattern|Gimmick")
    float WeakPointInterval = 30.0f; // 30초

    UPROPERTY(EditAnywhere, Category = "Boss|Pattern")
    float ProximityLimit = 200.f;

    UPROPERTY(EditAnywhere, Category = "Boss|Pattern|Combo")
    TArray<FComboData> ComboPatterns;

    UPROPERTY(EditAnywhere, Category = "Boss|Pattern|Reposition")
    FRepositionData RepositionData;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Montage")
    TSoftObjectPtr<UAnimMontage> SetLocationMontage = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Montage")
    TSoftObjectPtr<UAnimMontage> RightRotationMontage = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss|Montage")
    TSoftObjectPtr<UAnimMontage> LeftRotationMontage = nullptr;

    // 이 보스가 사용할 전용 VFX 데이터 저장소
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|VFX")
    TObjectPtr<UVL_VFXDataAsset> VFXDataAsset = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss|Phase|Emerge")
    TObjectPtr<UCurveFloat> EmergeCurve = nullptr; // 등장 시 사용할 타임라인 커브

    UPROPERTY(EditAnywhere, Category = "Projectile")
    TSoftClassPtr<class AVL_FinalBossAxe> AxeClass;


public:
    virtual float GetCooldownValue(FGameplayTag Tag) const override;

    const FBossPatternData* GetPatternData(EAIBossPattern Type) const;

    const FBossPatternData* GetPatternDataByTag(FGameplayTag InTag) const;
};
