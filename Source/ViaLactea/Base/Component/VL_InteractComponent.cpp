#include "Base/Component/VL_InteractComponent.h"
#include "Kismet/GameplayStatics.h"

UVL_InteractComponent::UVL_InteractComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}


// Called when the game starts
void UVL_InteractComponent::BeginPlay()
{
	Super::BeginPlay();

	// InteractionWidget이 명시적으로 지정되지 않았다면, 오너 액터에서 WidgetComponent를 자동으로 찾습니다.
	if (!InteractionWidget)
	{
		InteractionWidget = GetOwner()->FindComponentByClass<UWidgetComponent>();
	}

	// 시작 시 UI 숨김
	ShowInteractionWidget(false);
	
}


// Called every frame
void UVL_InteractComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (GetNetMode() == NM_DedicatedServer) return;

	// 위젯이 보일 때만 회전
	if (InteractionWidget && InteractionWidget->IsVisible())
	{
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (PC && PC->PlayerCameraManager)
		{
			FVector CameraLocation = PC->PlayerCameraManager->GetCameraLocation();
			FVector WidgetLocation = InteractionWidget->GetComponentLocation();
			FVector Direction = CameraLocation - WidgetLocation;

			FRotator TargetRot = Direction.Rotation();
			InteractionWidget->SetWorldRotation(FRotator(0.f, TargetRot.Yaw, 0.f));
		}
	}
}

void UVL_InteractComponent::Interact(AActor* Interactor)
{
	// 델리게이트를 통해 오너 액터(NPC, 아이템 등)에게 상호작용 당했음을 알림
	OnInteractEvent.Broadcast(Interactor);
}

void UVL_InteractComponent::ShowInteractionWidget(bool bShow)
{
	if (InteractionWidget)
	{
		InteractionWidget->SetVisibility(bShow);
	}
}