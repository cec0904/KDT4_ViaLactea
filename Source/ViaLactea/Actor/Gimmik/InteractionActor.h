// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractionActor.generated.h"


UCLASS()
class VIALACTEA_API AInteractionActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AInteractionActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	// 실제 표시될 메쉬
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Interaction")
	class UStaticMeshComponent* BaseMesh;

	// Widghet UI component
	UPROPERTY(VisibleAnywhere, Category = "UI")
	class UWidgetComponent* InteractionWidget;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// 1. 컴포넌트 선언
	UPROPERTY(VisibleAnywhere, Category = "Interaction")
	class UVL_InteractComponent* InteractComponent;

	// 2. 상호작용 시 호출될 실제 로직 함수
	UFUNCTION()
	void HandleOnInteract(AActor* Interactor);
//
//public: // 위젯보여주는 함수
//	void ShowInteractionWidget(bool bShow);
//
//
//public:
//	// 상호작용 시 호출될 함수 (Blueprint에서 오버라이드 가능)
//	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
//	void OnInteract();
//
//public:
//	// Called every frame
//	virtual void Tick(float DeltaTime) override;
};
