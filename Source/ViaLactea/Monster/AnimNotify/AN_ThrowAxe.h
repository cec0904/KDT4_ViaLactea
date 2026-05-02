// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AN_ThrowAxe.generated.h"

/**
 * 
 */
UCLASS()
class VIALACTEA_API UAN_ThrowAxe : public UAnimNotify
{
	GENERATED_BODY()
public:

	// 에디터 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bIsRightHand = true;

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	
};
