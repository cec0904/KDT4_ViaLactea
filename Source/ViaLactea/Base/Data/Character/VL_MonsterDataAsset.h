#pragma once

#include "CoreMinimal.h"
#include "Base/Data/Character/VL_CharacterDataAsset.h"
#include "Base/AI/AIBossPattern.h"
#include "VL_MonsterDataAsset.generated.h"

class UBehaviorTree;

UCLASS()
class VIALACTEA_API UVL_MonsterDataAsset : public UVL_CharacterDataAsset
{
	GENERATED_BODY()
public:
    // 몬스터 전용: 등급 (일반, 엘리트, 보스)
    //UPROPERTY(EditAnywhere, Category = "Monster|Rank")
    //EMonsterRank MonsterRank;

    UPROPERTY(EditDefaultsOnly, Category = "Monster|Visual")
    TSoftObjectPtr<UStaticMesh> WeaponSkeletalMesh = nullptr;

    UPROPERTY(EditAnywhere, Category = "Monster|Visual")
    FName AttackSocketName = TEXT("RightHandSocket");

    //UPROPERTY(EditDefaultsOnly, Category = "Monster|Visual")
    //TSoftClassPtr<UAnimInstance> PostProcessAnimClass = nullptr;


    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster|Animation")
    TSoftObjectPtr<UAnimMontage> MonsterGroggyMontage = nullptr;

    //////////// 무기 ////////////
    // 피격 시 재생할 몽타주
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster|Animation")
    TSoftObjectPtr<UAnimMontage> WeaponHitMontage = nullptr;

    // 그로기 발생 시 재생할 몽타주
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster|Animation")
    TSoftObjectPtr<UAnimMontage> WeaponGroggyMontage = nullptr;

    // 사망 시 재생할 몽타주
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster|Animation")
    TSoftObjectPtr<UAnimMontage> WeaponDeathMontage = nullptr;

    // 공격 시 재생할 몽타주
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster|Animation")
    TSoftObjectPtr<UAnimMontage> WeaponAttackMontage = nullptr;
    //////////// 무기 ////////////

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FVector AttackBoxSize = FVector(80.f, 70.f, 50.f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackOffset = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Appearance", meta = (ClampMin = "0.1", ClampMax = "100.0"))
    float MonsterScale = 1.0f;

    // 몬스터 전용: 처치 시 전리품 (Loot Table DataAsset 참조)
    UPROPERTY(EditAnywhere, Category = "Monster|Reward")
    TSoftObjectPtr<UPrimaryDataAsset> LootTable = nullptr;

    // 몬스터 전용: 인식 범위 및 공격 사거리 (AI 결정 요인)
    UPROPERTY(EditAnywhere, Category = "Monster|Detection")
    float DetectionRange = 1000.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Detection")
    float LoseTargetRange = DetectionRange * 1.5; // 타겟을 놓치는 거리

    UPROPERTY(EditAnywhere, Category = "Monster|Detection")
    float StickinessBuffer = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Detection", meta = (ClampMin = "0.0", ClampMax = "180.0"))
    float VisionAngle = 60.0f;

    UPROPERTY(EditAnywhere, Category = "Monster|Detection")
    float AttackRange = 200.f;

    UPROPERTY(EditAnywhere, Category = "Monster|Detection")
    float ExitAttackRange = 250.f;


    // 최대 그로기 수치 (이 수치가 차면 그로기 발생)
    UPROPERTY(EditDefaultsOnly, Category = "Monster|Stat")
    float MaxGroggyGauge = 100.f;

    // 초당 그로기 게이지 회복량 (전투가 없을 때 게이지가 줄어드는 속도)
    UPROPERTY(EditDefaultsOnly, Category = "Monster|Stat")
    float GroggyRecoveryRate = 5.f;

    // 만약 몽타주 재생이 아닌 '상태'로서의 시간을 관리한다면 필요합니다.
    UPROPERTY(EditDefaultsOnly, Category = "Monster|Stat")
    float GroggyDuration = 3.0f;

    UPROPERTY(EditAnywhere, Category = "Monster|Knockback")
    float Mass = 100.0f;

    UPROPERTY(EditAnywhere, Category = "Monster|Knockback")
    float KnockbackForce = 500.f;
    // 밀기 저항력 (0.0 ~ 1.0): 1.0이면 외부 힘(LaunchCharacter 등)에 전혀 밀리지 않음
    UPROPERTY(EditAnywhere, Category = "Monster|Knockback", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float KnockbackResistance = 0.0f;

    // 다른 캐릭터가 밀 때 밀리는 정도 (CharacterMovement 관련)
    UPROPERTY(EditAnywhere, Category = "Monster|Knockback")
    float PushForceFactor = 1.0f;

    UPROPERTY(EditAnywhere, Category = "Monster|Knockback")
    bool CanKnockback = true;

    UPROPERTY(EditAnywhere, Category = "Player|Knockback")
    float KnockbackStrength = 100.f;

    UPROPERTY(EditAnywhere, Category = "Player|Knockback")
    float KnockbackUpwardForce = 100.f;

    UPROPERTY(EditAnywhere, Category = "Monster|AI")
    TSoftObjectPtr<UBehaviorTree> BehaviorTreeAsset = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Boss")
    FText BossDisplayName;

public:

    virtual FPrimaryAssetId GetPrimaryAssetId() const override
    {
        return FPrimaryAssetId(FPrimaryAssetType("MonsterData"), DataID);
    }
};
