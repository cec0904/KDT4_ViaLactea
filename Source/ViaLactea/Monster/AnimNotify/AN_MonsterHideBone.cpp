#include "Monster/AnimNotify/AN_MonsterHideBone.h"
#include "Monster/Boss/VL_Boss1.h"


void UAN_MonsterHideBone::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp || !MeshComp->GetOwner()) return;

    // 캐릭터 클래스로 캐스팅 (AMyCharacter는 예시)
    if (AVL_Boss1* Monster = Cast<AVL_Boss1>(MeshComp->GetOwner()))
    {

        // 서버에서 실행 중인지 확인 후 멀티캐스트 호출
        if (Monster->HasAuthority())
        {
            Monster->Multicast_HideBones(BoneNames, bShouldHide); // bShouldHide는 노티파이 변수
        }
    }
}
