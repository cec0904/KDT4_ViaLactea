// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

// 선언 (외부에 존재함을 알림)
DECLARE_LOG_CATEGORY_EXTERN(ViaLactea_Log, Log, All);

#define CUSTOM_LOG(FMT, ...) \
do { \
    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT(FMT), ##__VA_ARGS__)); \
    UE_LOG(ViaLactea_Log, Warning, TEXT(FMT), ##__VA_ARGS__); \
} while (0)

/**
 * 
 */
