#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VL_FinalBossAxe.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;
class URotatingMovementComponent;

UCLASS()
class VIALACTEA_API AVL_FinalBossAxe : public AActor
{
	GENERATED_BODY()
	
public:	
	AVL_FinalBossAxe();

protected:
	virtual void BeginPlay() override;

public:	
	//virtual void Tick(float DeltaTime) override;

	/** 충돌체 (루트 컴포넌트) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* CollisionComponent;

	/** 도끼 메쉬 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* AxeMesh;

	/** 투사체 이동 컴포넌트 (던지기 물리 담당) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UProjectileMovementComponent* ProjectileMovement;

	/** 회전 컴포넌트 (뱅글뱅글 도는 연출 담당) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	URotatingMovementComponent* RotatingMovement;

	/** 던지기 초기화 함수 (보스가 호출) */
	void FireInDirection(const FVector& ShootDirection);

protected:
	UFUNCTION()
	void OnAxeOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnAxeHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	
	///** 서버에서 복제하기 위한 설정 */
	//virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 나중에 투척체가 많이 생기면 범용 투첵체 데이터 에셋으로 관리
	float DamageAmount = 20.f;
};
