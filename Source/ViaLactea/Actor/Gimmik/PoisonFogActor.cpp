// Fill out your copyright notice in the Description page of Project Settings.


#include "PoisonFogActor.h"
#include "Components/SphereComponent.h"
#include "Components/PrimitiveComponent.h"

#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/MainCharacterBase.h"

#include "../../CustomLog/CustomLog.h"


// Sets default values
APoisonFogActor::APoisonFogActor()
{
	bReplicates = true;
	SetReplicateMovement(false);

	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	// 1. 나이아가라 루트 설정
	FogNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("FogComponent"));
	FogNiagaraComponent->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> FogAsset(TEXT("/Game/Monster/Boss/Boss/FX/NS_FinalBossDark_Mist.NS_FinalBossDark_Mist"));
	if (FogAsset.Succeeded())
	{
		FogNiagaraComponent->SetAsset(FogAsset.Object);
	}
	FogNiagaraComponent->AddRelativeLocation(FVector(-620.f, 0.f, 0.f));

	// 2. 충돌 영역 설정
	FogCollision = CreateDefaultSubobject<USphereComponent>(TEXT("FogCollision"));
	FogCollision->SetupAttachment(RootComponent);
	FogCollision->SetSphereRadius(4000.0f);

	// 초기 상태 설정
	FogNiagaraComponent->bAutoActivate = true;
}

void APoisonFogActor::OnFogBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 캐릭터가 들어왔는지 확인 (플레이어 캐릭터만 하려면 플레이어 이름 적기)
	if (OtherActor && OtherActor->IsA(AMainCharacterBase::StaticClass()))
	{
		if (DamageTimers.Contains(OtherActor))
		{
			return;
		}
		// 1초마다 ApplyPoisonDamage 함수를 호출하는 타이머 가동
		FTimerHandle NewHandle;
		FTimerDelegate DamageDelegate;
		DamageDelegate.BindUObject(this, &APoisonFogActor::ApplyPoisonDamage, OtherActor);

		GetWorldTimerManager().SetTimer(NewHandle, DamageDelegate, 3.0f, true, 0.0f); // 0초 대기 후 즉시 시작

		// 관리 맵에 저장
		DamageTimers.Add(OtherActor, NewHandle);
	}
}

void APoisonFogActor::OnFogEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// 나간 캐릭터의 타이머를 제거
	if (OtherActor && DamageTimers.Contains(OtherActor))
	if(OtherActor)
	{
		GetWorldTimerManager().ClearTimer(DamageTimers[OtherActor]);
		DamageTimers.Remove(OtherActor);

		//CUSTOM_LOG("%s 독에서 벗어남 - 타이머 정지", *OtherActor->GetName());
	}
}

void APoisonFogActor::ApplyPoisonDamage(AActor* TargetActor)
{
	if (TargetActor && HasAuthority())
	{
		UGameplayStatics::ApplyDamage(TargetActor, DamageAmount, nullptr, this, UDamageType::StaticClass());
	}
}

// Called when the game starts or when spawned
void APoisonFogActor::BeginPlay()
{
	Super::BeginPlay();
	//if (FogNiagaraComponent)
	//{
	//	// 1. Color (Purple 1.0)
	//	//FLinearColor PoisonColor = FLinearColor(0.5f, 0.0f, 1.0f, 1.0f);

	//	//FogNiagaraComponent->SetVariableLinearColor(FName("User.Color"), PoisonColor);
	//}
	if (HasAuthority())
	{
		FogCollision->OnComponentBeginOverlap.AddDynamic(this, &APoisonFogActor::OnFogBeginOverlap);
		FogCollision->OnComponentEndOverlap.AddDynamic(this, &APoisonFogActor::OnFogEndOverlap);
	}

}

void APoisonFogActor::DestroyFog()
{
	Destroy(); // 스스로 파괴
}

void APoisonFogActor::HandleBossDeath()
{
	for (auto& Pair : DamageTimers)
	{
		GetWorldTimerManager().ClearTimer(Pair.Value);
	}
	DamageTimers.Empty();

	// 나이아가라 컴포넌트 비활성화 (부드러운 소멸을 위해)
	if (FogNiagaraComponent)
	{
		FogNiagaraComponent->Deactivate();
	}

	// 충돌 제거
	FogCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 액터 파괴 (약간의 유예 시간을 두어 파티클 소멸 대기)
	SetLifeSpan(1.0f);
}
