// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AN_AttackCheck.generated.h"

/**
 * 
 */
UCLASS()
class VIALACTEA_API UAN_AttackCheck : public UAnimNotify
{
	GENERATED_BODY()
public:
    // 1. 여기서 선언한 변수들이 몽타주 화면에 나타납니다.
    UPROPERTY(EditAnywhere, Category = "Attack Settings")
    bool bCanParry = true;

    UPROPERTY(EditAnywhere, Category = "Attack Settings")
    bool bShouldKnockback = false;

    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
