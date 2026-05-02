#pragma once
#include "CoreMinimal.h"
#include "AIState.h"
#include "AIBossPattern.generated.h"

//TMap<EPatternFamily, FBossPatternList> 으로 관리해서

UENUM(BlueprintType)
enum class EAIBossPattern : uint8
{
	None				UMETA(DisplayName = "None"),				// 0
	PhaseTransition     UMETA(DisplayName = "PhaseTransition"),		// 1
	AoE					UMETA(DisplayName = "AoE"),					// 2
	Rush				UMETA(DisplayName = "Rush"),				// 3
	Ultimate			UMETA(DisplayName = "Ultimate"),			// 4
	Summon				UMETA(DisplayName = "Summon"),				// 5
	BasicAttack			UMETA(DisplayName = "BasicAttack"),			// 6
	ComboAttack         UMETA(DisplayName = "ComboAttack"),			// 7
	Reposition			UMETA(DisplayName = "Reposition"),			// 8
	CloseExplosion		UMETA(DisplayName = "CloseExplosion"),		// 9
	LaserBeam			UMETA(DisplayName = "LaserBeam"),			// 10
	SpinAttack			UMETA(DisplayName = "SpinAttack"),			// 11
	CloseLightning		UMETA(DisplayName = "CloseLightning"),		// 12
	DarkStone			UMETA(DisplayName = "DarkStone"),			// 13
	BackstepAndThrow	UMETA(DisplayName = "BackstepAndThrow"),    // 14
	TripleThrow			UMETA(DisplayName = "TripleThrow")			// 15
};

UENUM(BlueprintType)
enum class EReposition : uint8
{
	None        UMETA(DisplayName = "None"),
	Rotate      UMETA(DisplayName = "Rotate"),   // 타겟 쪽으로 회전
	Strafe      UMETA(DisplayName = "Strafe"),   // 타겟 주변 횡이동
	Backstep    UMETA(DisplayName = "Backstep"), // 뒤로 회피
	SetLocation UMETA(DisplayName = "SetLocation") // 위치 다시 잡기
};

UENUM(BlueprintType)
enum class EBossPhase : uint8
{
	None		UMETA(DisplayName = "None"),
	Phase_1		UMETA(DisplayName = "Phase 1"),
	Phase_2		UMETA(DisplayName = "Phase 2"),
	Phase_3		UMETA(DisplayName = "Phase 3"),
	Phase_4		UMETA(DisplayName = "Phase 4")
};

USTRUCT(BlueprintType)
struct FBossCombatContext
{
	GENERATED_BODY()
	float TargetDistance2D = 0.f;
	float TargetAngleDeg = 0.f;
	float RearWallDistance = 0.f;
	float BackstepSpace = 0.f;
	float DistanceFromArenaCenter = 0.f;
	int32 NumPlayersNearBoss = 0;
	int32 NumPlayersBehindBoss = 0;
	bool bHasLineOfSight = false;
	bool bTargetReachable = false;
	bool bTargetBehind = false;
	bool bTooCloseForTooLong = false;
};

UENUM(BlueprintType)
enum class EPatternFamily : uint8
{
	None				UMETA(DisplayName = "None"),
	Entry				UMETA(DisplayName = "Entry"),			// 접근
	NeutralMelee		UMETA(DisplayName = "NeutralMelee"),	// 근접 공격
	AntiClose			UMETA(DisplayName = "AntiClose"),		// 근접 견제
	AntiRear			UMETA(DisplayName = "AntiRear"),		// 뒤를 공격
	RangedPressure		UMETA(DisplayName = "RangedPressure"),  // 원거리 공격
	Reposition			UMETA(DisplayName = "Reposition"),		// 자리 잡기
	TimeCycle			UMETA(DisplayName = "TimeCycle")		// 시간 주기 공격
};


UENUM(BlueprintType)
enum class EAIBossPatternRP : uint8
{
	None				UMETA(DisplayName = "None"),
	BackstepAndThrow	UMETA(DisplayName = "BackstepAndThrow"),
	TripleThrow			UMETA(DisplayName = "TripleThrow")
};