// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/AI/BehaviorTree/Tasks/VL_BTTaskNode.h"
#include "BTTask_SpawnPoisonFog.generated.h"

class APoisonFogActor;

UCLASS()
class VIALACTEA_API UBTTask_SpawnPoisonFog : public UVL_BTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_SpawnPoisonFog();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Spawn")
    TSubclassOf<APoisonFogActor> FogClass;

    UPROPERTY(EditAnywhere, Category = "Spawn")
    float SpawnDistance = 1500.f; // 보스 정면으로부터의 거리
	
};
