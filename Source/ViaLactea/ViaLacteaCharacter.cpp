// Copyright Epic Games, Inc. All Rights Reserved.

#include "ViaLacteaCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "ViaLactea.h"

//0212khb
#include "Actor/Gimmik/Component/EnvironmentDamageComponent.h"
#include "Net/UnrealNetwork.h"
#include "Components/SphereComponent.h"
#include "Actor/Gimmik/InteractionActor.h"
#include "Kismet/GameplayStatics.h"
#include "Base/Component/VL_InteractComponent.h"



#include "CustomLog/CustomLog.h"

AViaLacteaCharacter::AViaLacteaCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	//260212KHB
	CurrentHP = 100.f;
	EnvironmentDamageComp = CreateDefaultSubobject<UEnvironmentDamageComponent>(TEXT("EnvDamageComp_New"));

	InteractionCheckSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionCheckSphere"));
	InteractionCheckSphere->SetupAttachment(RootComponent);
	InteractionCheckSphere->SetSphereRadius(200.0f); // 캐릭터 주변 2.5미터

	InteractionCheckSphere->SetCollisionProfileName(TEXT("Trigger"));
}

void AViaLacteaCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	CUSTOM_LOG("KEY BIND Start. ");
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AViaLacteaCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AViaLacteaCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AViaLacteaCharacter::Look);
		if (InteractionAction)
		{
			// 4. 자식 클래스의 함수(Interact)에 바인딩!
			EnhancedInputComponent->BindAction(InteractionAction, ETriggerEvent::Started, this, &AViaLacteaCharacter::OnInteractKeyPressed);

			CUSTOM_LOG("E INERTACT KEY BIND SUCCESS : %s", *GetName());
			UE_LOG(LogTemp, Display, TEXT("E INERTACT KEY BIND SUCCESS : %s"), *GetName());
		}
		else
		{
			UE_LOG(LogTemp, Display, TEXT("E INERTACT KEY BIND Failed. ") );
			CUSTOM_LOG("E INERTACT KEY BIND Failed.");


		}
	}
	else
	{
		UE_LOG(LogViaLactea, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AViaLacteaCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void AViaLacteaCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AViaLacteaCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void AViaLacteaCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AViaLacteaCharacter::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void AViaLacteaCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}

float AViaLacteaCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// 1. Damage calculation by parent function call
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	// 2. Change HP in Only SERVER
	if (HasAuthority() && ActualDamage > 0.f && CurrentHP > 0.f)
	{
		// HP compare to 0
		CurrentHP = FMath::Max(CurrentHP - ActualDamage, 0.f);

		CUSTOM_LOG("현재 HP: %f", CurrentHP);

		// 3. A character dies when HP reaches 0.
		if (CurrentHP <= 0.f)
		{
			// 재스폰 대기 시간, 스폰위치 등 초기화하는 함수 필요
			//Die();
			UE_LOG(LogTemp, Warning, TEXT("[%s] 캐릭터가 사망했습니다!"), *GetName());
		}
	}
	return ActualDamage;
}

void AViaLacteaCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AViaLacteaCharacter, CurrentHP);
}

void AViaLacteaCharacter::OnInteractKeyPressed()
{
	CUSTOM_LOG("Client: E Key Pressed");

	if (CurrentInteractable)
	{
		// 네트워크 안정성을 위해 RPC로는 컴포넌트 자체가 아닌 '액터'를 전달합니다.
		ServerRPCInteract(CurrentInteractable->GetOwner());
	}
	else
	{
		CUSTOM_LOG("No Target Found Nearby");
	}
}

bool AViaLacteaCharacter::ServerRPCInteract_Validate(AActor* TargetActor)
{
	if (!TargetActor) return true;

	// 거리 검증: 클라이언트가 핵으로 멀리 있는 걸 줍는지 확인
	float Dist = GetDistanceTo(TargetActor);
	return Dist <= 400.0f; // Sphere 반지름보다 약간 넉넉하게  
}

void AViaLacteaCharacter::ServerRPCInteract_Implementation(AActor* TargetActor)
{
	if (!TargetActor) return;

	// 서버 측에서 해당 액터의 컴포넌트를 찾아 실행
	UVL_InteractComponent* InteractComp = TargetActor->FindComponentByClass<UVL_InteractComponent>();
	if (InteractComp)
	{
		// 상호작용 실행! (이 컴포넌트에 바인딩된 NPC 대화나 아이템 획득 로직이 트리거됨)
		InteractComp->Interact(this);
		CUSTOM_LOG("Interacted With %s", *TargetActor->GetName());
	}
}

void AViaLacteaCharacter::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PlayerController = Cast<APlayerController>(Controller);

	if (IsValid(PlayerController))
	{
		UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
		if (IsValid(InputSubsystem) && DefaultMappingContext)
		{
			InputSubsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	// 내 화면(로컬 컨트롤러)에서만 상호작용 UI 체크 타이머 가동
	if (IsLocallyControlled())
	{
		GetWorldTimerManager().SetTimer(
			InteractionTimerHandle,
			this,
			&AViaLacteaCharacter::CheckInteractionTarget,
			0.2f, // 0.2초 간격
			true  // 반복 여부
		);
	}
}

void AViaLacteaCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 캐릭터가 사라질 때 타이머 해제
	GetWorldTimerManager().ClearTimer(InteractionTimerHandle);

	Super::EndPlay(EndPlayReason);
}

UVL_InteractComponent* AViaLacteaCharacter::FindBestInteractionTarget() const
{
	TArray<AActor*> OverlappingActors;
	// 특정 클래스가 아닌 모든 액터를 오버랩으로 가져옵니다 (또는 Collision Profile로 필터링)
	InteractionCheckSphere->GetOverlappingActors(OverlappingActors);

	UVL_InteractComponent* ClosestInteractable = nullptr;
	float MinDistance = MAX_FLT;

	for (AActor* Actor : OverlappingActors)
	{
		if (Actor && Actor != this)
		{
			// 액터가 UVL_InteractComponent를 가지고 있는지 확인
			UVL_InteractComponent* InteractComp = Actor->FindComponentByClass<UVL_InteractComponent>();

			if (InteractComp)
			{
				float Dist = GetDistanceTo(Actor);
				if (Dist < MinDistance)
				{
					MinDistance = Dist;
					ClosestInteractable = InteractComp;
				}
			}
		}
	}
	return ClosestInteractable;
}

void AViaLacteaCharacter::CheckInteractionTarget()
{
	UVL_InteractComponent* NewInteractable = FindBestInteractionTarget();

	if (CurrentInteractable != NewInteractable)
	{
		if (CurrentInteractable)
		{
			CurrentInteractable->ShowInteractionWidget(false);
		}

		CurrentInteractable = NewInteractable;

		if (CurrentInteractable)
		{
			CurrentInteractable->ShowInteractionWidget(true);
		}
	}

	// [추가 예외 처리] 타겟은 그대로인데, 갑자기 거리가 멀어진 경우 (물리적 예외)
	if (CurrentInteractable)
	{
		float Dist = GetDistanceTo(CurrentInteractable->GetOwner());
		if (Dist > 450.0f) // Sphere 반지름보다 큰 값
		{
			CurrentInteractable->ShowInteractionWidget(false);
			CurrentInteractable = nullptr;
		}
	}
}
