// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnvironmentDamageComponent.generated.h"

class UPrimitiveComponent;
class AViaLacteaCharacter;

UCLASS()
class VIALACTEA_API UEnvironmentDamageComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	// Sets default values for this component's properties
	UEnvironmentDamageComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// Server TimerHandle
	FTimerHandle DamageTimerHandle;


	// Overlap Event Function
protected:
	// 컴포넌트 전용 오버랩 함수 (매개변수 중요!)
	UFUNCTION()
	void OnCapsuleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnCapsuleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void ApplyEnvironmentDamage();

protected:
	UFUNCTION()
	void OnActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);

	UFUNCTION()
	void OnActorEndOverlap(AActor* OverlappedActor, AActor* OtherActor);
private:
	// 나를 들고 있는 주인 캐릭터를 미리 메모리에 할당
	UPROPERTY()
	AViaLacteaCharacter* CachedOwner;
};
