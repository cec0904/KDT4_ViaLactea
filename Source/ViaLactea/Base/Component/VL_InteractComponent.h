// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/WidgetComponent.h"
#include "VL_InteractComponent.generated.h"

// 상호작용 이벤트를 블루프린트나 다른 C++ 클래스에 전달하기 위한 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractSignature, AActor*, Interactor);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VIALACTEA_API UVL_InteractComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UVL_InteractComponent();

protected:
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// 상호작용 시 호출될 함수
	void Interact(AActor* Interactor);

	void ShowInteractionWidget(bool bShow);

	// 이벤트 델리게이트 (NPC나 아이템이 블루프린트에서 이 이벤트에 노드를 연결해 각자 행동 정의)
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnInteractSignature OnInteractEvent;

	// UI를 띄울 위젯 컴포넌트
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	UWidgetComponent* InteractionWidget = nullptr;
};
