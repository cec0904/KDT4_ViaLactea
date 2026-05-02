// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/UI/VL_PlayerInfoUMG.h"
#include "Player/MainCharacterBase.h"
#include "Base/Component/VL_StatComponent.h"
#include "GameFramework/PlayerController.h"

#include "Components/Image.h"
#include "Materials/MaterialInstanceDynamic.h"

void UVL_PlayerInfoUMG::RebindAndRefresh()
{
	if (CachedStatComponent)
	{
		CachedStatComponent->OnHPChanged.RemoveDynamic(this, &UVL_PlayerInfoUMG::HandleHPChanged);
		CachedStatComponent->OnStaminaChanged.RemoveDynamic(this, &UVL_PlayerInfoUMG::HandleStaminaChanged);
	}

	CachedCharacter = nullptr;
	CachedStatComponent = nullptr;

	BindToStatComponent();
	RefreshAll();
}

void UVL_PlayerInfoUMG::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bStartDelay)
	{
		return;
	}

	UpdateHPDelayBar(InDeltaTime);
}

void UVL_PlayerInfoUMG::NativeConstruct()
{
	Super::NativeConstruct();

	InitializeHPMaterial();
	InitializeHPDelayMaterial();
	InitializeStaminaMaterial();

	RebindAndRefresh();
}

void UVL_PlayerInfoUMG::BindToStatComponent()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		return;
	}

	CachedCharacter = Cast<AMainCharacterBase>(PC->GetPawn());
	if (!CachedCharacter)
	{
		return;
	}

	CachedStatComponent = CachedCharacter->GetStatComponent();
	if (!CachedStatComponent)
	{
		return;
	}

	CachedStatComponent->OnHPChanged.AddDynamic(this, &UVL_PlayerInfoUMG::HandleHPChanged);
	CachedStatComponent->OnStaminaChanged.AddDynamic(this, &UVL_PlayerInfoUMG::HandleStaminaChanged);
}

void UVL_PlayerInfoUMG::HandleHPChanged(float NewHP, float MaxHP)
{
	const float Percent = (MaxHP > KINDA_SMALL_NUMBER) ? (NewHP / MaxHP) : 0.f;

	TargetHPPercent = Percent;

	// 실제 HP 바는 즉시 반영
	if (MID_HPBar)
	{
		MID_HPBar->SetScalarParameterValue(TEXT("Progress"), Percent);
	}

	// 회복일 때는 딜레이 없이 바로 맞춰도 됨
	if (DisplayedDelayHPPercent < Percent)
	{
		DisplayedDelayHPPercent = Percent;

		if (MID_HPDelayBar)
		{
			MID_HPDelayBar->SetScalarParameterValue(TEXT("Progress"), DisplayedDelayHPPercent);
		}

		GetWorld()->GetTimerManager().ClearTimer(HPDelayTimerHandle);
		return;
	}

	// 데미지일 때는 약간 늦게 시작
	GetWorld()->GetTimerManager().ClearTimer(HPDelayTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(
		HPDelayTimerHandle,
		this,
		&UVL_PlayerInfoUMG::StartHPDelayBar,
		HPDelayStartTime,
		false
	);
}

void UVL_PlayerInfoUMG::HandleStaminaChanged(float NewStamina, float MaxStamina)
{
	const float Percent = (MaxStamina > KINDA_SMALL_NUMBER) ? (NewStamina / MaxStamina) : 0.f;

	if (MID_SPBar)
	{
		MID_SPBar->SetScalarParameterValue(TEXT("Progress"), Percent);
	}

	//UE_LOG(LogTemp, Warning, TEXT("SP Percent = %.2f"), Percent);
}

void UVL_PlayerInfoUMG::RefreshAll()
{
	if (!CachedStatComponent)
	{
		return;
	}

	const float HPPercent =
		(CachedStatComponent->GetMaxHP() > KINDA_SMALL_NUMBER)
		? (CachedStatComponent->GetCurrentHP() / CachedStatComponent->GetMaxHP())
		: 0.f;

	TargetHPPercent = HPPercent;
	DisplayedDelayHPPercent = HPPercent;

	if (MID_HPBar)
	{
		MID_HPBar->SetScalarParameterValue(TEXT("Progress"), HPPercent);
	}

	if (MID_HPDelayBar)
	{
		MID_HPDelayBar->SetScalarParameterValue(TEXT("Progress"), HPPercent);
	}

	HandleStaminaChanged(CachedStatComponent->GetCurrentStamina(), CachedStatComponent->GetMaxStamina());
}

void UVL_PlayerInfoUMG::InitializeHPMaterial()
{
	if (!IMG_HPFill || !HPFillMaterial)
	{
		return;
	}

	MID_HPBar = UMaterialInstanceDynamic::Create(HPFillMaterial, this);
	if (!MID_HPBar)
	{
		return;
	}

	IMG_HPFill->SetBrushFromMaterial(MID_HPBar);
	MID_HPBar->SetScalarParameterValue(TEXT("Progress"), 1.0f);
}

void UVL_PlayerInfoUMG::InitializeStaminaMaterial()
{
	UE_LOG(LogTemp, Warning, TEXT("SPFillMaterial = %s"), *GetNameSafe(SPFillMaterial));

	if (!IMG_SPFill || !SPFillMaterial)
	{
		return;
	}

	MID_SPBar = UMaterialInstanceDynamic::Create(SPFillMaterial, this);

	if (!MID_SPBar)
	{
		return;
	}

	IMG_SPFill->SetBrushFromMaterial(MID_SPBar);
	MID_SPBar->SetScalarParameterValue(TEXT("Progress"), 1.0f);
}

void UVL_PlayerInfoUMG::InitializeHPDelayMaterial()
{
	if (!IMG_HPDelayFill || !HPDelayFillMaterial)
	{
		return;
	}

	MID_HPDelayBar = UMaterialInstanceDynamic::Create(HPDelayFillMaterial, this);
	if (!MID_HPDelayBar)
	{
		return;
	}

	IMG_HPDelayFill->SetBrushFromMaterial(MID_HPDelayBar);
	MID_HPDelayBar->SetScalarParameterValue(TEXT("Progress"), 1.0f);
}

void UVL_PlayerInfoUMG::StartHPDelayBar()
{
	bStartDelay = true;
}

void UVL_PlayerInfoUMG::UpdateHPDelayBar(float DeltaTime)
{
	if (!MID_HPDelayBar)
	{
		return;
	}

	if (DisplayedDelayHPPercent <= TargetHPPercent + KINDA_SMALL_NUMBER)
	{
		DisplayedDelayHPPercent = TargetHPPercent;
		MID_HPDelayBar->SetScalarParameterValue(TEXT("Progress"), DisplayedDelayHPPercent);

		bStartDelay = false;
		return;
	}

	DisplayedDelayHPPercent = FMath::FInterpTo(
		DisplayedDelayHPPercent,
		TargetHPPercent,
		DeltaTime,
		HPDelayLerpSpeed
	);

	MID_HPDelayBar->SetScalarParameterValue(TEXT("Progress"), DisplayedDelayHPPercent);
}
