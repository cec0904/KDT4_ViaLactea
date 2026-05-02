// Fill out your copyright notice in the Description page of Project Settings.


#include "VL_GameInstance.h"
#include "Base/SaveGame/VL_SaveSubsystem.h"

UVL_GameInstance::UVL_GameInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CurrentCharacterSlotId = -1;
	CurrentWorldSlotId = -1;
	CurrentMapName = NAME_None;
}

void UVL_GameInstance::Init()
{
	Super::Init();

	ClearCurrentSelectionState();

	UVL_SaveSubsystem* SaveSubsystem = GetVL_SaveSubsystem();
	if (!SaveSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("VL_GameInstance::Init - SaveSubsystem is nullptr"));
		return;
	}

	FString PlatformUserId;
	if (SaveSubsystem->GetPlatformUserId(PlatformUserId))
	{
		CurrentUserId = PlatformUserId;
		UE_LOG(LogTemp, Log, TEXT("VL_GameInstance::Init - CurrentUserId = %s"), *CurrentUserId);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("VL_GameInstance::Init - Failed to get platform user id"));
	}
}

UVL_SaveSubsystem* UVL_GameInstance::GetVL_SaveSubsystem() const
{
	return GetSubsystem<UVL_SaveSubsystem>();
}

void UVL_GameInstance::SetCurrentUserId(const FString& InUserId)
{
	CurrentUserId = InUserId;
}

FString UVL_GameInstance::GetCurrentUserId() const
{
	return CurrentUserId;
}

void UVL_GameInstance::SetCurrentCharacterInfo(int32 InCharacterSlotId, const FString& InCharacterName)
{
	CurrentCharacterSlotId = InCharacterSlotId;
	CurrentCharacterName = InCharacterName;
}

void UVL_GameInstance::SetCurrentCharacterSelection(const FString& InUserId, int32 InCharacterSlotId, const FString& InCharacterName)
{
	CurrentUserId = InUserId;
	CurrentCharacterSlotId = InCharacterSlotId;
	CurrentCharacterName = InCharacterName;
}

int32 UVL_GameInstance::GetCurrentCharacterSlotId() const
{
	return CurrentCharacterSlotId;
}

FString UVL_GameInstance::GetCurrentCharacterName() const
{
	return CurrentCharacterName;
}

void UVL_GameInstance::ClearCurrentCharacterInfo()
{
	CurrentCharacterSlotId = -1;
	CurrentCharacterName.Empty();
}

bool UVL_GameInstance::HasValidCurrentCharacter() const
{
	return !CurrentUserId.IsEmpty()
		&& CurrentCharacterSlotId >= 0
		&& !CurrentCharacterName.IsEmpty();
}

void UVL_GameInstance::SetCurrentWorldInfo(int32 InWorldSlotId, const FString& InWorldName, FName InMapName)
{
	CurrentWorldSlotId = InWorldSlotId;
	CurrentWorldName = InWorldName;
	CurrentMapName = InMapName;
}

int32 UVL_GameInstance::GetCurrentWorldSlotId() const
{
	return CurrentWorldSlotId;
}

FString UVL_GameInstance::GetCurrentWorldName() const
{
	return CurrentWorldName;
}

FName UVL_GameInstance::GetCurrentMapName() const
{
	return CurrentMapName;
}

void UVL_GameInstance::ClearCurrentWorldInfo()
{
	CurrentWorldSlotId = -1;
	CurrentWorldName.Empty();
	CurrentMapName = NAME_None;
}

bool UVL_GameInstance::HasValidCurrentWorld() const
{
	return CurrentWorldSlotId >= 0
		&& !CurrentWorldName.IsEmpty()
		&& !CurrentMapName.IsNone();
}

void UVL_GameInstance::ClearCurrentSelectionState()
{
	ClearCurrentCharacterInfo();
	ClearCurrentWorldInfo();
}
