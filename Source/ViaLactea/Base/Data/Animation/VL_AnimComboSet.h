// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "VL_AnimComboSet.generated.h"

/**
 * 
 */
UCLASS()
class VIALACTEA_API UVL_AnimComboSet : public UDataAsset
{
	GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    TArray<TSoftObjectPtr<UAnimMontage>> ComboMontages;

    // 인덱스로 안전하게 몽타주를 가져오는 함수
    UFUNCTION(BlueprintCallable, Category = "Animation")
    UAnimMontage* GetMontageByIndex(int32 Index) const;


    // 전체 콤보 개수를 알려주는 유틸리티 함수 (블루프린트용)
    UFUNCTION(BlueprintPure, Category = "Animation")
    int32 GetMaxComboCount() const { return ComboMontages.Num(); }

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    float ResetDelay = 1.0f; // 콤보가 초기화되는 시간
};
