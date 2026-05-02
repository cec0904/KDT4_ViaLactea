#include "Monster/Boss/Animation/BossAnimInstanceBase.h"
#include "../VL_Boss1.h"

void UBossAnimInstanceBase::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    // 미리 캐스팅해서 저장해둡니다.
    OwnerBoss = Cast<AVL_Boss1>(OwnerCharacter);
}

void UBossAnimInstanceBase::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerBoss && OwnerCharacter)
    {
        OwnerBoss = Cast<AVL_Boss1>(OwnerCharacter);
    }
    if (OwnerBoss)
    {
        bIsRushing = OwnerBoss->GetIsRushing();
    }

}