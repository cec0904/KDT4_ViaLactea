// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "VL_CharacterMontageData.generated.h"

USTRUCT(BlueprintType)
struct FVL_TagToMontage
{
    GENERATED_BODY()

    // 무기/스킬의 ActionData에서 넘겨줄 식별 태그 (예: Action.Attack.Normal)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTag ActionTag;

    // 해당 태그를 받았을 때 이 캐릭터가 재생할 실제 애니메이션
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UAnimMontage> AnimMontage = nullptr;
};

UCLASS()
class VIALACTEA_API UVL_CharacterMontageData : public UDataAsset
{
	GENERATED_BODY()

public:
    // 태그와 몽타주를 매핑한 리스트
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    TArray<FVL_TagToMontage> MontageMap;

    // 특정 태그를 넣으면 대응하는 몽타주를 찾아주는 헬퍼 함수
    UFUNCTION(BlueprintCallable, Category = "Animation")
    UAnimMontage* GetMontageByTag(FGameplayTag Tag) const;
};

/*
콘텐츠 브라우저에서 기타 -> 데이터 에셋 으로 해당 클래스 기반으로 생성*/