// Fill out your copyright notice in the Description page of Project Settings.


#include "Base/Data/Animation/VL_CharacterMontageData.h"

UAnimMontage* UVL_CharacterMontageData::GetMontageByTag(FGameplayTag Tag) const
{
    for (const FVL_TagToMontage& Entry : MontageMap)
    {
        // 1. MatchesTagExact: 정확히 일치하는 태그만 찾음 (추천)
        // 2. MatchesTag: 부모 태그까지 포함해서 찾음 (유연함)
        if (Entry.ActionTag.MatchesTagExact(Tag))
        {
            // SoftPtr이므로 메모리에 로드되지 않았을 가능성 체크
            if (Entry.AnimMontage.IsPending())
            {
                // 실시간 게임 중엔 끊김(Hitch)이 발생할 수 있으니 
                // 전투 진입 시 미리 로드해두는 것이 좋습니다.
                return Entry.AnimMontage.LoadSynchronous();
            }
            return Entry.AnimMontage.Get();
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("[%s] 대응하는 몽타주를 찾을 수 없습니다: %s"),
        *GetName(), *Tag.ToString());
    return nullptr;
}

/*
캐릭터에서 사용법:
1. 몽타주 데이터 에셋과 무기데이터 에셋을 헤더로 정의
UPROPERTY(EditAnywhere, Category = "Animation")
TObjectPtr<class UVL_CharacterMontageData> MyMontageData;

// 현재 들고 있는 무기 데이터 (ActionData를 포함하고 있음)
UPROPERTY(VisibleAnywhere, Category = "Weapon")
TObjectPtr<class UVL_WeaponDataAsset> CurrentWeapon;

void AVL_CharacterBase::Attack()
{
    if (!CurrentWeapon || !MyMontageData) return;

    // 1. 무기에서 현재 실행할 액션의 태그를 가져옴
    // (예: 무기에 등록된 기본 공격 액션의 태그 "Action.Attack.Normal")
    FGameplayTag ActionTag = CurrentWeapon->GetDefaultAttackTag();

    // 2. 내 주소록에서 내 전용 몽타주를 찾음
    UAnimMontage* SelectedMontage = MyMontageData->GetMontageByTag(ActionTag);

    if (SelectedMontage)
    {
        PlayAnimMontage(SelectedMontage);
    }
}

캐릭터가 무기를 바꿀 때 해당 무기에 포함된 액션 태그들을 
미리 스캔하여 애니메이션을 로드하는 'Preload' 로직 알아보기
*/