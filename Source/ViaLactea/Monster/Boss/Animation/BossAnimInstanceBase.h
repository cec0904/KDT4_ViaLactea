// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/Character/VL_CharacterAnimInstanceBase.h"
#include "BossAnimInstanceBase.generated.h"

class AVL_Boss1;

UCLASS()
class VIALACTEA_API UBossAnimInstanceBase : public UVL_CharacterAnimInstanceBase
{
	GENERATED_BODY()

public:
    virtual void NativeInitializeAnimation() override;

    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Character")
    AVL_Boss1* OwnerBoss;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State")
    bool bIsRushing;
};
