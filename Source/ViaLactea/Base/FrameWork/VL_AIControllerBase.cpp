// Fill out your copyright notice in the Description page of Project Settings.


#include "Base/FrameWork/VL_AIControllerBase.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "../Character/VL_AICharacterBase.h"

#include"CustomLog/CustomLog.h"

AVL_AIControllerBase::AVL_AIControllerBase()
{
    // 1. 퍼셉션 컴포넌트 생성
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));

    // 2. 시야(Sight) 설정 객체 생성
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));

    // 3. 시야 세부 파라미터 설정
    SightConfig->SightRadius = 1500.f;            // 감지 거리
    SightConfig->LoseSightRadius = 2000.f;        // 놓치는 거리
    SightConfig->PeripheralVisionAngleDegrees = 90.f; // 시야각 (반각 90도 = 총 180도)
    SightConfig->SetMaxAge(5.f);                  // 기억 유지 시간

    // 소속별 감지 설정 (플레이어가 Neutral인 경우가 많으므로 필수 체크)
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

    // 4. 컴포넌트에 설정 적용
    AIPerceptionComponent->ConfigureSense(*SightConfig);
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
}

void AVL_AIControllerBase::BeginPlay()
{
    Super::BeginPlay();

    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AVL_AIControllerBase::OnTargetDetected);
    }
}

void AVL_AIControllerBase::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    //if (AVL_AICharacterBase* Monster = Cast<AVL_AICharacterBase>(InPawn))
    //{
    //    UBlackboardComponent* BB = GetBlackboardComponent();
    //    if (BB)
    //    {
    //        FVector CurrentLoc = Monster->GetActorLocation();
    //        UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());

    //        if (NavSys)
    //        {
    //            FNavLocation ProjectedLocation;
    //            // 현재 위치에서 반경 100 유닛 내의 가장 가까운 네비게이션 메시 위 점을 찾음
    //            if (NavSys->ProjectPointToNavigation(CurrentLoc, ProjectedLocation))
    //            {
    //                CUSTOM_LOG("홈포스 저장");
    //                BB->SetValueAsVector(TEXT("HomePos"), ProjectedLocation.Location);
    //            }
    //            else
    //            {
    //                // 이 로그가 찍힌다면 독립 실행형에서 네비게이션 메시가 아직 로드되지 않은 것임
    //                CUSTOM_LOG("Fail to Project HomePos to Navigation Mesh in Standalone!");
    //                BB->SetValueAsVector(TEXT("HomePos"), CurrentLoc);
    //            }
    //        }
    //    }
    //}
}

void AVL_AIControllerBase::OnTargetDetected(AActor* Actor, FAIStimulus Stimulus)
{
    //if (!Actor || !Actor->ActorHasTag(TEXT("Player"))) return;
    //APawn* ControlledPawn = GetPawn();
    //if (!ControlledPawn) return;

    //UBlackboardComponent* BBComp = GetBlackboardComponent();
    //if (!BBComp || !Actor) return;

    //if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
    //{
    //    if (Stimulus.WasSuccessfullySensed())
    //    {
    //        if (Actor->ActorHasTag(TEXT("Player")))
    //        {
    //            BBComp->SetValueAsObject(TEXT("TargetActor"), Actor);
    //            // CUSTOM_LOG("Player Detected!");
    //        }
    //    }
    //    else
    //    {
    //        AActor* CurrentTarget = Cast<AActor>(BBComp->GetValueAsObject(TEXT("TargetActor")));
    //        if (CurrentTarget == Actor)
    //        {
    //            BBComp->SetValueAsObject(TEXT("TargetActor"), nullptr);
    //        }
    //    }
    //}
}



