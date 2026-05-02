// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PoisonFogActor.generated.h"


UCLASS()
class VIALACTEA_API APoisonFogActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APoisonFogActor();

	UFUNCTION()
	void DestroyFog();


	UFUNCTION()
	void HandleBossDeath();

protected:
	UPROPERTY(VisibleAnywhere, Category = "FX")
	class UNiagaraComponent* FogNiagaraComponent;

	// 충돌 범위 (캐릭터가 들어왔는지 감지)
	UPROPERTY(VisibleAnywhere, Category = "Collision")
	class USphereComponent* FogCollision;

	// 대미지 로직 관련 함수
	UFUNCTION()
	void OnFogBeginOverlap(class UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnFogEndOverlap(class UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	UFUNCTION()
	void ApplyPoisonDamage(AActor* TargetActor);

private:
	float DamageAmount = 5.0f; // 초당 대미지

	// 캐릭터별로 타이머를 관리하기 위한 맵 (여러 명 대응)
	// 충돌 상태 클라이언트 별로 관리 데이터 테이블 필요 없으면 안해도 됨
	TMap<AActor*, FTimerHandle> DamageTimers;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


};
