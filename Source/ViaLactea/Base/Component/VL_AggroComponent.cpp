#include "Base/Component/VL_AggroComponent.h"
#include "Player/MainCharacterBase.h"
#include "CustomLog/CustomLog.h"

// Sets default values for this component's properties
UVL_AggroComponent::UVL_AggroComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}

void UVL_AggroComponent::RegisterInitialTarget(AActor* Target)
{
    if (!Target || ThreatMap.Contains(Target)) return;

    // 데미지는 0이지만 맵에 등록하여 보스가 인지하게 함
    // 아주 작은 기본값(예: 1.0f)을 주어 '우선순위'를 생성할 수도 있음
    ThreatMap.Add(Target, 0.f);
}

// 보스몬스터 맵 입장
void UVL_AggroComponent::AddThreat(AActor* Attacker, float DamageAmount)
{
	if (!Attacker) return;

	// 이미 맵에 있으면 기존 값에 더하고, 없으면 새로 추가
	float& CurrentThreat = ThreatMap.FindOrAdd(Attacker);
	CurrentThreat += (DamageAmount * DamageWeight);
}

// 죽거나 보스몬스터 맵 퇴장 
void UVL_AggroComponent::RemoveThreat(AActor* Target)
{
	if (Target && ThreatMap.Contains(Target))
	{
		ThreatMap.Remove(Target);
	}
}

void UVL_AggroComponent::ClearAllThreat()
{
    ThreatMap.Empty();
}

AActor* UVL_AggroComponent::GetHighestAggroTarget(float DetectionRange, AActor* CurrentTarget) const
{
    if (ThreatMap.IsEmpty()) return nullptr;
    struct FAggroCandidate {
        AActor* Actor;
        float RawThreat;
        float RawDistance;
        float FinalWeight;
    };

    TArray<FAggroCandidate> Candidates;
    float TotalThreat = 0.0f;
    float TotalDistanceScore = 0.0f;
    const FVector OwnerLoc = GetOwner()->GetActorLocation();
    const float RangeSq = FMath::Square(DetectionRange);

    for (auto& Elem : ThreatMap)
    {
        AActor* Target = Elem.Key;
        if (!IsValid(Target)) continue;

        AMainCharacterBase* Player = Cast<AMainCharacterBase>(Target);
        if (Player && Player->GetStatComponent()->IsDead()) continue;

        float DistSq = FVector::DistSquared(OwnerLoc, Target->GetActorLocation());
        if (DistSq > RangeSq) continue;

        float Dist = FMath::Sqrt(DistSq);
        float Threat = Elem.Value;

        // 거리 점수: 멀수록 0에 가깝고, 가까울수록 DetectionRange에 가까운 점수
        float DistScore = FMath::Max(0.0f, DetectionRange - Dist);

        Candidates.Add({ Target, Threat, DistScore, 0.0f });
        TotalThreat += Threat;
        TotalDistanceScore += DistScore;
    }

    int32 N = Candidates.Num();
    if (N == 0) return nullptr;

    // 2. 가중치 계산 (비율화)
    float TotalWeightSum = 0.0f;
    float BaseProb = 1.0f / (float)N; // 인원수 기반 기본 확률 (4명이면 0.25)

    for (auto& Candidate : Candidates)
    {
        // 데미지 비중 (데미지가 하나도 없으면 0)
        float ThreatRatio = (TotalThreat > 0) ? (Candidate.RawThreat / TotalThreat) : 0.0f;

        // 거리 비중 (모두 사거리 밖이면 0)
        float DistanceRatio = (TotalDistanceScore > 0) ? (Candidate.RawDistance / TotalDistanceScore) : 0.0f;

        // 최종 가중치 합산 (기본 25% + 데미지 비중 + 거리 비중)
        // 각 비중의 가중치는 테스트로 결정
        Candidate.FinalWeight = BaseProb + ThreatRatio + (DistanceRatio * 0.5f);

        // 현재 타겟에게 약간의 점수를 더 줘서 너무 잦은 타겟 변경 방지
        if (Candidate.Actor == CurrentTarget) Candidate.FinalWeight *= 1.2f;

        TotalWeightSum += Candidate.FinalWeight;
    }

    // 3. 가중치 랜덤 추첨 (Roulette Wheel)
    float Roll = FMath::FRandRange(0.0f, TotalWeightSum);
    float CurrentBeam = 0.0f;

    for (const auto& Candidate : Candidates)
    {
        CurrentBeam += Candidate.FinalWeight;
        if (Roll <= CurrentBeam)
        {
            return Candidate.Actor;
        }
    }
    return Candidates[0].Actor; // 예외 케이스 대비
}

TMap<AActor*, float>* UVL_AggroComponent::GetThreatMap()
{
    return &ThreatMap;
}

// Called when the game starts
void UVL_AggroComponent::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void UVL_AggroComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

