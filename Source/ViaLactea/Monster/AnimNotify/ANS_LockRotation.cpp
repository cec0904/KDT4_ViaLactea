#include "Monster/AnimNotify/ANS_LockRotation.h"
#include "Monster/Boss/VL_Boss1.h"
#include "GameFramework/CharacterMovementComponent.h"

void UANS_LockRotation::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (MeshComp && MeshComp->GetOwner())
	{
		if (AVL_Boss1* Boss = Cast<AVL_Boss1>(MeshComp->GetOwner()))
		{
			if (UCharacterMovementComponent* MovementComp = Boss->GetCharacterMovement())
			{
				// 공격 시작: 타겟을 향한 자동 회전을 멈춤
				MovementComp->bUseControllerDesiredRotation = false;
			}
			Boss->SetCanRotate(false);

		}
	}
}

void UANS_LockRotation::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (MeshComp && MeshComp->GetOwner())
	{
		if (AVL_Boss1* Boss = Cast<AVL_Boss1>(MeshComp->GetOwner()))
		{
			if (UCharacterMovementComponent* MovementComp = Boss->GetCharacterMovement())
			{
				// 공격(또는 헌신 구간) 종료: 다시 타겟을 바라보도록 허용
				MovementComp->bUseControllerDesiredRotation = true;
			}
			Boss->SetCanRotate(true);
		}
	}
}