#include "Base/Component/VL_CooldownComponent.h"
#include "GameplayTagContainer.h"
#include "Base/Interfaces/VL_CooldownProvider.h"
#include "GameFramework/GameStateBase.h"

#include "Monster/Boss/VL_Boss1.h"


#include "CustomLog/CustomLog.h"

UVL_CooldownComponent::UVL_CooldownComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UVL_CooldownComponent::IsReady(FGameplayTag Tag) const
{
	if (!Tag.IsValid()) return true;


	// 1. 주인 액터로부터 데이터 에셋(인터페이스) 가져오기
	IVL_CooldownProvider* Provider = Cast<IVL_CooldownProvider>(GetOwner());

	if (!Provider)
	{
		// 만약 이 로그가 찍힌다면 Boss1 클래스 선언에 인터페이스가 빠진 것
		CUSTOM_LOG("에러: 소유자(%s)가 IVL_CooldownProvider를 상속받지 않았습니다!", *this->GetName());
		return true;
	}
	// 2. 데이터 에셋에서 정의된 쿨타임 수치 가져오기
	float CooldownDuration = Provider->GetCooldownValue(Tag);


	// 3. 수동 체크 로직으로 넘김
	return IsReadyManual(Tag, CooldownDuration);
}

bool UVL_CooldownComponent::IsReadyManual(FGameplayTag Tag, float Duration) const
{
	if (!CooldownHistory.Contains(Tag)) return true;

	float LastUsedTime = CooldownHistory[Tag];
	return (GetCurrentTime() - LastUsedTime) >= Duration;
}

void UVL_CooldownComponent::StartCooldown(FGameplayTag Tag)
{
	if (!GetOwner()->HasAuthority()) return;

	if (!Tag.IsValid()) return;
	CooldownHistory.Emplace(Tag, GetCurrentTime());

}

void UVL_CooldownComponent::ResetCooldown(FGameplayTag Tag)
{
	if (!GetOwner()->HasAuthority()) return;

	CooldownHistory.Remove(Tag);
}

// 시간 오차를 막기위한 공통시간 호출 GetServerWorldTimeSeconds
float UVL_CooldownComponent::GetCurrentTime() const
{
	if (UWorld* World = GetWorld())
	{
		if (AGameStateBase* GameState = World->GetGameState())
		{
			// 서버와 클라이언트가 동기화된 공통 시간
			return GameState->GetServerWorldTimeSeconds();

		}
		return World->GetTimeSeconds(); // GameState가 아직 없을 때의 예외 처리
	}
	return 0.0f;
}