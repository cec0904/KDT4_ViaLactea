#include "Monster/AnimNotify/AN_CheckCombo.h"
#include "Monster/Boss/VL_Boss1.h"
#include "CustomLog/CustomLog.h"

void UAN_CheckCombo::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    if (MeshComp && MeshComp->GetOwner())
    {
        if (AVL_Boss1* Boss = Cast<AVL_Boss1>(MeshComp->GetOwner()))
        {
            Boss->CheckNextCombo();
        }
    }
}