#include "Actor/Gimmik/VL_SpawnManager.h"
#include "NavigationSystem.h"
#include "GameFramework/Character.h"
#include "CustomLog/CustomLog.h"

// Sets default values
AVL_SpawnManager::AVL_SpawnManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

void AVL_SpawnManager::BeginPlay()
{
	Super::BeginPlay();

	// 타이머 시작: SpawnInterval마다 SpawnMonster 호출
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AVL_SpawnManager::SpawnMonster, SpawnInterval, true);
	
}

void AVL_SpawnManager::SpawnMonster()
{
    // 1. 현재 살아있는 몬스터 수 체크
    if (!MonsterClass || SpawnedMonsters.Num() >= MaxMonsterCount)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (World)
    {
        // 랜덤 위치 계산 (예시: 반경 1000 내)
        //FVector SpawnLocation = GetActorLocation() + FVector(FMath::RandRange(-1000.f, 1000.f), FMath::RandRange(-1000.f, 1000.f), 0.f);
        FVector SpawnLocation = GetRandomNavLocation();
        if (SpawnLocation.Equals(GetActorLocation(), 1.0f))
        {
            return;
        }
        SpawnLocation.Z += 90.f;

        FRotator SpawnRotation = FRotator(0.f, FMath::RandRange(0.f, 360.f), 0.f);

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        ACharacter* NewMonster = World->SpawnActor<ACharacter>(MonsterClass, SpawnLocation, SpawnRotation, SpawnParams);

        if (NewMonster)
        {
            // 3. 리스트에 추가
            SpawnedMonsters.Add(NewMonster);

            // 4. 몬스터가 죽거나 파괴될 때를 대비해 OnDestroyed 이벤트 바인딩
            NewMonster->OnDestroyed.AddDynamic(this, &AVL_SpawnManager::OnMonsterDestroyed);
        }
    }
}

FVector AVL_SpawnManager::GetRandomNavLocation()
{
    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (NavSys)
    {
        FNavLocation ResultLocation;
        // 2000 유닛 반경 내 랜덤 지점 찾기
        for (int i = 0; i < 5; ++i)
        {
            if (NavSys->GetRandomReachablePointInRadius(GetActorLocation(), 2000.f, ResultLocation))
            {
                return ResultLocation.Location;
            }
        }
    }
    return GetActorLocation(); // 실패 시 기본 위치 반환
}

void AVL_SpawnManager::OnMonsterDestroyed(AActor* DestroyedActor)
{
    if (ACharacter* Monster = Cast<ACharacter>(DestroyedActor))
    {
        // 리스트에서 제거
        SpawnedMonsters.Remove(Monster);

        // 로그 출력
        CUSTOM_LOG("Monster Destroyed. Current Count: %d", SpawnedMonsters.Num());
    }
}

void AVL_SpawnManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    // 타이머 해제
    GetWorldTimerManager().ClearTimer(SpawnTimerHandle);

    // 필요 시 관리하던 몬스터 모두 제거 (선택 사항)

    for (ACharacter* Monster : SpawnedMonsters)
    {
        if (Monster) Monster->Destroy();
    }

    SpawnedMonsters.Empty();
}