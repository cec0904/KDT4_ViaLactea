#pragma once

#include "CoreMinimal.h"
#include "VL_WorldSlotInfo.generated.h"

USTRUCT(BlueprintType)
struct FVLWorldSlotInfo
{
	GENERATED_BODY()

public:
	// 월드 슬롯 번호
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	int32 WorldSlotId = -1;

	// UI에 보여줄 월드 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FString WorldName;

	// 어떤 맵으로 입장할지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FName MapName = NAME_None;

	// 이 슬롯 사용 중인지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	bool bOccupied = false;

	// 호스트 체크박스 체크 했는지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	bool bHostEnabled = false;
};