#include "Base/Data/Animation/VL_AnimComboSet.h"
#include "CustomLog/CustomLog.h"

UAnimMontage* UVL_AnimComboSet::GetMontageByIndex(int32 Index) const
{
    if (ComboMontages.IsValidIndex(Index))
    {
        // 유효한 인덱스라면 로드 후 반환
        return ComboMontages[Index].LoadSynchronous();
    }
    // 인덱스 범위를 벗어나면 로그를 찍고 null 반환
    CUSTOM_LOG("Combo Index %d is invalid!", Index);
    return nullptr;
}