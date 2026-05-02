// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "VL_AIControllerBase.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;

UCLASS()
class VIALACTEA_API AVL_AIControllerBase : public AAIController
{
	GENERATED_BODY()
public:
    AVL_AIControllerBase();

protected:
    virtual void BeginPlay() override;

    virtual void OnPossess(APawn* InPawn) override;

    // 감지 시 호출될 함수 (반드시 UFUNCTION이어야 함)
    UFUNCTION()
    void OnTargetDetected(AActor* Actor, FAIStimulus Stimulus);

private:
    // 퍼셉션 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = "true"))
    UAIPerceptionComponent* AIPerceptionComponent;

    // 시야 설정 객체
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = "true"))
    UAISenseConfig_Sight* SightConfig;
	
};
