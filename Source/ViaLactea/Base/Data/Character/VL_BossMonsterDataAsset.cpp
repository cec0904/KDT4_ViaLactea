#include "Base/Data/Character/VL_BossMonsterDataAsset.h"
#include "../../../CustomLog/CustomLog.h"


float UVL_BossMonsterDataAsset::GetCooldownValue(FGameplayTag Tag) const
{
    if (!Tag.IsValid())
    {
        //CUSTOM_LOG("오류: 전달된 태그가 유효하지 않습니다!");
        return 0.0f;
    }
    for (const FBossPatternData& Pattern : BossPatterns)
    {
        if (Pattern.PatternTag == Tag)
        {
            return Pattern.Cooldown;
        }

    }
    return Super::GetCooldownValue(Tag);
}

const FBossPatternData* UVL_BossMonsterDataAsset::GetPatternData(EAIBossPattern Type) const
{
    for (const FBossPatternData& Pattern : BossPatterns)
    {
        if (Pattern.PatternType == Type)
        {
            return &Pattern;
        }
    }
    return nullptr;
}

const FBossPatternData* UVL_BossMonsterDataAsset::GetPatternDataByTag(FGameplayTag InTag) const
{
    for (const FBossPatternData& Pattern : BossPatterns)
    {
        // 입력받은 태그가 패턴의 태그와 일치하는지 확인
        if (Pattern.PatternTag.MatchesTagExact(InTag))
        {
            return &Pattern;
        }
    }
    return nullptr;
}