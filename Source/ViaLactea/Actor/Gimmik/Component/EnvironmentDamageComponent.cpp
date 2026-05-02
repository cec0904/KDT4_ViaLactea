// Fill out your copyright notice in the Description page of Project Settings.


#include "EnvironmentDamageComponent.h"
#include "Components/CapsuleComponent.h"
#include "../../../ViaLacteaCharacter.h"
#include "WaterBodyActor.h"
#include "TimerManager.h"
#include "Engine/DamageEvents.h"
//#include "Components/PrimitiveComponent.h"

#include "CustomLog/CustomLog.h"

// Sets default values for this component's properties
UEnvironmentDamageComponent::UEnvironmentDamageComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	// ...
}

// Called when the game starts
void UEnvironmentDamageComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!GetOwner()->HasAuthority()) return;
	// 시작할 때 딱 한 번만 "누가 나를 들고 있는지" 확인해서 저장
	CachedOwner = Cast<AViaLacteaCharacter>(GetOwner());

	if (CachedOwner)
	{
		UCapsuleComponent* Capsule = CachedOwner->GetCapsuleComponent();
		if (Capsule)
		{
			Capsule->OnComponentBeginOverlap.AddDynamic(this, &UEnvironmentDamageComponent::OnCapsuleOverlap);
			Capsule->OnComponentEndOverlap.AddDynamic(this, &UEnvironmentDamageComponent::OnCapsuleEndOverlap);
		}
	}
}

void UEnvironmentDamageComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(DamageTimerHandle);
	}
	Super::EndPlay(EndPlayReason);
}

void UEnvironmentDamageComponent::OnCapsuleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor->IsA(AWaterBody::StaticClass()))
	{
		FTimerManager& TimerManager = GetWorld()->GetTimerManager();

		// [최적화] 이미 타이머가 돌아가고 있다면 중복 실행 방지
		if (!TimerManager.IsTimerActive(DamageTimerHandle))
		{
			CUSTOM_LOG("캡슐 물 감지: 타이머 시작");
			TimerManager.SetTimer(DamageTimerHandle, this, &UEnvironmentDamageComponent::ApplyEnvironmentDamage, 1.0f, true);

			// 진입 즉시 첫 대미지를 주고 싶다면 아래 주석 해제
			// ApplyEnvironmentDamage();
		}
	}
}

void UEnvironmentDamageComponent::OnCapsuleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && OtherActor->IsA(AWaterBody::StaticClass()))
	{
		CUSTOM_LOG("캡슐이 물에서 벗어남. 타이머 종료");
		GetWorld()->GetTimerManager().ClearTimer(DamageTimerHandle);
	}
}

void UEnvironmentDamageComponent::ApplyEnvironmentDamage()
{
	if (CachedOwner)
	{
		CachedOwner->TakeDamage(5.0f, FDamageEvent(), nullptr, nullptr);
		//CUSTOM_LOG("환경 대미지 5 적용됨.");
	}
}

void UEnvironmentDamageComponent::OnActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{

}

void UEnvironmentDamageComponent::OnActorEndOverlap(AActor* OverlappedActor, AActor* OtherActor)
{

}
