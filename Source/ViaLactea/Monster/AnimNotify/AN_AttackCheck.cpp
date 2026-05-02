#include "Monster/AnimNotify/AN_AttackCheck.h"
#include "Base/Character/VL_AICharacterBase.h"

void UAN_AttackCheck::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    if (auto* Character = Cast<AVL_AICharacterBase>(MeshComp->GetOwner()))
    {
        Character->AttackCheck(bCanParry, bShouldKnockback);
    }
}
