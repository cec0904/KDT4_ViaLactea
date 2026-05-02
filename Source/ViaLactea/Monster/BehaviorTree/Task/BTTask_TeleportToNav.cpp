// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_TeleportToNav.h"
#include "NavigationSystem.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/CapsuleComponent.h"

#include "CustomLog/CustomLog.h"


UBTTask_TeleportToNav::UBTTask_TeleportToNav()
{
    // 비헤이비어 트리 노드에 표시될 기본 이름
    NodeName = TEXT("Teleport To Nav");
}

EBTNodeResult::Type UBTTask_TeleportToNav::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIC = Cast<AAIController>(OwnerComp.GetAIOwner());
    if (!AIC) return EBTNodeResult::Failed;

    ACharacter* Monster = Cast<ACharacter>(AIC->GetPawn());
    if (!Monster) return EBTNodeResult::Failed;

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB) return EBTNodeResult::Failed;


    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (NavSys)
    {
        FNavLocation ClosestNavLocation;
        // 1. 가장 가까운 NavMesh 지점 찾기 (반경 넉넉히 1000)
        bool bFound = NavSys->ProjectPointToNavigation(
            Monster->GetActorLocation(),
            ClosestNavLocation,
            FVector(1200.f, 1200.f, 800.f)
        );

        FVector TargetLocation;
        if (bFound)
        {
            TargetLocation = ClosestNavLocation.Location;
            //CUSTOM_LOG("Teleporting to closest NavMesh point.");
        }
        else
        {
            // 2. 근처에 땅이 아예 없으면 HomePos로 복귀
            TargetLocation = BB->GetValueAsVector(TEXT("HomePos"));
            //CUSTOM_LOG("No NavMesh nearby. Teleporting to HomePos.");
        }

        float HalfHeight = Monster->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
        TargetLocation.Z += HalfHeight;

        // Physics 연산을 초기화하며 텔레포트 (리슨 서버 동기화에 유리)
        Monster->SetActorLocation(TargetLocation, false, nullptr, ETeleportType::TeleportPhysics);
        Monster->GetCharacterMovement()->StopMovementImmediately();

        return EBTNodeResult::Succeeded;
    }

    return EBTNodeResult::Failed;
}

