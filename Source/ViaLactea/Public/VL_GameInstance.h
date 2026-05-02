// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AdvancedFriendsGameInstance.h"
#include "VL_GameInstance.generated.h"


class UVL_SaveSubsystem;
/**
 * 
 */
UCLASS()
class VIALACTEA_API UVL_GameInstance : public UAdvancedFriendsGameInstance
{
	GENERATED_BODY()

public :
	UFUNCTION(BlueprintPure, Category = "VL|Save")
	UVL_SaveSubsystem* GetVL_SaveSubsystem() const;

	UVL_GameInstance(const FObjectInitializer& ObjectInitializer);

	virtual void Init() override;

	//////////////////////////////////////////////////
	// Account
	UFUNCTION(BlueprintCallable, Category = "VL|Account")
	void SetCurrentUserId(const FString& InUserId);

	UFUNCTION(BlueprintPure, Category = "VL|Account")
	FString GetCurrentUserId() const;


	//////////////////////////////////////////////////
	// Character Selection
	UFUNCTION(BlueprintCallable, Category = "VL|Character")
	void SetCurrentCharacterInfo(int32 InCharacterSlotId, const FString& InCharacterName);

	UFUNCTION(BlueprintCallable, Category = "VL|Character")
	void SetCurrentCharacterSelection(const FString& InUserId, int32 InCharacterSlotId, const FString& InCharacterName);

	UFUNCTION(BlueprintPure, Category = "VL|Character")
	int32 GetCurrentCharacterSlotId() const;

	UFUNCTION(BlueprintPure, Category = "VL|Character")
	FString GetCurrentCharacterName() const;

	UFUNCTION(BlueprintCallable, Category = "VL|Character")
	void ClearCurrentCharacterInfo();

	UFUNCTION(BlueprintPure, Category = "VL|Character")
	bool HasValidCurrentCharacter() const;

	//////////////////////////////////////////////////
	// World Selection
	UFUNCTION(BlueprintCallable, Category = "VL|World")
	void SetCurrentWorldInfo(int32 InWorldSlotId, const FString& InWorldName, FName InMapName);

	UFUNCTION(BlueprintPure, Category = "VL|World")
	int32 GetCurrentWorldSlotId() const;

	UFUNCTION(BlueprintPure, Category = "VL|World")
	FString GetCurrentWorldName() const;

	UFUNCTION(BlueprintPure, Category = "VL|World")
	FName GetCurrentMapName() const;

	UFUNCTION(BlueprintCallable, Category = "VL|World")
	void ClearCurrentWorldInfo();

	UFUNCTION(BlueprintPure, Category = "VL|World")
	bool HasValidCurrentWorld() const;

	//////////////////////////////////////////////////
	// 전체 초기화
	UFUNCTION(BlueprintCallable, Category = "VL|State")
	void ClearCurrentSelectionState();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "VL|Account")
	FString CurrentUserId;

	UPROPERTY(BlueprintReadOnly, Category = "VL|Character")
	int32 CurrentCharacterSlotId = -1;

	UPROPERTY(BlueprintReadOnly, Category = "VL|Character")
	FString CurrentCharacterName;

	UPROPERTY(BlueprintReadOnly, Category = "VL|World")
	int32 CurrentWorldSlotId = -1;

	UPROPERTY(BlueprintReadOnly, Category = "VL|World")
	FString CurrentWorldName;

	UPROPERTY(BlueprintReadOnly, Category = "VL|World")
	FName CurrentMapName = NAME_None;
	
};
