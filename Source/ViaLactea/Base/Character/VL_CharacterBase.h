// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Base/Interfaces/VL_CooldownProvider.h"
#include "VL_CharacterBase.generated.h"


class UVL_CharacterDataAsset;

UCLASS()
class VIALACTEA_API AVL_CharacterBase : public ACharacter, public IVL_CooldownProvider
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AVL_CharacterBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	// 인터페이스 구현부
	virtual float GetCooldownValue(FGameplayTag Tag) const override;

protected:
	// 모든 캐릭터가 공통으로 가질 데이터 에셋 (플레이어 에셋, 몬스터 에셋의 부모)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
	TObjectPtr<UVL_CharacterDataAsset> CharacterDataAsset;


};
