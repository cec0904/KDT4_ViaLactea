// Fill out your copyright notice in the Description page of Project Settings.


#include "VL_CharacterAnimInstanceBase.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "KismetAnimationLibrary.h"

#include "CustomLog/CustomLog.h"

void UVL_CharacterAnimInstanceBase::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
    if (OwnerCharacter)
    {
        // 무브먼트 컴포넌트 캐싱 (매 프레임 Cast하는 비용 절약)
        OwnerMovementComponent = OwnerCharacter->GetCharacterMovement();
    }
}

void UVL_CharacterAnimInstanceBase::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (OwnerCharacter == nullptr)
	{

		OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
		if (OwnerCharacter == nullptr) return;

		// 캐릭터가 뒤늦게 찾아졌다면 무브먼트도 다시 세팅
		OwnerMovementComponent = OwnerCharacter->GetCharacterMovement();
	}

	if (OwnerCharacter && OwnerMovementComponent)
	{
		bIsAccelerating = OwnerMovementComponent->GetCurrentAcceleration().Size() > 0.f;

		bIsInAir = OwnerMovementComponent->IsFalling();

		FVector Velocity = OwnerCharacter->GetVelocity();
		GroundSpeed = Velocity.Size2D();
		if (GroundSpeed > 0.f)
		{


			FRotator AimRotation = OwnerCharacter->GetBaseAimRotation();
			FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(Velocity);

			// 캐릭터의 시선 방향 대비 이동 방향의 차이(-180~180)를 구함

			Direction = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;
		}

		FRotator ActorRotation = OwnerCharacter->GetActorRotation();
		FRotator AimRotation = OwnerCharacter->GetBaseAimRotation();

		// 2. Aim Offset 계산 (보스가 타겟을 바라보는 상대적 각도)
		// Delta를 구한 뒤 Normalize를 통해 -180 ~ 180 사이 값으로 변환
		FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(AimRotation, ActorRotation);

		// 3. 보간을 통한 부드러운 값 적용 (급격한 회전 방지)
		AimYaw = FMath::FInterpTo(AimYaw, Delta.Yaw, DeltaSeconds, InterpSpeed);
		AimPitch = FMath::FInterpTo(AimPitch, Delta.Pitch, DeltaSeconds, InterpSpeed);

		// 4. 제자리 회전(Turn In Place)을 위한 로직 (옵션)
		// 움직이지 않을 때 보스의 몸체와 발의 방향 차이를 계산하여 애니메이션에서 사용
		if (OwnerCharacter->GetVelocity().Size() < 1.f)
		{
			RootYawOffset = FMath::FInterpTo(RootYawOffset, Delta.Yaw, DeltaSeconds, InterpSpeed);
		}
		else
		{
			// 이동 중일 때는 오프셋을 초기화하여 정면을 보게 함
			RootYawOffset = FMath::FInterpTo(RootYawOffset, 0.f, DeltaSeconds, InterpSpeed);
		}

		//bFullBody = IsAnyMontagePlaying();
	}
	else
	{
		CUSTOM_LOG("프레임 갱신 못함");
	}

}