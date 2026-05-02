// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "ViaLacteaCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;
//260212_KHB forward declaration
class UEnvironmentDamageComponent;
class USphereComponent;
class AInteractionActor;
class AController;
class UVL_InteractComponent;
//

/**
 *  A simple player-controllable third person character
 *  Implements a controllable orbiting camera
 */
UCLASS(abstract)
class AViaLacteaCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
protected:

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MouseLookAction;

	//260212 KHB ADD//
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* InteractionAction;

public:

	/** Constructor */
	AViaLacteaCharacter();	

protected:

	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

public:
	////////260212KHB
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	class UInputMappingContext* DefaultMappingContext;
	////////////
	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles look inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }


/// <summary>
/// 260212_KHB_ADD
/// </summary>

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UEnvironmentDamageComponent* EnvironmentDamageComp;

	UPROPERTY(VisibleAnywhere)
	USphereComponent* InteractionCheckSphere;

public:
	// TakeDamage Override
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


protected:
	// 1. Replicated Value 
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Character")
	float CurrentHP;
public:
	// E 키 입력 처리
	void OnInteractKeyPressed();

	// 서버에 상호작용 실행을 요청하는 RPC
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRPCInteract(AActor* TargetActor);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	UVL_InteractComponent* FindBestInteractionTarget() const;

protected:
	// 타이머 핸들 선언
	FTimerHandle InteractionTimerHandle;

	// 타이머에 의해 실행될 함수
	void CheckInteractionTarget();

	// 현재 UI가 켜져 있는 엑터를 기억하기 위한 변수
	UPROPERTY()
	AInteractionActor* CurrentInteractionActor;

	UPROPERTY()
	UVL_InteractComponent* CurrentInteractable;
};

