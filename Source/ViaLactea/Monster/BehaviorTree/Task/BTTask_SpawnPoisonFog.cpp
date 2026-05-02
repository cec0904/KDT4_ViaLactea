#include "Monster/BehaviorTree/Task/BTTask_SpawnPoisonFog.h"
#include "AIController.h"
#include "Actor/Gimmik/PoisonFogActor.h"
#include "Monster/Boss/VL_Boss1.h"

UBTTask_SpawnPoisonFog::UBTTask_SpawnPoisonFog()
{
    NodeName = TEXT("Spawn Poison Fog");

}

EBTNodeResult::Type UBTTask_SpawnPoisonFog::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AVL_Boss1* BossPawn = Cast<AVL_Boss1>(OwnerComp.GetAIOwner()->GetPawn());
    if (!BossPawn || !FogClass) return EBTNodeResult::Failed;

    FVector ForwardVector = BossPawn->GetActorForwardVector();
    FRotator SpawnRotation = BossPawn->GetActorRotation();
    FVector BossLocation = BossPawn->GetActorLocation();

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = BossPawn;
    SpawnParams.Instigator = BossPawn;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    int32 SuccessCount = 0;

    float InnerRadius = SpawnDistance;
    //float OuterRadius = SpawnDistance * 2.0f;


    APoisonFogActor* SpawnedActor = GetWorld()->SpawnActor<APoisonFogActor>(
        FogClass,
        BossLocation,
        SpawnRotation,
        SpawnParams
    );
        
    if (SpawnedActor)
    {
        // ---------------------------------------------------------
        // [핵심] 스폰 직후 보스의 사망 이벤트에 안개의 파괴 함수 바인딩
        // ---------------------------------------------------------
        // 보스 클래스에 OnBossDeathEvent가 멀티캐스트 델리게이트로 선언되어 있어야 합니다.
        BossPawn->OnBossDeathEvent.AddDynamic(SpawnedActor, &APoisonFogActor::HandleBossDeath);

        return EBTNodeResult::Succeeded;
    }

    //for (int32 i = 0; i < 4; ++i)
    //{
    //    float Angle = i * 90.0f;

    //    // 2. 앞방향 벡터를 Z축 기준으로 회전시킴
    //    FVector RotatedDirection = ForwardVector.RotateAngleAxis(Angle, FVector(0, 0, 1));

    //    // 3. 최종 위치 계산
    //    FVector FinalSpawnLocation = BossLocation + (RotatedDirection * InnerRadius);

    //    // 4. 액터 스폰
    //    APoisonFogActor* SpawnedActor = GetWorld()->SpawnActor<APoisonFogActor>(
    //        FogClass,
    //        FinalSpawnLocation,
    //        SpawnRotation,
    //        SpawnParams
    //    );

    //    if (SpawnedActor)
    //    {
    //        BossPawn->OnBossDeathEvent.AddUObject(SpawnedActor, &APoisonFogActor::DestroyFog);

    //        SuccessCount++;
    //    }
    //}

    // 1. 스폰 위치 계산 (보스 위치 + 정면 방향 * 거리)
    //for (int32 i = 0; i < 8; ++i)
    //{
    //    float Angle = i * 45.0f;

    //    // 2. 앞방향 벡터를 Z축 기준으로 회전시킴
    //    FVector RotatedDirection = ForwardVector.RotateAngleAxis(Angle, FVector(0, 0, 1));

    //    // 3. 최종 위치 계산
    //    FVector FinalSpawnLocation = BossLocation + (RotatedDirection * InnerRadius);

    //    // 4. 액터 스폰
    //    APoisonFogActor* SpawnedActor = GetWorld()->SpawnActor<APoisonFogActor>(
    //        FogClass,
    //        FinalSpawnLocation,
    //        SpawnRotation,
    //        SpawnParams
    //    );
    //    
    //    if (SpawnedActor)
    //    {
    //        BossPawn->OnBossDeathEvent.AddUObject(SpawnedActor, &APoisonFogActor::DestroyFog);

    //        SuccessCount++;
    //    }
    //}

    //for (int32 i = 0; i < 12; ++i)
    //{
    //    float Angle = i * 30.0f;

    //    // 2. 앞방향 벡터를 Z축 기준으로 회전시킴
    //    FVector RotatedDirection = ForwardVector.RotateAngleAxis(Angle, FVector(0, 0, 1));

    //    // 3. 최종 위치 계산
    //    FVector FinalSpawnLocation = BossLocation + (RotatedDirection * OuterRadius);

    //    // 4. 액터 스폰
    //    APoisonFogActor* SpawnedActor = GetWorld()->SpawnActor<APoisonFogActor>(
    //        FogClass,
    //        FinalSpawnLocation,
    //        SpawnRotation,
    //        SpawnParams
    //    );
    //    if (SpawnedActor)
    //    {
    //        BossPawn->OnBossDeathEvent.AddUObject(SpawnedActor, &APoisonFogActor::DestroyFog);

    //        SuccessCount++;
    //    }
    //}

    // 2. 액터 스폰

    //return (SuccessCount > 0) ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
    return  EBTNodeResult::Succeeded;
}