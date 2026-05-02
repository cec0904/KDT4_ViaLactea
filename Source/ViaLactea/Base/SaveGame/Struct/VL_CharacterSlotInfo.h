#pragma once

#include "CoreMinimal.h"
#include "VL_CharacterSlotInfo.generated.h"

USTRUCT(BlueprintType)
struct FVLCharacterSlotInfo
{
	GENERATED_BODY()

public:
	// 슬롯 번호 0 1 2 3 4 
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, SaveGame)
	int32 CharacterSlotId = -1;

	// startmenu 에서 보여줄 이름
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, SaveGame)
	FString CharacterName;

	// 빈 슬롯, 사용중인 슬롯 판정
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, SaveGame)
	bool bOccupied = false;

};