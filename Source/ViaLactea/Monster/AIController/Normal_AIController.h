// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

//.Build.cs 파일 내 PublicIncludePaths에 해당 폴더들을 등록해두면, 
// 복잡한 경로 대신 #include "VL_AIControllerBase.h" 만으로도 호출
#include "Base/FrameWork/VL_AIControllerBase.h"

#include "Normal_AIController.generated.h"

/**
 * 
 */
UCLASS()
class VIALACTEA_API ANormal_AIController : public AVL_AIControllerBase
{
	GENERATED_BODY()

};
