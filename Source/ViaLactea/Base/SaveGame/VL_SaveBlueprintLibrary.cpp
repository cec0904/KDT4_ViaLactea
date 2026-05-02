// Fill out your copyright notice in the Description page of Project Settings.


#include "Base/SaveGame/VL_SaveBlueprintLibrary.h"

#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"

#include "Player/VL_PlayerState.h"
#include "Base/SaveGame/VL_SaveSubsystem.h"

// 현재 playerstate 에서 값들을 모아서 저장
bool UVL_SaveBlueprintLibrary::SaveCurrentPlayer(UObject* WorldContextObject, APlayerController* PlayerController)
{
    if (!WorldContextObject || !PlayerController)
    {
        return false;
    }

    UWorld* World = WorldContextObject->GetWorld();
    if (!World)
    {
        return false;
    }

    UGameInstance* GameInstance = World->GetGameInstance();
    if (!GameInstance)
    {
        return false;
    }

    UVL_SaveSubsystem* SaveSubSystem = GameInstance->GetSubsystem<UVL_SaveSubsystem>();
    if (!SaveSubSystem)
    {
        return false;
    }

    AVL_PlayerState* VLPlayerState = Cast<AVL_PlayerState>(PlayerController->PlayerState);
    if (!VLPlayerState)
    {
        return false;
    }

    return SaveSubSystem->SavePlayerStateToCharacterSlot(VLPlayerState);
}

// 선택한 슬롯의 세이브 파일을 읽어서 playerstate 값, pawn 위치값 적용
bool UVL_SaveBlueprintLibrary::LoadSelectedCharacter(UObject* WorldContextObject, APlayerController* PlayerController, const FString& UserId, int32 CharacterSlotId)
{
    if (!WorldContextObject || !PlayerController)
    {
        return false;
    }

    UWorld* World = WorldContextObject->GetWorld();
    if (!World)
    {
        return false;
    }

    UGameInstance* GameInstance = World->GetGameInstance();
    if (!GameInstance)
    {
        return false;
    }

    UVL_SaveSubsystem* SaveSubSystem = GameInstance->GetSubsystem<UVL_SaveSubsystem>();
    if (!SaveSubSystem)
    {
        return false;
    }

    AVL_PlayerState* VLPlayerState = Cast<AVL_PlayerState>(PlayerController->PlayerState);
    if (!VLPlayerState)
    {
        return false;
    }

    return SaveSubSystem->LoadPlayerStateFromCharacterSlot(VLPlayerState, UserId, CharacterSlotId);
}

// 새 게임 시작 혹은 슬롯 선택 직후 먼저 넣는 값
bool UVL_SaveBlueprintLibrary::SetCurrentPlayerIdentity(UObject* WorldContextObject, APlayerController* PlayerController, const FString& UserId, int32 CharacterSlotId, const FString& CharacterName)
{
    if (!WorldContextObject || !PlayerController)
    {
        return false;
    }

    AVL_PlayerState* VLPLayerState = Cast<AVL_PlayerState>(PlayerController->PlayerState);
    if (!VLPLayerState)
    {
        return false;
    }

    VLPLayerState->SetPlayerIdentity(UserId, CharacterSlotId, CharacterName);
    return true;
}

