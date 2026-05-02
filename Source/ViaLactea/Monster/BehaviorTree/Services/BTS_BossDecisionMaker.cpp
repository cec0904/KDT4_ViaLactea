#include "Monster/BehaviorTree/Services/BTS_BossDecisionMaker.h"
#include "AIController.h"              
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"

#include "Base/AI/AIBossPattern.h"
#include "Base/Data/Character/VL_BossMonsterDataAsset.h"
#include "Monster/Boss/VL_Boss1.h"
#include "Base/Component/VL_AggroComponent.h"

#include "Player/MainCharacterBase.h"

#include "CustomLog/CustomLog.h"


UBTS_BossDecisionMaker::UBTS_BossDecisionMaker()
{
    NodeName = TEXT("Boss Pattern Decision Service");
    Interval = 0.11f;
    //RandomDeviation = 0.03f;
    bCreateNodeInstance = true;
}

void UBTS_BossDecisionMaker::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return;

	if (AAIController* AIC = OwnerComp.GetAIOwner())
	{
		CachedBoss = Cast<AVL_Boss1>(AIC->GetPawn());
		if (CachedBoss)
		{
			CachedDataAsset = CachedBoss->GetBossDataAsset();
		}
	}

	//APawn* Target = Cast<APawn>(BB->GetValueAsObject(TEXT("TargetActor")));

	//// 진입하자마자 즉시 상황 파악 실행
	//if (CachedBoss && Target)
	//{
	//	UpdateCombatContext(CachedBoss, Target, BB);
	//}

	ContextUpdateTimer = 0.0f;

}

void UBTS_BossDecisionMaker::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!AIC || !AIC->HasAuthority()) return;

	if (!IsValid(CachedBoss) || !IsValid(CachedDataAsset))
	{

		CachedBoss = Cast<AVL_Boss1>(AIC->GetPawn());
		if (CachedBoss)
		{
			CachedDataAsset = CachedBoss->GetBossDataAsset();
		}

	}

	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();


	CurrentTime = GetWorld()->GetTimeSeconds();

	if (CachedContext.NumPlayersNearBoss > 0)
	{	
		CloseRangeTimer += DeltaSeconds;
	}
	else
	{
		CloseRangeTimer = FMath::Max(0.0f, CloseRangeTimer - DeltaSeconds);
	}

	if (CachedBoss->GetIsGroggy() == false)
	{
		float RecoveryAmount = CachedDataAsset->GroggyRecoveryRate * DeltaSeconds;
		CachedBoss->DecreaseGroggyGauge(RecoveryAmount);

	}
	if (TryPhaseTransition(BB))
	{
		// 페이즈 전환이 시작되었다면 아래의 일반 패턴 결정 로직을 모두 건너뜁니다.
		return;
	}

	NavCheckTimer += DeltaSeconds;
	if (NavCheckTimer >= 2.0f) { NavCheckTimer = 0.0f; CheckBossNavMeshLocation(BB); }



	APawn* Target = nullptr;
	EAIBossPattern CurrentPattern = (EAIBossPattern)BB->GetValueAsEnum("AIBossPattern");

	if (BB->GetValueAsEnum(BossPatternKey.SelectedKeyName) == (uint8)EAIBossPattern::None)
	{
		
		// 2. 상태 체크
		EAIState CurrentAIState = (EAIState)BB->GetValueAsEnum(TEXT("AIState"));
		if (CurrentAIState == EAIState::Groggy) return;

		// 3. 현재 타겟 유효성 검사 (죽음 체크) - 상단에서 한 번만 수행
		AActor* CurrentTarget = Cast<AActor>(BB->GetValueAsObject(TEXT("TargetActor")));
		UVL_AggroComponent* AggroComp = CachedBoss->GetAggroComponent();

		if (AggroComp && CurrentTarget)
		{
			AMainCharacterBase* Character = Cast<AMainCharacterBase>(CurrentTarget);
			if (Character && Character->GetStatComponent()->IsDead())
			{
				AggroComp->RemoveThreat(CurrentTarget);
				BB->SetValueAsObject(TEXT("TargetActor"), nullptr);
				CurrentTarget = nullptr; // 하단 로직을 위해 null 처리
			}
		}

		// 4. 타겟 결정 로직 (BIsLocked 여부에 따라 분기)
		bool bIsLockedKey = BB->GetValueAsBool(TEXT("BIsLocked"));

		bool bIsRushMode = CachedBoss->GetRushMode();

		if (!bIsLockedKey && !bIsRushMode)
		{
			// [일반 상황] 어그로 기반으로 최적의 타겟 검색
			if (AggroComp)
			{
				float DetectionRange = CachedDataAsset->DetectionRange;
				AActor* BestTarget = AggroComp->GetHighestAggroTarget(DetectionRange, CurrentTarget);

				if (BestTarget)
				{
					BB->SetValueAsObject(TEXT("TargetActor"), BestTarget);
					BB->SetValueAsEnum(TEXT("AIState"), (uint8)EAIState::Combat);
					AIC->SetFocus(BestTarget);
					Target = Cast<APawn>(BestTarget);
				}
				else
				{
					// 주변에 아무도 없음 -> Idle 전환
					BB->SetValueAsObject(TEXT("TargetActor"), nullptr);
					BB->SetValueAsEnum(TEXT("AIState"), (uint8)EAIState::Idle);
					AIC->ClearFocus(EAIFocusPriority::Gameplay);
					return;
				}
			}
		}
		else
		{

			// [Locked 상황] 타겟 갱신 없이 유지 (이전 타겟이 죽었으면 위에서 이미 null 됨)
			Target = Cast<APawn>(CurrentTarget);
			if (Target)
			{
				AIC->SetFocus(Target);
			}
			else
			{
				// 락이 걸려있는데 타겟이 죽어서 없어졌다면? -> 강제 락 해제 및 탈출
				BB->SetValueAsBool(TEXT("BIsLocked"), false);
				return;
			}

			if (!bIsRushMode)
			{
				BB->SetValueAsBool(TEXT("BIsLocked"), false);
				// CUSTOM_LOG("일회성 락 소모 완료 (Strafe/Backstep 이후)");
			}
		}

		// 5. 최종 결과 확인
		if (!Target)
		{
			CloseRangeTimer = 0.f;
			return;
		}
		if (EAIState::Combat != (EAIState)BB->GetValueAsEnum(TEXT("AIState")))
		{
			//CUSTOM_LOG("전투상황이 아님");
			return;
		}

		// [1단계] 전투 상황 계산
		UpdateCombatContext(CachedBoss, Target, BB);


		EPatternFamily SelectedFamily;


		if (CachedDataAsset->PhaseSettings[EBossPhase::Phase_1].MovementMode == MOVE_None)
		{
			//우드보스일 때는 약간 다른 필터링이 필요함
			SelectedFamily = WoodDecidePatternFamily(CachedContext);
		}
		else
		{
			SelectedFamily = DecidePatternFamily(CachedContext);
		}

		// 패턴 테스트 코드
		//SelectedFamily = EPatternFamily::NeutralMelee;
		//SelectedFamily = EPatternFamily::Entry;
		//SelectedFamily = EPatternFamily::AntiClose;

		if (SelectedFamily != EPatternFamily::Reposition)
		{

			SelectOnePattern(SelectedFamily, BB);
		}
		else
		{
			SelectPositioning(Target, BB);
		}
	}
	return;
}

bool UBTS_BossDecisionMaker::TryPhaseTransition(UBlackboardComponent* BB)
{
	if (!BB || !CachedBoss || !CachedDataAsset) return false;

	// 이미 페이즈 전환 중이라면 중복 실행 방지
	if (BB->GetValueAsEnum(BossPatternKey.SelectedKeyName) == (uint8)EAIBossPattern::PhaseTransition)
		return false;

	float HPRatio = CachedBoss->GetHPRatio();
	EBossPhase CurrentPhase = CachedBoss->GetCurrentPhase();

	EBossPhase NextPhase = EBossPhase::None; // 기본값은 None

	for (int32 i = CachedDataAsset->PhaseThresholds.Num() - 1; i >= 0; --i)
	{
		const auto& Threshold = CachedDataAsset->PhaseThresholds[i];

		if (HPRatio <= Threshold.HealthRatio)
		{
			NextPhase = Threshold.TargetPhase;


			break;
		}

	}
	// 현재 페이즈보다 결정된 페이즈가 높을 때만 전이 발생
	if (NextPhase != EBossPhase::None &&NextPhase > CurrentPhase)
	{
		BB->SetValueAsEnum(PhaseKey.SelectedKeyName, (uint8)NextPhase);

		BB->SetValueAsEnum(BossPatternKey.SelectedKeyName, (uint8)EAIBossPattern::PhaseTransition);

		return true;
	}
	return false;
}

float UBTS_BossDecisionMaker::GetRepositionDuration(EReposition Type) const
{
	switch (Type)
	{
	case EReposition::Strafe:
		return CachedDataAsset->RepositionData.StrafeDuration;
	case EReposition::Backstep:
		return CachedDataAsset->RepositionData.BackstepDuration;
	case EReposition::Rotate:
		return CachedDataAsset->RepositionData.RotateDuration;
	case EReposition::SetLocation:
		return CachedDataAsset->RepositionData.SetLocationDuration;
	default:
		return 0.f;
	}
}

void UBTS_BossDecisionMaker::UpdateCombatContext(AVL_Boss1* Boss, APawn* Target, UBlackboardComponent* BB)
{
	if (!Boss || !Target) return;

	FVector BossLoc = Boss->GetActorLocation();
	FVector TargetLoc = Target->GetActorLocation();
	FVector ToTarget = (TargetLoc - BossLoc);
	ToTarget.Z = 0.f; // 2D 계산을 위해 Z축 제거

	// 1. 기본 거리 및 각도
	CachedContext.TargetDistance2D = ToTarget.Size();

	FVector Forward = Boss->GetActorForwardVector();
	Forward.Z = 0.f;
	Forward.Normalize();
	ToTarget = ToTarget.GetSafeNormal();

	// 내적(Dot Product)을 이용한 각도 계산
	// 부동 소수점 오차 때문에 Clamp 함수로 범위 설정
	float Dot = FVector::DotProduct(Forward, ToTarget);
	CachedContext.TargetAngleDeg = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(Dot, -1.f, 1.f)));

	// 타겟이 뒤에 있는지 확인 (내적이 음수면 뒤쪽)
	CachedContext.bTargetBehind = (Dot < 0.f);

	// 2. 아레나 중앙(스폰 위치)으로부터의 거리
	FVector ArenaCenterLoc = BB->GetValueAsVector("HomePos");

	CachedContext.DistanceFromArenaCenter = FVector::Dist2D(BossLoc, ArenaCenterLoc);

	// 3. 주변 플레이어 수 체크 (Overlap 또는 Iterator)
	TArray<FHitResult> OutHits;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(CachedDataAsset->ProximityLimit); // 체크 반경
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Boss);

	GetWorld()->SweepMultiByChannel(OutHits, BossLoc, BossLoc, FQuat::Identity, ECC_Pawn, Sphere, Params);

	int32 NearCount = 0;
	int32 BehindCount = 0;
	for (auto& Hit : OutHits)
	{
		AActor* HitActor = Hit.GetActor();
		// 캐릭터 생성자에 태그를 달면 Hit.GetActor()->ActorHasTag(TEXT("Player")) 
		if (HitActor && Cast<AMainCharacterBase>(HitActor))
		{
			//CUSTOM_LOG("CachedDataAsset->ProximityLimit: %.2f dist : %.2f",
			//CachedDataAsset->ProximityLimit, CachedContext.TargetDistance2D);
			++NearCount;
			// 여기서도 내적을 사용해 내 뒤에 있는지 판별 가능
			FVector ToPlayer = (HitActor->GetActorLocation() - BossLoc).GetSafeNormal2D();
			// 타겟으로 향하는 벡터가 보스의 벡터 기준 120~180도 사이에 있을 때
			if (FVector::DotProduct(Forward, ToPlayer) < -0.5f) ++BehindCount;
		}
	}
	CachedContext.NumPlayersNearBoss = NearCount;
	CachedContext.NumPlayersBehindBoss = BehindCount;

	// 4. 시야 확인 (LineTrace)
	CachedContext.bHasLineOfSight = Boss->GetController()->LineOfSightTo(Target);

	// 5. 벽까지의 거리 (LineTrace - 뒤쪽 방향)
	//FHitResult WallHit;
	//FVector End = Start - (Boss->GetActorForwardVector() * 500.f); // 뒤로 벽 체크 ECC콜리전 채널 설정 필요할 수도
	//if (GetWorld()->LineTraceSingleByChannel(WallHit, Start, End, ECC_WorldStatic))
	//{
	//	CachedContext.RearWallDistance = WallHit.Distance;
	//}
	//else
	//{
	//	CachedContext.RearWallDistance = 500.f; // 벽 없음
	//}
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (NavSys)
	{
		// RearWallDistance
		FVector ToBoss = -ToTarget;
		FVector Start = BossLoc;
		FVector End = Start + (ToBoss * 500.f);
		FVector HitLocation;

		// NavigationRaycast로 보스 뒤 500 범위 내의 이동 가능 구역 확인
		bool bHitEdge = NavSys->NavigationRaycast(Boss, Start, End, HitLocation);

		if (bHitEdge)
		{
			// 부딪힌 곳까지의 거리 저장
			CachedContext.BackstepSpace = FVector::Distance(Start, HitLocation);
		}
		else
		{
			CachedContext.BackstepSpace = 500.f;
		}
		// --- [TargetReachable (타겟 도달 가능 여부)] ---
		FNavLocation ProjectedLocation;
		// 타겟 주변 반경(100, 100, 200)에 유효한 내비매시가 있는지 체크
		CachedContext.bTargetReachable = NavSys->ProjectPointToNavigation(TargetLoc, ProjectedLocation, FVector(100.f, 100.f, 300.f));
	}
	else
	{
		// NavSystem이 없는 예외 상황 처리 (기본값 세팅)
		CachedContext.BackstepSpace = 500.f;
		CachedContext.bTargetReachable = false;
	}

	// 6. 도달 가능 여부 (NavSystem)
	//if (NavSys)
	//{
	//	FNavLocation ProjectedLocation;
	//	// 타겟 위치 주변 100 이내에 유효한 내비메시 데이터가 있는지 확인
	//	bool bOnNavMesh = NavSys->ProjectPointToNavigation(TargetLoc, ProjectedLocation, FVector(100.f, 100.f, 200.f));

	//	CachedContext.bTargetReachable = bOnNavMesh;
	//}

	// 7. bTooCloseForTooLong 
	CachedContext.bTooCloseForTooLong = (CloseRangeTimer > CloseRangeThreshold);
}

EPatternFamily UBTS_BossDecisionMaker::DecidePatternFamily(const FBossCombatContext& Context) const
{

	//if (FMath::RandRange(0, 100) < 10)
	//{
	//	return EPatternFamily::Reposition; // 백스텝 등의 위치 재조정
	//}
	//
	if (Context.NumPlayersNearBoss >= 3)
	{
		return FMath::RandRange(0, 100) > 30 ? EPatternFamily::AntiClose : EPatternFamily::Reposition;
	}
	// 리포지션 필터링
	// 플레이어가 근처에 오래 있었음
	if (Context.bTooCloseForTooLong )
	{
		// 백스텝 거리가 협소하면 점프로 자리찾기
		// 백스텝 거리가 멀면 백스텝 후 투척 or 번개 주변에 뿌리기 
		if (Context.BackstepSpace > 400.f)
		{
			return EPatternFamily::Reposition;
		}
		else
		{
			return EPatternFamily::AntiClose;
		}
	}
	if (Context.NumPlayersBehindBoss > 0)
	{
		// 내 뒤에 플레이어가 있다면 뒤돌아 베기나 꼬리치기 패밀리 선택
		return EPatternFamily::AntiRear;
	}


	if (!Context.bTargetReachable)
	{
		return EPatternFamily::RangedPressure;
	}

	if (Context.TargetDistance2D > (CachedDataAsset->DetectionRange))
	{

		return FMath::RandBool() ? EPatternFamily::RangedPressure : EPatternFamily::Entry;
	}
	
	// =========================================================
	// 5순위: 접근 (Entry)
	// =========================================================
	if (Context.TargetDistance2D > CachedDataAsset->AttackRange) // 보스의 기본 사거리 기준
	{
		return FMath::RandRange(0, 100) > 50 ? EPatternFamily::RangedPressure : EPatternFamily::Entry;
	}
	// =========================================================
	// 6순위: 기본 근접전 
	// 위 모든 특수 상황(위험)이 없고, 사거리 안
	// =========================================================
	return FMath::RandBool() ? EPatternFamily::NeutralMelee : EPatternFamily::AntiClose;
}
EPatternFamily UBTS_BossDecisionMaker::WoodDecidePatternFamily(const FBossCombatContext& Context) const
{
	const bool bIsStationary = CachedDataAsset ? CachedDataAsset->bIsStationary : false;

	//if (Context.NumPlayersNearBoss >= 3)
	if (Context.NumPlayersNearBoss >= 2)
	{
		return EPatternFamily::AntiClose;
	}

	if (Context.bTooCloseForTooLong)
	{
		// 너무 인접해있는 시간이 김
		return EPatternFamily::AntiClose;
	}


	// =========================================================
	// 6순위: 기본 근접전 
	// 위 모든 특수 상황(위험)이 없고, 사거리 안
	// =========================================================
	if (Context.TargetDistance2D < CachedDataAsset->AttackRange)
	{
		//CUSTOM_LOG("타겟이 사거리보다 가까움 : TargetDistance2D : %.2f, AttackRange : %.2f", Context.TargetDistance2D, CachedDataAsset->AttackRange);
		return EPatternFamily::NeutralMelee;
	}
	return EPatternFamily::None;
}

void UBTS_BossDecisionMaker::SelectOnePattern(EPatternFamily SelectedFamily, UBlackboardComponent* BB)
{

	TArray<FBossPatternData> ValidPatterns;
	float TotalWeight = 0.0f;
	EBossPhase CurrentPhase = CachedBoss->GetCurrentPhase();

	for (const FBossPatternData& Pattern : CachedDataAsset->BossPatterns)
	{
		// SelectedFamily과 일치하지 않으면 continue, 단 TimeCycle은 필터링 되지 않고 어느정도는 주기에 맞게 실행될 수 있게
		if (Pattern.PatternFamily != SelectedFamily && Pattern.PatternFamily != EPatternFamily::TimeCycle)
		{
			continue;
		}

		if (Pattern.PatternType == EAIBossPattern::PhaseTransition) continue;

		bool bPhaseValid = (uint8)CurrentPhase >= (uint8)Pattern.RequiredPhase;
		bool bDistanceValid = (CachedContext.TargetDistance2D >= Pattern.MinRange && CachedContext.TargetDistance2D <= Pattern.MaxRange);
		bool bCooldownValid = CachedBoss->IsPatternReady(Pattern.PatternType);

		if (bPhaseValid && bDistanceValid && bCooldownValid)
		{
			ValidPatterns.Add(Pattern);
			TotalWeight += Pattern.Weight;
		}
	}
	if (ValidPatterns.Num() > 0)
	{
		//. 가용 패턴이 있다면 그 가중치 확률 대로 패턴을 선택하는 로직
		float RandomVal = FMath::FRandRange(0.0f, TotalWeight);
		float CumulativeWeight = 0.0f;

		for (const FBossPatternData& ValidPattern : ValidPatterns)
		{
			CumulativeWeight += ValidPattern.Weight;
			if (RandomVal <= CumulativeWeight)
			{
				// 근접 타이머 시간으로 체크한 패턴이면 타이머 초기화
				if (SelectedFamily == EPatternFamily::AntiClose) CloseRangeTimer = 0.f;

				// 최종 선택된 패턴을 블랙보드에 기록
				BB->SetValueAsEnum(BossPatternKey.SelectedKeyName, (uint8)ValidPattern.PatternType);
				BB->SetValueAsEnum(BossFamilyPatternKey.SelectedKeyName, (uint8)SelectedFamily);

			/*	BB->SetValueAsFloat("ActionStartTime", CurrentTime);
				BB->SetValueAsFloat("ActionDuration", ValidPattern.Duration);*/

				break;
			}
		}
	}
	else
	{
		if (CachedDataAsset->PhaseSettings[EBossPhase::Phase_1].MovementMode == MOVE_Walking)
		{
			APawn* Target = Cast<APawn>(BB->GetValueAsObject(TEXT("TargetActor")));

			SelectPositioning(Target, BB);
		}

	}
}

// 10%확률, 근처에 사람이 많거나, 근접전투 상황에서 뒤에 벽이 있을 때 
void UBTS_BossDecisionMaker::SelectPositioning(APawn* Target, UBlackboardComponent* BB)
{
	EReposition SelectedType;
	// 근처에 적이 많음
	if(CachedContext.NumPlayersNearBoss>=3)
	{ 
		SelectedType = EReposition::SetLocation;
		CloseRangeTimer = 0.0f;
	}
	//  타겟이 뒤에 있음
	else if (CachedContext.bTargetBehind) //  (약 110도)
	{
		SelectedType = EReposition::Rotate;
	}
	else if (CachedContext .bTooCloseForTooLong && CachedContext.BackstepSpace > 400.f)
	{
		SelectedType = EReposition::Backstep;
		CloseRangeTimer = 0.0f;
	}
	else // 횡이동 섞기
	{
		SelectedType = EReposition::Strafe;
	}
	BB->SetValueAsEnum(RepositionTypeKey.SelectedKeyName, (uint8)SelectedType);
	BB->SetValueAsEnum(BossPatternKey.SelectedKeyName, (uint8)EAIBossPattern::Reposition);
	return;
}

void UBTS_BossDecisionMaker::CheckBossNavMeshLocation(UBlackboardComponent* BB)
{
	if (!CachedBoss || !BB|| !CachedBoss->HasAuthority()) return;

	if (CachedDataAsset->PhaseSettings[EBossPhase::Phase_1].MovementMode == MOVE_None)
	{
		return;
	}

	FVector BossLoc = CachedBoss->GetActorLocation();
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());

	if (!NavSys) return;
	
	FNavLocation ProjectedLoc;
	// 1. 현재 위치가 내비메쉬 위인지 확인 (Extents는 보스의 크기에 맞춰 적절히 설정)
	bool bOnNav = NavSys->ProjectPointToNavigation(BossLoc, ProjectedLoc, FVector(200.f, 200.f, 500.f));

	if (!bOnNav)
	{

		// [Case A] 가장 가까운 내비메쉬 지점으로 강제 텔레포트 (끼임 방지)
		// 너무 멀리 튕겨나갔을 때를 대비해 검색 반경을 넓게 잡고 재시도
		if (NavSys->ProjectPointToNavigation(BossLoc, ProjectedLoc, FVector(1000.f, 1000.f, 1000.f)))
		{
			BB->SetValueAsEnum(TEXT("AIState"), (uint8)EAIState::Returnto);

		}
		else
		{
			// [Case B] 주변에 아예 내비메쉬가 없다면? (맵 밖으로 추락 등)
			// 블랙보드에 저장된 HomePos로 복귀시키거나 리셋 로직 실행

			FVector HomePos = BB->GetValueAsVector(TEXT("HomePos"));
			CachedBoss->SetActorLocation(HomePos);
	
		}
	}
}