// Fill out your copyright notice in the Description page of Project Settings.



#include "Base/SaveGame/VL_SaveSubsystem.h"
#include "Base/SaveGame/VL_CharacterSaveGame.h"
#include "Base/SaveGame/VL_AccountSaveGame.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Player/VL_PlayerState.h"
#include "Kismet/GameplayStatics.h"

// 슬롯 이름 만들기
FString UVL_SaveSubsystem::MakeCharacterSlotName(const FString& UserId, int32 CharacterSlotId) const
{
    return FString::Printf(TEXT("Character_%s_%d"), *UserId, CharacterSlotId);
    // Character_Steam123_0
    // Character_Steam123_1
}

// 로드 or 생성
UVL_CharacterSaveGame* UVL_SaveSubsystem::LoadOrCreateCharacterSave(const FString& UserId, int32 CharacterSlotId)
{
    const FString SlotName = MakeCharacterSlotName(UserId, CharacterSlotId);

    if (UGameplayStatics::DoesSaveGameExist(SlotName, 0))
    {
        return Cast<UVL_CharacterSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
    }

    UVL_CharacterSaveGame* NewSave = Cast<UVL_CharacterSaveGame>(UGameplayStatics::CreateSaveGameObject(UVL_CharacterSaveGame::StaticClass()));

    if (!NewSave)
    {
        return nullptr;
    }

    NewSave->PlayerData.UserId = UserId;
    NewSave->PlayerData.CharacterSlotId = CharacterSlotId;

    if (!UGameplayStatics::SaveGameToSlot(NewSave, SlotName, 0))
    {
        return nullptr;
    }

    return NewSave;
}

// 현재 PlayerState 저장
bool UVL_SaveSubsystem::SavePlayerStateToCharacterSlot(AVL_PlayerState* PlayerState)
{
    if (!PlayerState)
    {
        return false;
    }

    const FString UserId = PlayerState->GetUserId();
    const int32 CharacterSlotId = PlayerState->GetCharacterSlotId();

    if (UserId.IsEmpty() || CharacterSlotId < 0)
    {
        return false;
    }

    UVL_CharacterSaveGame* CharacterSave = LoadOrCreateCharacterSave(UserId, CharacterSlotId);
    if (!CharacterSave)
    {
        return false;
    }

    CharacterSave->PlayerData = PlayerState->BuildPlayerSaveData();

    const FString SlotName = MakeCharacterSlotName(UserId, CharacterSlotId);
    return UGameplayStatics::SaveGameToSlot(CharacterSave, SlotName, 0) ;
}

// 저장된 PlayerState 불러오기
bool UVL_SaveSubsystem::LoadPlayerStateFromCharacterSlot(AVL_PlayerState* PlayerState, const FString& UserId, int32 CharacterSlotId)
{
    if (!PlayerState)
    {
        return false;
    }

    if (UserId.IsEmpty() || CharacterSlotId < 0)
    {
        return false;
    }

    const FString SlotName = MakeCharacterSlotName(UserId, CharacterSlotId);

    if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
    {
        return false;
    }

    UVL_CharacterSaveGame* CharacterSave = Cast<UVL_CharacterSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));

    if (!CharacterSave)
    {
        return false;
    }

    PlayerState->ApplyPlayerSaveData(CharacterSave->PlayerData);

    return true;
}

bool UVL_SaveSubsystem::GetPlatformUserId(FString& OutUserId) const
{
    OutUserId.Empty();

    IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
    if (!OnlineSubsystem)
    {
        return false;
    }

    IOnlineIdentityPtr IdentityInterface = OnlineSubsystem->GetIdentityInterface();
    if (!IdentityInterface.IsValid())
    {
        return false;
    }

    TSharedPtr<const FUniqueNetId> UniqueNetId = IdentityInterface->GetUniquePlayerId(0);
    if (!UniqueNetId.IsValid())
    {
        return false;
    }

    OutUserId = UniqueNetId->ToString();
    return !OutUserId.IsEmpty();
}

FString UVL_SaveSubsystem::GetAccountSlotName(const FString& UserId) const
{
    // UserId 기반으로
    return FString::Printf(TEXT("Account_%s"), *UserId);
}

UVL_AccountSaveGame* UVL_SaveSubsystem::LoadOrCreateAccountSave(const FString& UserId)
{
    // 이 계정의 AccountSave 가 있으면 로드
    // 없으면 이 계정 전용 AccountSave 생성
    if (UserId.IsEmpty())
    {
        return nullptr;
    }

    const FString SlotName = GetAccountSlotName(UserId);

    if (UGameplayStatics::DoesSaveGameExist(SlotName, 0))
    {
        return Cast<UVL_AccountSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
    }

    UVL_AccountSaveGame* NewSave =
        Cast<UVL_AccountSaveGame>(UGameplayStatics::CreateSaveGameObject(UVL_AccountSaveGame::StaticClass()));

    if (!NewSave)
    {
        return nullptr;
    }

    NewSave->UserId = UserId;

    NewSave->CharacterSlots.Empty();

    for (int32 i = 0; i < 5; i++)
    {
        FVLCharacterSlotInfo NewSlot;
        NewSlot.CharacterSlotId = i;
        NewSlot.CharacterName = TEXT("");
        NewSlot.bOccupied = false;

        NewSave->CharacterSlots.Add(NewSlot);
    }

    if (!UGameplayStatics::SaveGameToSlot(NewSave, SlotName, 0))
    {
        return nullptr;
    }

    return NewSave;
}

bool UVL_SaveSubsystem::SaveAccountUserId(const FString& UserId)
{
    // 해당 계정 전용 AccountSave 안에 UserId 저장
    if (UserId.IsEmpty())
    {
        return false;
    }

    UVL_AccountSaveGame* AccountSave = LoadOrCreateAccountSave(UserId);
    if (!AccountSave)
    {
        return false;
    }

    AccountSave->UserId = UserId;

    return UGameplayStatics::SaveGameToSlot(AccountSave, GetAccountSlotName(UserId), 0);
}

bool UVL_SaveSubsystem::LoadAccountUserId(const FString& UserId, FString& OutUserId)
{
    // 이미 알고 있는 UserId 기준으로 해당 AccountSave 안에 저장된 UserId 를 다시 읽음
    OutUserId.Empty();

    if (UserId.IsEmpty())
    {
        return false;
    }

    const FString SlotName = GetAccountSlotName(UserId);

    if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
    {
        return false;
    }

    UVL_AccountSaveGame* AccountSave =
        Cast<UVL_AccountSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));

    if (!AccountSave)
    {
        return false;
    }

    if (AccountSave->UserId.IsEmpty())
    {
        return false;
    }

    OutUserId = AccountSave->UserId;
    return true;
}

bool UVL_SaveSubsystem::GetCharacterSlots(const FString& UserId, TArray<FVLCharacterSlotInfo>& OutCharacterSlots)
{
    OutCharacterSlots.Empty();

    if (UserId.IsEmpty())
    {
        return false;
    }

    UVL_AccountSaveGame* AccountSave = LoadOrCreateAccountSave(UserId);
    if (!AccountSave)
    {
        return false;
    }

    OutCharacterSlots = AccountSave->CharacterSlots;

    return true;
}

bool UVL_SaveSubsystem::FindEmptyCharacterSlot(const FString& UserId, int32& OutCharacterSlotId)
{
    OutCharacterSlotId = -1;

    if (UserId.IsEmpty())
    {
        return false;
    }

    UVL_AccountSaveGame* AccountSave = LoadOrCreateAccountSave(UserId);
    if (!AccountSave)
    {
        return false;
    }

    for (const FVLCharacterSlotInfo& SlotInfo : AccountSave->CharacterSlots)
    {
        if (!SlotInfo.bOccupied)
        {
            OutCharacterSlotId = SlotInfo.CharacterSlotId;
            return true;
        }
    }

    return false;
}

bool UVL_SaveSubsystem::CreateCharacterSlot(const FString& UserId, const FString& CharacterName, int32& OutCharacterSlotId)
{
    OutCharacterSlotId = -1;

    if (UserId.IsEmpty() || CharacterName.IsEmpty())
    {
        return false;
    }

    UVL_AccountSaveGame* AccountSave = LoadOrCreateAccountSave(UserId);
    if (!AccountSave)
    {
        return false;
    }

    for (FVLCharacterSlotInfo& SlotInfo : AccountSave->CharacterSlots)
    {
        if (!SlotInfo.bOccupied)
        {
            SlotInfo.bOccupied = true;
            SlotInfo.CharacterName = CharacterName;

            OutCharacterSlotId = SlotInfo.CharacterSlotId;

            return UGameplayStatics::SaveGameToSlot(AccountSave, GetAccountSlotName(UserId), 0);
        }
    }

    return false;
}

bool UVL_SaveSubsystem::DeleteCharacterSlot(const FString& UserId, int32 CharacterSlotId)
{
    if (UserId.IsEmpty() || CharacterSlotId < 0)
    {
        return false;
    }

    UVL_AccountSaveGame* AccountSave = LoadOrCreateAccountSave(UserId);
    if (!AccountSave)
    {
        return false;
    }

    bool bFoundSlot = false;

    for (FVLCharacterSlotInfo& SlotInfo : AccountSave->CharacterSlots)
    {
        if (SlotInfo.CharacterSlotId == CharacterSlotId)
        {
            SlotInfo.bOccupied = false;
            SlotInfo.CharacterName = TEXT("");
            bFoundSlot = true;
            break;
        }
    }

    if (!bFoundSlot)
    {
        return false;
    }

    const FString CharacterSlotName = MakeCharacterSlotName(UserId, CharacterSlotId);

    if (UGameplayStatics::DoesSaveGameExist(CharacterSlotName, 0))
    {
        UGameplayStatics::DeleteGameInSlot(CharacterSlotName, 0);
    }

    return UGameplayStatics::SaveGameToSlot(AccountSave, GetAccountSlotName(UserId), 0);
}

FString UVL_SaveSubsystem::GetWorldSlotName(const FString& UserId, int32 CharacterSlotId) const
{
    return FString::Printf(TEXT("World_%s_%d"), *UserId, CharacterSlotId);
}

UVL_WorldSaveGame* UVL_SaveSubsystem::LoadOrCreateWorldSave(const FString& UserId, int32 CharacterSlotId)
{
    if (UserId.IsEmpty() || CharacterSlotId < 0)
    {
        return nullptr;
    }

    const FString SlotName = GetWorldSlotName(UserId, CharacterSlotId);

    if (UGameplayStatics::DoesSaveGameExist(SlotName, 0))
    {
        UVL_WorldSaveGame* LoadedSave =
            Cast<UVL_WorldSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));

        if (!LoadedSave)
        {
            return nullptr;
        }

        if (LoadedSave->WorldSlots.Num() < 100)
        {
            const int32 OldNum = LoadedSave->WorldSlots.Num();

            for (int32 i = OldNum; i < 100; i++)
            {
                FVLWorldSlotInfo NewSlot;
                NewSlot.WorldSlotId = i;
                NewSlot.WorldName = TEXT("");
                NewSlot.MapName = NAME_None;
                NewSlot.bOccupied = false;
                NewSlot.bHostEnabled = false;

                LoadedSave->WorldSlots.Add(NewSlot);
            }

            UGameplayStatics::SaveGameToSlot(LoadedSave, SlotName, 0);
        }

        return LoadedSave;
    }

    UVL_WorldSaveGame* NewSave =
        Cast<UVL_WorldSaveGame>(UGameplayStatics::CreateSaveGameObject(UVL_WorldSaveGame::StaticClass()));

    if (!NewSave)
    {
        return nullptr;
    }

    NewSave->OwnerUserId = UserId;
    NewSave->OwnerCharacterSlotId = CharacterSlotId;
    NewSave->WorldSlots.Empty();

    for (int32 i = 0; i < 100; i++)
    {
        FVLWorldSlotInfo NewSlot;
        NewSlot.WorldSlotId = i;
        NewSlot.WorldName = TEXT("");
        NewSlot.MapName = NAME_None;
        NewSlot.bOccupied = false;
        NewSlot.bHostEnabled = false;

        NewSave->WorldSlots.Add(NewSlot);
    }

    if (!UGameplayStatics::SaveGameToSlot(NewSave, SlotName, 0))
    {
        return nullptr;
    }

    return NewSave;
}

bool UVL_SaveSubsystem::GetWorldSlots(const FString& UserId, int32 CharacterSlotId, TArray<FVLWorldSlotInfo>& OutWorldSlots)
{
    OutWorldSlots.Empty();

    if (UserId.IsEmpty() || CharacterSlotId < 0)
    {
        return false;
    }

    UVL_WorldSaveGame* WorldSave = LoadOrCreateWorldSave(UserId, CharacterSlotId);
    if (!WorldSave)
    {
        return false;
    }

    OutWorldSlots = WorldSave->WorldSlots;
    return true;
}

bool UVL_SaveSubsystem::FindEmptyWorldSlot(const FString& UserId, int32 CharacterSlotId, int32& OutWorldSlotId)
{
    OutWorldSlotId = -1;

    if (UserId.IsEmpty() || CharacterSlotId < 0)
    {
        return false;
    }

    UVL_WorldSaveGame* WorldSave = LoadOrCreateWorldSave(UserId, CharacterSlotId);
    if (!WorldSave)
    {
        return false;
    }

    for (const FVLWorldSlotInfo& SlotInfo : WorldSave->WorldSlots)
    {
        if (!SlotInfo.bOccupied)
        {
            OutWorldSlotId = SlotInfo.WorldSlotId;
            return true;
        }
    }

    return false;
}

bool UVL_SaveSubsystem::CreateWorldSlot(const FString& UserId, int32 CharacterSlotId, const FString& WorldName, const FName& MapName, bool bHostEnabled, int32& OutWorldSlotId)
{
    OutWorldSlotId = -1;

    if (UserId.IsEmpty() || CharacterSlotId < 0 || WorldName.IsEmpty() || MapName.IsNone())
    {
        return false;
    }

    UVL_WorldSaveGame* WorldSave = LoadOrCreateWorldSave(UserId, CharacterSlotId);
    if (!WorldSave)
    {
        return false;
    }

    for (FVLWorldSlotInfo& SlotInfo : WorldSave->WorldSlots)
    {
        if (!SlotInfo.bOccupied)
        {
            SlotInfo.bOccupied = true;
            SlotInfo.WorldName = WorldName;
            SlotInfo.MapName = MapName;
            SlotInfo.bHostEnabled = bHostEnabled;

            OutWorldSlotId = SlotInfo.WorldSlotId;

            return UGameplayStatics::SaveGameToSlot(WorldSave, GetWorldSlotName(UserId, CharacterSlotId), 0);
        }
    }

    return false;
}

bool UVL_SaveSubsystem::DeleteWorldSlot(const FString& UserId, int32 CharacterSlotId, int32 WorldSlotId)
{
    if (UserId.IsEmpty() || CharacterSlotId < 0 || WorldSlotId < 0)
    {
        return false;
    }

    UVL_WorldSaveGame* WorldSave = LoadOrCreateWorldSave(UserId, CharacterSlotId);
    if (!WorldSave)
    {
        return false;
    }

    bool bFoundSlot = false;

    for (FVLWorldSlotInfo& SlotInfo : WorldSave->WorldSlots)
    {
        if (SlotInfo.WorldSlotId == WorldSlotId)
        {
            SlotInfo.bOccupied = false;
            SlotInfo.WorldName = TEXT("");
            SlotInfo.MapName = NAME_None;
            SlotInfo.bHostEnabled = false;
            bFoundSlot = true;
            break;
        }
    }

    if (!bFoundSlot)
    {
        return false;
    }

    return UGameplayStatics::SaveGameToSlot(WorldSave, GetWorldSlotName(UserId, CharacterSlotId), 0);
}

bool UVL_SaveSubsystem::HasAnyOccupiedWorldSlot(const FString& UserId, int32 CharacterSlotId)
{
    if (UserId.IsEmpty() || CharacterSlotId < 0)
    {
        return false;
    }

    UVL_WorldSaveGame* WorldSave = LoadOrCreateWorldSave(UserId, CharacterSlotId);
    if (!WorldSave)
    {
        return false;
    }

    for (const FVLWorldSlotInfo& SlotInfo : WorldSave->WorldSlots)
    {
        if (SlotInfo.bOccupied)
        {
            return true;
        }
    }

    return false;
}

bool UVL_SaveSubsystem::DeleteSelectedWorldForCharacter(const FString& UserId, int32 CharacterSlotId, int32 WorldSlotId)
{
    if (UserId.IsEmpty() || CharacterSlotId < 0 || WorldSlotId <0)
    {
        return false;
    }

    return DeleteWorldSlot(UserId, CharacterSlotId, WorldSlotId);
}
