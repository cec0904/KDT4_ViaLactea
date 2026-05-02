#include "Monster/AnimNotify/ANS_BossRotation.h"
#include "Monster/Boss/VL_Boss1.h"

void UANS_BossRotation::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

    if (MeshComp && MeshComp->GetOwner())
    {
        // 보스 클래스로 형변환 후 회전 함수 호출
        if (AVL_Boss1* Boss = Cast<AVL_Boss1>(MeshComp->GetOwner()))
        {
            Boss->RotateToTarget(FrameDeltaTime);

            //Boss->MoveToTarget(FrameDeltaTime, 200.0f);
        }
    }
}
