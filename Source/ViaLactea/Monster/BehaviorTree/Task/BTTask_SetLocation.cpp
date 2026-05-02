#include "Monster/BehaviorTree/Task/BTTask_SetLocation.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "NavMesh/RecastNavMesh.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Monster/Boss/VL_Boss1.h"
#include "Base/Data/Character/VL_BossMonsterDataAsset.h"

#include "CustomLog/CustomLog.h"

UBTTask_SetLocation::UBTTask_SetLocation()
{
    NodeName = "Boss SetLocation";
    bNotifyTick = false;
    bCreateNodeInstance = true;
    INIT_TASK_NODE_NOTIFY_FLAGS();

}

EBTNodeResult::Type UBTTask_SetLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    AVL_Boss1* BossCharacter = Cast<AVL_Boss1>(AIController->GetPawn());
    if (!BossCharacter) return EBTNodeResult::Failed;

    UVL_BossMonsterDataAsset* BossDataAsset = BossCharacter->GetBossDataAsset();
    if (!BossDataAsset) return EBTNodeResult::Failed;

    // 2. 보스 캐릭터가 소유한 데이터 에셋 가져오기
    // (보스 캐릭터에 구현된 데이터 에셋 반환 함수나 변수를 호출합니다)
    CachedMontage = BossDataAsset->SetLocationMontage.LoadSynchronous();

    if (!CachedMontage) return EBTNodeResult::Failed;

    // 3. 네비게이션 및 위치 계산
    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    const ARecastNavMesh* NavMesh = NavSys ? Cast<ARecastNavMesh>(NavSys->GetMainNavData()) : nullptr;
    if (!NavMesh) return EBTNodeResult::Failed;

    FVector Origin = BossCharacter->GetCharacterMovement()->GetActorFeetLocation();
    FVector BackDir = -BossCharacter->GetActorForwardVector();

    float JumpDistance = BossDataAsset->ProximityLimit;
    TArray<float> TestAngles = { 0.0f, -30.0f, 30.0f, -60.0f, 60.0f, -90.f, 90.f, -120.f, 120.f, -150.f, 150.f, 180.f };

    float BestDistance = MAX_FLT;
    FVector BestLocation = FVector::ZeroVector;
    FVector HomeLocation = OwnerComp.GetBlackboardComponent()->GetValueAsVector(FName("HomePos")); // bb->home 위치

    for (float Angle : TestAngles)
    {
        FVector RotatedDir = BackDir.RotateAngleAxis(Angle, FVector::UpVector);
        FVector CandidateLoc = Origin + (RotatedDir * JumpDistance);

        FNavLocation NavLoc;
        if (NavSys->ProjectPointToNavigation(CandidateLoc, NavLoc, FVector(200.0f, 200.0f, 500.0f)))
        {
            // 2. ARecastNavMesh의 FindDistanceToWall 호출
            // FindDistanceToWall 현재 nullptr, 전용 필터 만약 특정 구역(예: 물, 용암)을 피해야 한다면 ANavigationData 기본 필터를 명시적으로 넘기기
            float DistToHome = FVector::Dist(NavLoc.Location, HomeLocation);
            if (DistToHome < BestDistance)
            {
                BestDistance = DistToHome;
                BestLocation = NavLoc.Location;
            }
        }

    }
    // 점프할 위치를 찾았다면
    if (BestDistance < MAX_FLT)
    {
        FVector LaunchVelocity;

        if (CalculateJumpVelocity(Origin, BestLocation, LaunchVelocity, JumpTime))
        {

            CachedOwnerComp = &OwnerComp;
            CachedBoss = BossCharacter;

            BossCharacter->LandedDelegate.AddDynamic(this, &UBTTask_SetLocation::OnBossLanded);

            UAnimInstance* AnimInst = BossCharacter->GetMesh()->GetAnimInstance();
            if (AnimInst)
            {

                //CUSTOM_LOG("setlocation 실행");

                const float TotalFrames = 40.0f;
                const float JumpEndFrame = 25.0f;
                float JumpPhaseRatio = JumpEndFrame / TotalFrames;

                float SequenceLength = CachedMontage->GetPlayLength() * JumpPhaseRatio;
                float AnimPlayRate = FMath::Clamp(SequenceLength / JumpTime, 0.5f, 1.5f);
                BossCharacter->PlayAnimMontage(CachedMontage, AnimPlayRate);

                // 핵심: 지역 변수로 선언하여 참조(&) 전달이 가능하게 함
                FOnMontageEnded EndDelegate;
                EndDelegate.BindUObject(this, &UBTTask_SetLocation::OnSetLocationMontageEnded);

                // 첫 번째 인자로 만든 델리게이트를, 두 번째 인자로 대상 몽타주를 넘깁니다.
                AnimInst->Montage_SetEndDelegate(EndDelegate, CachedMontage);
            }


            BossCharacter->LaunchCharacter(LaunchVelocity, true, true);
            return EBTNodeResult::InProgress;
        }

    }

    return EBTNodeResult::Failed;
}

void UBTTask_SetLocation::OnBossLanded(const FHitResult& Hit)
{
    if (CachedBoss)
    {
        CachedBoss->LandedDelegate.RemoveDynamic(this, &UBTTask_SetLocation::OnBossLanded);
        FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);

        //UAnimInstance* AnimInst = CachedBoss->GetMesh()->GetAnimInstance();
        //if (AnimInst && CachedMontage)
        //{
        //    // 착지 모션으로 전환 (아직 태스크는 안 끝남)
        //    AnimInst->Montage_JumpToSection(FName("JumpEnd"), CachedMontage);
        //    AnimInst->Montage_SetPlayRate(CachedMontage, 1.0f);
        //}
    }
}


// 포물선 궤적 계산 함수
bool UBTTask_SetLocation::CalculateJumpVelocity(FVector Start, FVector End, FVector& OutVelocity, float& Time)
{
    float Gravity = FMath::Min(-100.0f, GetWorld()->GetGravityZ());
    float JumpHeight = 200.0f;

    float DisplacementZ = End.Z - Start.Z;
    FVector DisplacementXY = FVector(End.X - Start.X, End.Y - Start.Y, 0.0f);

    // 판별식(Discriminant) 계산 시 안전성 확보
    // 등가속도 운동 공식($v^2 = v_0^2 + 2as$)을 변형한 식
    float VZSq = -2.0f * Gravity * JumpHeight;
    float VelocityZ = FMath::Sqrt(FMath::Max(0.0f, VZSq));

    float UnderSqrt = (VelocityZ * VelocityZ) - (2.0f * Gravity * DisplacementZ);
    // 높이가 너무 높아서 도달할 수 없음 false
    if (UnderSqrt < 0.0f)
    {
        //CUSTOM_LOG("높이가 너무 높음 undersqrt : %.f", UnderSqrt);
        return false;
    }
    // 해당 높이에 도달하는 시간 찾음
    Time = (-VelocityZ - FMath::Sqrt(UnderSqrt)) / Gravity;

    if (Time <= KINDA_SMALL_NUMBER)
    {
        //CUSTOM_LOG(" 시간 보장 불가");
        return false;
    }

    FVector VelocityXY = DisplacementXY / Time;
    OutVelocity = VelocityXY + FVector(0, 0, VelocityZ);

    return true;
}

void UBTTask_SetLocation::OnSetLocationMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (CachedOwnerComp)
    {

        // FinishLatentTask가 호출되면 자동으로 OnTaskFinished가 실행
        FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
    }
}

void UBTTask_SetLocation::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
    if (UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
    {
        BB->SetValueAsEnum(TEXT("AIBossPattern"), (uint8)EAIBossPattern::None);
        BB->SetValueAsEnum(TEXT("Reposition"), (uint8)EReposition::None);
        BB->SetValueAsBool(TEXT("BIsLocked"), true);
    }

    // 모든 델리게이트 안전 해제
    if (CachedBoss)
    {
        // 착지 델리게이트 해제
        CachedBoss->LandedDelegate.RemoveAll(this);

        if (UAnimInstance* AnimInst = CachedBoss->GetMesh()->GetAnimInstance())
        {
            FOnMontageEnded EmptyDelegate;
            AnimInst->Montage_SetEndDelegate(EmptyDelegate, nullptr);
        }
    }

    Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);

    CachedBoss = nullptr;
    CachedOwnerComp = nullptr;
    CachedMontage = nullptr;
}

EBTNodeResult::Type UBTTask_SetLocation::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    return Super::AbortTask(OwnerComp, NodeMemory);;
}
