// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Struct/VL_CharacterSlotInfo.h"
#include "Struct/VL_WorldSlotInfo.h"
#include "VL_WorldSaveGame.h"
#include "VL_SaveSubsystem.generated.h"

/**
* 슬롯 이름 만들기
* 캐릭터 세이브 저장 객체 로드/생성
* 현재 playerstate 저장
* 저장된 playerstate 불러오기
* 
 * 
 */

class AVL_PlayerState;
class UVL_CharacterSaveGame;
class UVL_AccountSaveGame;
class UVL_WorldSaveGame;

UCLASS(BlueprintType)
class VIALACTEA_API UVL_SaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// 캐릭터 저장
	UFUNCTION(BlueprintCallable, Category = "Save")
	FString MakeCharacterSlotName(const FString& UserId, int32 CharacterSlotId) const;

	UFUNCTION(BlueprintCallable, Category = "Save")
	UVL_CharacterSaveGame* LoadOrCreateCharacterSave(const FString& UserId, int32 CharacterSlotId);

	UFUNCTION(BlueprintCallable, Category = "Save")
	bool SavePlayerStateToCharacterSlot(AVL_PlayerState* PlayerState);

	UFUNCTION(BlueprintCallable, Category = "Save")
	bool LoadPlayerStateFromCharacterSlot(AVL_PlayerState* PlayerState, const FString& UserId, int32 CharacterSlotId);


	// 계정 저장
	UFUNCTION(BlueprintCallable, Category = "Save|Account")
	bool GetPlatformUserId(FString& OutUserId) const;

	// Account 세이브 파일 이름을 한 곳에서 관리
	UFUNCTION(BlueprintCallable, Category = "Save|Account")
	FString GetAccountSlotName(const FString& UserId) const;

	// Account Save File 확보
	UFUNCTION(BlueprintCallable, Category = "Save|Account")
	UVL_AccountSaveGame* LoadOrCreateAccountSave(const FString& UserId);

	// 현재 계정 식별값 저장
	UFUNCTION(BlueprintCallable, Category = "Save|Account")
	bool SaveAccountUserId(const FString& UserId);

	// 저장된 계정 식별값 읽기
	UFUNCTION(BlueprintCallable, Category = "Save|Account")
	bool LoadAccountUserId(const FString& UserId, FString& OutUserId);

	// 캐릭터 슬롯 목록
	// 계정의 슬롯 배열 통째로 꺼내기
	UFUNCTION(BlueprintCallable, Category = "Save|Account")
	bool GetCharacterSlots(const FString& UserId, TArray<FVLCharacterSlotInfo>& OutCharacterSlots);

	// 비어있는 슬롯 찾기
	UFUNCTION(BlueprintCallable, Category = "Save|Account")
	bool FindEmptyCharacterSlot(const FString& UserId, int32& OutCharacterSlotId);

	// 빈 슬롯 찾아서 이름 저장, bOccupied = true, CharaterSlotId 반환
	UFUNCTION(BlueprintCallable, Category = "Save|Account")
	bool CreateCharacterSlot(const FString& UserId, const FString& CharacterName, int32& OutCharacterSlotId);

	// 해당 슬롯 비우기, 이름 초기화, save 파일 삭제
	UFUNCTION(BlueprintCallable, Category = "Save|Account")
	bool DeleteCharacterSlot(const FString& UserId, int32 CharacterSlotId);


	//////////////////////////////////////////////////// 월드 저장
	UFUNCTION(BlueprintCallable, Category = "Save|World")
	FString GetWorldSlotName(const FString& UserId, int32 CharacterSlotId) const;

	UFUNCTION(BlueprintCallable, Category = "Save|World")
	UVL_WorldSaveGame* LoadOrCreateWorldSave(const FString& UserId, int32 CharacterSlotId);

	UFUNCTION(BlueprintCallable, Category = "Save|World")
	bool GetWorldSlots(const FString& UserId, int32 CharacterSlotId, TArray<FVLWorldSlotInfo>& OutWorldSlots);

	UFUNCTION(BlueprintCallable, Category = "Save|World")
	bool FindEmptyWorldSlot(const FString& UserId, int32 CharacterSlotId, int32& OutWorldSlotId);

	UFUNCTION(BlueprintCallable, Category = "Save|World")
	bool CreateWorldSlot(const FString& UserId, int32 CharacterSlotId, const FString& WorldName, const FName& MapName, bool bHostEnabled, int32& OutWorldSlotId);

	UFUNCTION(BlueprintCallable, Category = "Save|World")
	bool DeleteWorldSlot(const FString& UserId, int32 CharacterSlotId, int32 WorldSlotId);


	UFUNCTION(BlueprintCallable, Category = "Save|World")
	bool HasAnyOccupiedWorldSlot(const FString& UserId, int32 CharacterSlotId);

	UFUNCTION(BlueprintCallable, Category = "Save|World")
	bool DeleteSelectedWorldForCharacter(const FString& UserId, int32 CharacterSlotId, int32 WorldSlotId);
	
};
