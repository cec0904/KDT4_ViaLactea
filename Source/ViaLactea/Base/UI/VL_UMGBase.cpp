
#include "Base/UI/VL_UMGBase.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

void UVL_UMGBase::OpenWidget()
{
    if (!IsInViewport())
    {
        AddToViewport(10);
    }

    SetVisibility(ESlateVisibility::Visible);
}

void UVL_UMGBase::CloseWidget()
{
    SetVisibility(ESlateVisibility::Collapsed);

    // 나중에 애니메이션 넣을 때는 조건부 수정.
    RemoveFromParent();
}

void UVL_UMGBase::PlayUISound(USoundBase* Sound)
{
    if (!Sound)
    {
        return;
    }

    UGameplayStatics::PlaySound2D(this, Sound);
}

void UVL_UMGBase::NativeConstruct()
{
    Super::NativeConstruct();

    // 공통 초기화 로직용.
    // 여기서 자동으로 사운드 재생하지 말 것.
}