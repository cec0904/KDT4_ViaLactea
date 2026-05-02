// Fill out your copyright notice in the Description page of Project Settings.


#include "Base/Data/Animation/VL_AnimSkillSet.h"

UAnimMontage* UVL_AnimSkillSet::GetMontageByTag(FGameplayTag Tag) const
{
    // Find()는 해당 키가 없으면 nullptr을 반환
    if (const TSoftObjectPtr<UAnimMontage>* FoundPtr = SkillMontages.Find(Tag))
    {
        // 포인터가 가리키는 SoftPtr이 유효한지 확인 후 로드
        return FoundPtr->LoadSynchronous();
    }

    UE_LOG(LogTemp, Warning, TEXT("태그 [%s]에 해당하는 스킬 몽타주가 없습니다."), *Tag.ToString());
    return nullptr;
}