// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/UI/VL_BossInfoUMG.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Materials/MaterialInstanceDynamic.h"

void UVL_BossInfoUMG::NativeConstruct()
{
	Super::NativeConstruct();

	if (IMG_BossHPBar)
	{
		BossHPMID = IMG_BossHPBar->GetDynamicMaterial();
	}
}

void UVL_BossInfoUMG::UpdateBossHP(float CurrentHP, float MaxHP)
{
	if (!BossHPMID || MaxHP <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	const float Percent = FMath::Clamp(CurrentHP / MaxHP, 0.f, 1.f);

	BossHPMID->SetScalarParameterValue(TEXT("Percent"), Percent);
}

void UVL_BossInfoUMG::SetBossName(const FText& Name)
{
	if (TXT_BossName)
	{
		TXT_BossName->SetText(Name);
	}
}
