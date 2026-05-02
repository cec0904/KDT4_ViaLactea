#include "Monster/AnimNotify/AN_ThrowAxe.h"
#include "Monster/Boss/VL_Boss1.h"

void UAN_ThrowAxe::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp || !MeshComp->GetOwner()) return;

	// 1. 보스 클래스로 캐스팅
	AVL_Boss1* Boss = Cast<AVL_Boss1>(MeshComp->GetOwner());
	if (Boss)
	{
		if (Boss->HasAuthority())
		{
			Boss->ThrowAxe(bIsRightHand);
		}
	}
}
