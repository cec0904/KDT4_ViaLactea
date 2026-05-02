// Fill out your copyright notice in the Description page of Project Settings.


#include "ViaLactea_BFLibrary.h"

#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"

bool UViaLactea_BFLibrary::ClientTravelToGameSession(UObject* WorldContextObject, APlayerController* PlayerController, bool bAbsolute)
{

	if (!WorldContextObject || !PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("ClientTravelToGameSession: invalid params"));
		return false;
	}

	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (!Subsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("ClientTravelToGameSession: no online subsystem"));
		return false;
	}

	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("ClientTravelToGameSession: invalid session interface"));
		return false;
	}

	// 기본 세션 이름은 엔진 기본인 NAME_GameSession (="GameSession")
	FString ConnectString;
	const bool bGot = SessionInterface->GetResolvedConnectString(NAME_GameSession, ConnectString);

	if (!bGot || ConnectString.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("ClientTravelToGameSession: failed to get connect string"));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("ClientTravelToGameSession: %s"), *ConnectString);

	PlayerController->ClientTravel(
		ConnectString,
		bAbsolute ? ETravelType::TRAVEL_Absolute : ETravelType::TRAVEL_Relative
	);

	return true;
}
