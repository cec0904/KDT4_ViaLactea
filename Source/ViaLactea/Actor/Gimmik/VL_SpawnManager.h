// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VL_SpawnManager.generated.h"

class ACharacter;

UCLASS()
class VIALACTEA_API AVL_SpawnManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVL_SpawnManager();

protected:
    virtual void BeginPlay() override;

    // 스폰할 몬스터 클래스 (에디터에서 지정)
    UPROPERTY(EditAnywhere, Category = "Spawning")
    TSubclassOf<ACharacter> MonsterClass;

    UPROPERTY()
    TArray<class ACharacter*> SpawnedMonsters;

    // 스폰 주기
    UPROPERTY(EditAnywhere, Category = "Spawning")
    float SpawnInterval = 1.0f;

    // 최대 스폰 가능 수
    UPROPERTY(EditAnywhere, Category = "Spawning")
    int32 MaxMonsterCount = 10;

    // 스폰 로직 실행 함수
    void SpawnMonster();

    UFUNCTION()
    void OnMonsterDestroyed(AActor* DestroyedActor);

    FVector GetRandomNavLocation();

    void EndPlay(const EEndPlayReason::Type EndPlayReason);

    FTimerHandle SpawnTimerHandle;

};
