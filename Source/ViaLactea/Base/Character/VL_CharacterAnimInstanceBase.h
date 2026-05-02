// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "VL_CharacterAnimInstanceBase.generated.h"

class ACharacter;
class UCharacterMovementComponent;

UCLASS()
class VIALACTEA_API UVL_CharacterAnimInstanceBase : public UAnimInstance
{
	GENERATED_BODY()
public:
	// 초기화 (BeginPlay와 유사)
	virtual void NativeInitializeAnimation() override;

	// 매 프레임 업데이트 (Tick과 유사)
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	ACharacter* OwnerCharacter = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	UCharacterMovementComponent* OwnerMovementComponent = nullptr;

	// 스테이트 머신 규칙(Transition Rule)에 꽂을 변수들
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float GroundSpeed = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bIsInAir = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float Direction = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsAccelerating = false;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bShouldMove = false; // 가속도(입력)가 있는지 여부

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	float AimYaw = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	float AimPitch = 0.f;

	// 제자리 회전(Turn In Place) 감지용 Delta Yaw
	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	float RootYawOffset = false;

	// 보간 속도
	UPROPERTY(EditAnywhere, Category = "Animation")
	float InterpSpeed = 10.f;

public:
	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	bool bFullBody = false;
};
