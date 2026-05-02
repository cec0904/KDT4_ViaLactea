// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/VL_PlayerController.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Math/UnrealMathUtility.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

#include "Player/UI/VL_PlayerQuickSlot.h"
#include "Player/UI/VL_HeldItemWidget.h"
#include "Player/MainCharacterBase.h"
#include "Player/VL_PlayerState.h"

#include "Base/UI/VL_UMGBase.h"
#include "Base/Component/VL_InventoryComponent.h"
#include "Player/UI/VL_PlayerBaseALLWidget.h"
#include "Player/UI/VL_PlayerCrosshairWidget.h"
#include "Player/UI/VL_PlayerInfoUMG.h"
#include "Weapon/BowBase.h"

#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

#include "Monster/Boss/VL_Boss1.h"
#include "Monster/UI/VL_BossInfoUMG.h"

#include "CustomLog/CustomLog.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"

void AVL_PlayerController::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("AVL_PlayerController BeginPlay OK"));
    bInventoryOpen = false;
    UE_LOG(LogTemp, Warning, TEXT("BeginPlay bInventoryOpen = %d"), bInventoryOpen);

    if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
            LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
        {
            Subsystem->AddMappingContext(IMC_Main, 0);
        }
    }

    if (IsLocalController())
    {
        if (PlayerBaseALLWidgetClass && !PlayerBaseALLWidget)
        {
            PlayerBaseALLWidget = CreateWidget<UVL_PlayerBaseALLWidget>(this, PlayerBaseALLWidgetClass);
            if (PlayerBaseALLWidget)
            {
                PlayerBaseALLWidget->AddToViewport();
            }
        }

        if (BossInfoWidgetClass && !BossInfoWidget)
        {
            BossInfoWidget = CreateWidget<UVL_BossInfoUMG>(this, BossInfoWidgetClass);
            if (BossInfoWidget)
            {
                BossInfoWidget->AddToViewport(50);
                BossInfoWidget->SetVisibility(ESlateVisibility::Collapsed);
            }
        }

        if (HeldItemWidgetClass && !HeldItemWidget)
        {
            HeldItemWidget = CreateWidget<UVL_HeldItemWidget>(this, HeldItemWidgetClass);
            if (HeldItemWidget)
            {
                HeldItemWidget->AddToViewport(999);
                HeldItemWidget->SetVisibility(ESlateVisibility::Collapsed);
            }
        }

        TryBindPlayerBaseALLWidgets();

        AVL_PlayerState* PS = GetPlayerState<AVL_PlayerState>();
        if (PS && HeldItemWidget)
        {
            if (UVL_InventoryComponent* InvComp = PS->GetInventoryComponent())
            {
                HeldItemWidget->SetInventoryComponent(InvComp);
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("IMC added: %s"), IMC_Main ? TEXT("YES") : TEXT("NO"));
}

void AVL_PlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
    {
        if (IA_ToggleInventory)
        {
            EIC->BindAction(IA_ToggleInventory, ETriggerEvent::Started, this, &AVL_PlayerController::ToggleInventory);
        }

        if (IA_CloseInventory)
        {
            EIC->BindAction(IA_CloseInventory, ETriggerEvent::Started, this, &AVL_PlayerController::CloseInventory);
        }

        if (IA_InventoryUse)
        {
            EIC->BindAction(IA_InventoryUse, ETriggerEvent::Started, this, &AVL_PlayerController::OnInventoryUse);
        }

        EIC->BindAction(IA_QuickSlotSelect1, ETriggerEvent::Started, this, &AVL_PlayerController::OnQuickSlot1);
        EIC->BindAction(IA_QuickSlotSelect2, ETriggerEvent::Started, this, &AVL_PlayerController::OnQuickSlot2);
        EIC->BindAction(IA_QuickSlotSelect3, ETriggerEvent::Started, this, &AVL_PlayerController::OnQuickSlot3);
        EIC->BindAction(IA_QuickSlotSelect4, ETriggerEvent::Started, this, &AVL_PlayerController::OnQuickSlot4);
        EIC->BindAction(IA_QuickSlotSelect5, ETriggerEvent::Started, this, &AVL_PlayerController::OnQuickSlot5);
        EIC->BindAction(IA_QuickSlotSelect6, ETriggerEvent::Started, this, &AVL_PlayerController::OnQuickSlot6);
        EIC->BindAction(IA_QuickSlotSelect7, ETriggerEvent::Started, this, &AVL_PlayerController::OnQuickSlot7);
        EIC->BindAction(IA_QuickSlotSelect8, ETriggerEvent::Started, this, &AVL_PlayerController::OnQuickSlot8);
        EIC->BindAction(IA_QuickSlotSelect9, ETriggerEvent::Started, this, &AVL_PlayerController::OnQuickSlot9);
    }
}

void AVL_PlayerController::TryBindPlayerBaseALLWidgets()
{
    if (!IsLocalController())
    {
        return;
    }

    if (!PlayerBaseALLWidget)
    {
        return;
    }

    if (!PlayerCrosshairWidget)
    {
        PlayerCrosshairWidget = PlayerBaseALLWidget->GetPlayerCrosshairWidget();

        UE_LOG(LogTemp, Warning, TEXT("TryBindPlayerBaseALLWidgets | Crosshair = %s"),
            PlayerCrosshairWidget ? TEXT("Valid") : TEXT("Null"));
    }

    if (!PlayerBaseALLQuickSlotWidget)
    {
        PlayerBaseALLQuickSlotWidget = PlayerBaseALLWidget->GetPlayerQuickSlotWidget();

        UE_LOG(LogTemp, Warning, TEXT("TryBindPlayerBaseALLWidgets | QuickSlot = %s"),
            PlayerBaseALLQuickSlotWidget ? TEXT("Valid") : TEXT("Null"));
    }

    if (!PlayerBaseALLQuickSlotWidget)
    {
        return;
    }

    if (PlayerBaseALLQuickSlotWidget->InventoryComponent)
    {
        return;
    }

    AVL_PlayerState* PS = GetPlayerState<AVL_PlayerState>();
    if (!PS)
    {
        UE_LOG(LogTemp, Warning, TEXT("TryBindPlayerBaseALLWidgets | PlayerState = Null"));
        return;
    }

    UVL_InventoryComponent* InvComp = PS->GetInventoryComponent();
    if (!InvComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("TryBindPlayerBaseALLWidgets | InventoryComponent = Null"));
        return;
    }

    PlayerBaseALLQuickSlotWidget->SetInventoryComponent(InvComp);
    PlayerBaseALLQuickSlotWidget->SelectSlot(CurrentSlotIndex);

    UE_LOG(LogTemp, Warning, TEXT("TryBindPlayerBaseALLWidgets | QuickSlot Bind COMPLETE"));
}

void AVL_PlayerController::TryBindHeldItemWidget()
{
    if (!IsLocalController())
    {
        return;
    }

    if (!HeldItemWidget)
    {
        return;
    }

    AVL_PlayerState* PS = GetPlayerState<AVL_PlayerState>();
    if (!PS)
    {
        UE_LOG(LogTemp, Warning, TEXT("TryBindHeldItemWidget | PlayerState = Null"));
        return;
    }

    UVL_InventoryComponent* InvComp = PS->GetInventoryComponent();
    if (!InvComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("TryBindHeldItemWidget | InventoryComponent = Null"));
        return;
    }

    HeldItemWidget->SetInventoryComponent(InvComp);
}

void AVL_PlayerController::UpdateBossHPWidget()
{
    if (!IsLocalController())
    {
        return;
    }

    APawn* MyPawn = GetPawn();
    if (!MyPawn)
    {
        UnbindBossHP();
        return;
    }

    TArray<AActor*> FoundBosses;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AVL_Boss1::StaticClass(), FoundBosses);

    AVL_Boss1* NearestBoss = nullptr;
    float NearestDistSq = FMath::Square(BossHPDetectRange);

    for (AActor* Actor : FoundBosses)
    {
        AVL_Boss1* Boss = Cast<AVL_Boss1>(Actor);
        if (!Boss)
        {
            continue;
        }

        const float DistSq = FVector::DistSquared(MyPawn->GetActorLocation(), Boss->GetActorLocation());

        if (DistSq <= NearestDistSq)
        {
            NearestDistSq = DistSq;
            NearestBoss = Boss;
        }
    }

    if (NearestBoss)
    {
        if (CurrentBoss != NearestBoss)
        {
            BindBossHP(NearestBoss);
        }

        if (BossInfoWidget)
        {
            BossInfoWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
            BossInfoWidget->UpdateBossHP(NearestBoss->GetCurrentHP(), NearestBoss->GetMaxHP());
        }
    }
    else
    {
        UnbindBossHP();
    }
}

void AVL_PlayerController::BindBossHP(AVL_Boss1* NewBoss)
{
    UnbindBossHP();

    CurrentBoss = NewBoss;

    if (CurrentBoss)
    {
        CurrentBoss->OnHPChanged.AddDynamic(this, &AVL_PlayerController::OnBossHPChanged);
    }

    if (BossInfoWidget && CurrentBoss)
    {
        BossInfoWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
        BossInfoWidget->UpdateBossHP(CurrentBoss->GetCurrentHP(), CurrentBoss->GetMaxHP());
        BossInfoWidget->SetBossName(CurrentBoss->GetBossName());
    }
}

void AVL_PlayerController::UnbindBossHP()
{
    if (CurrentBoss)
    {
        CurrentBoss->OnHPChanged.RemoveDynamic(this, &AVL_PlayerController::OnBossHPChanged);
        CurrentBoss = nullptr;
    }

    if (BossInfoWidget)
    {
        BossInfoWidget->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void AVL_PlayerController::OnBossHPChanged(float NewHP, float MaxHP)
{
    if (BossInfoWidget)
    {
        BossInfoWidget->UpdateBossHP(NewHP, MaxHP);

        if (NewHP <= 0.f)
        {
            BossInfoWidget->SetVisibility(ESlateVisibility::Collapsed);
        }
    }
}

void AVL_PlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TryBindPlayerBaseALLWidgets();
    TryBindHeldItemWidget();
    UpdateBossHPWidget();

    AMainCharacterBase* MyCharacter = Cast<AMainCharacterBase>(GetPawn());
    if (!MyCharacter || !PlayerCrosshairWidget)
    {
        return;
    }

    PlayerCrosshairWidget->SetAimState(MyCharacter->IsAiming());

    float ChargeAlpha = 0.f;
    bool bShowRing = false;

    if (ABowBase* Bow = MyCharacter->GetEquippedBow())
    {
        ChargeAlpha = Bow->GetCrosshairChargeAlpha();
        bShowRing = Bow->ShouldShowCrosshairChargeRing();
    }

    PlayerCrosshairWidget->SetChargeAlpha(ChargeAlpha);
    PlayerCrosshairWidget->SetChargeRingVisible(bShowRing);
}

void AVL_PlayerController::Client_BeginLoadingTransition_Implementation()
{
    if (!IsLocalController())
    {
        return;
    }

    SetIgnoreMoveInput(true);
    SetIgnoreLookInput(true);

    bShowMouseCursor = false;
    SetInputMode(FInputModeGameOnly());

    if (LoadingWidget)
    {
        LoadingWidget->RemoveFromParent();
        LoadingWidget = nullptr;
    }

    if (LoadingWidgetClass)
    {
        LoadingWidget = CreateWidget<UUserWidget>(this, LoadingWidgetClass);

        if (LoadingWidget)
        {
            LoadingWidget->AddToViewport(10000);
        }
    }
    CUSTOM_LOG("Client_BeginLoadingTransition");
}

void AVL_PlayerController::Client_EndLoadingTransition_Implementation()
{
    if (!IsLocalController())
    {
        return;
    }

    if (LoadingWidget)
    {
        LoadingWidget->RemoveFromParent();
        LoadingWidget = nullptr;
    }

    SetIgnoreMoveInput(false);
    SetIgnoreLookInput(false);

    bShowMouseCursor = false;
    SetInputMode(FInputModeGameOnly());
    CUSTOM_LOG("Client_EndLoadingTransition");
}



void AVL_PlayerController::OnQuickSlot1(const FInputActionValue& Value) { UE_LOG(LogTemp, Warning, TEXT("QuickSlot 1 pressed"));  SetSlotIndex(0); }
void AVL_PlayerController::OnQuickSlot2(const FInputActionValue& Value) { UE_LOG(LogTemp, Warning, TEXT("QuickSlot 2 pressed")); SetSlotIndex(1); }
void AVL_PlayerController::OnQuickSlot3(const FInputActionValue& Value) { SetSlotIndex(2); }
void AVL_PlayerController::OnQuickSlot4(const FInputActionValue& Value) { SetSlotIndex(3); }
void AVL_PlayerController::OnQuickSlot5(const FInputActionValue& Value) { SetSlotIndex(4); }
void AVL_PlayerController::OnQuickSlot6(const FInputActionValue& Value) { SetSlotIndex(5); }
void AVL_PlayerController::OnQuickSlot7(const FInputActionValue& Value) { SetSlotIndex(6); }
void AVL_PlayerController::OnQuickSlot8(const FInputActionValue& Value) { SetSlotIndex(7); }
void AVL_PlayerController::OnQuickSlot9(const FInputActionValue& Value) { SetSlotIndex(8); }

void AVL_PlayerController::OnInventoryUse(const FInputActionValue& Value)
{
    UE_LOG(LogTemp, Warning, TEXT("OnInventoryUse Called | Type=%d Index=%d"),
        (int32)SelectedSlotType, SelectedSlotIndex);

    if (SelectedSlotType == EVLSlotType::None || SelectedSlotIndex == INDEX_NONE)
    {
        UE_LOG(LogTemp, Warning, TEXT("OnInventoryUse Failed: No selected slot."));
        return;
    }

    AVL_PlayerState* PS = GetPlayerState<AVL_PlayerState>();
    if (!PS)
    {
        UE_LOG(LogTemp, Warning, TEXT("OnInventoryUse Failed: PlayerState is null."));
        return;
    }

    UVL_InventoryComponent* InvComp = PS->GetInventoryComponent();
    if (!InvComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("OnInventoryUse Failed: InventoryComponent is null."));
        return;
    }

    ServerUseInventoryItem(SelectedSlotType, SelectedSlotIndex);

    UE_LOG(LogTemp, Warning, TEXT("OnInventoryUse Request Sent | Type=%d Index=%d"),
        (int32)SelectedSlotType, SelectedSlotIndex);
}

void AVL_PlayerController::ServerMoveOrSwapSlot_Implementation(EVLSlotType FromType, int32 FromIndex, EVLSlotType ToType, int32 ToIndex)
{
    AVL_PlayerState* PS = GetPlayerState<AVL_PlayerState>();
    if (!PS)
    {
        return;
    }

    UVL_InventoryComponent* InvComp = PS->GetInventoryComponent();
    if (!InvComp)
    {
        return;
    }

    InvComp->MoveOrSwapSlot(FromType, FromIndex, ToType, ToIndex);
}

void AVL_PlayerController::ClientPlayDrinkPotionSound_Implementation(USoundBase* Sound)
{
    if (!Sound)
    {
        return;
    }

    UGameplayStatics::PlaySound2D(this, Sound);
}

void AVL_PlayerController::ServerUseInventoryItem_Implementation(EVLSlotType InSlotType, int32 InSlotIndex)
{
    AVL_PlayerState* PS = GetPlayerState<AVL_PlayerState>();
    if (!PS)
    {
        return;
    }

    UVL_InventoryComponent* InvComp = PS->GetInventoryComponent();
    if (!InvComp)
    {
        return;
    }

    InvComp->UseItem(InSlotType, InSlotIndex);
}


void AVL_PlayerController::SetSlotIndex(int32 NewIndex)
{
    CurrentSlotIndex = FMath::Clamp(NewIndex, 0, 8);

    if (PlayerBaseALLQuickSlotWidget)
    {
        PlayerBaseALLQuickSlotWidget->SelectSlot(CurrentSlotIndex);
    }
    else
    {
        TryBindPlayerBaseALLWidgets();

        if (PlayerBaseALLQuickSlotWidget)
        {
            PlayerBaseALLQuickSlotWidget->SelectSlot(CurrentSlotIndex);
        }
    }

    AVL_PlayerState* PS = GetPlayerState<AVL_PlayerState>();
    if (PS)
    {
        if (UVL_InventoryComponent* InvComp = PS->GetInventoryComponent())
        {
            InvComp->UseQuickSlot(CurrentSlotIndex);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("현재 퀵슬롯 인덱스 = %d"), CurrentSlotIndex);
}

void AVL_PlayerController::OpenInventory()
{
    UE_LOG(LogTemp, Warning, TEXT("OpenInventory Called"));
    if (bInventoryOpen)
    {
        return;
    }

    if (InventoryOpenSound)
    {
        UGameplayStatics::PlaySound2D(this, InventoryOpenSound);
    }

    if (!InventoryWidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("InventoryWidgetClass is null"));
        return;
    }

    if (!InventoryWidget)
    {
        InventoryWidget = CreateWidget<UVL_UMGBase>(this, InventoryWidgetClass);
    }

    if (!InventoryWidget)
    {
        UE_LOG(LogTemp, Warning, TEXT("InventoryWidget Create Failed"));
        return;
    }

    AMainCharacterBase* MyCharacter = Cast<AMainCharacterBase>(GetPawn());

    if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
            LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
        {
            if (IMC_Main)
            {
                Subsystem->RemoveMappingContext(IMC_Main);
                UE_LOG(LogTemp, Warning, TEXT("IMC_Main Removed"));
            }

            if (IMC_Inventory)
            {
                Subsystem->AddMappingContext(IMC_Inventory, 10);
                UE_LOG(LogTemp, Warning, TEXT("IMC_Inventory Added"));
            }
        }
    }

    SetIgnoreLookInput(true);
    SetIgnoreMoveInput(true);

    InventoryWidget->OpenWidget();
    bInventoryOpen = true;

    bShowMouseCursor = true;

    FInputModeGameAndUI InputMode;
    InputMode.SetWidgetToFocus(InventoryWidget->TakeWidget());
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    SetInputMode(InputMode);
    UE_LOG(LogTemp, Warning, TEXT("Inventory Open Complete"));
}

void AVL_PlayerController::CloseInventory()
{
    UE_LOG(LogTemp, Warning, TEXT("CloseInventory Called"));

    if (!bInventoryOpen)
    {
        return;
    }

    AMainCharacterBase* MyCharacter = Cast<AMainCharacterBase>(GetPawn());

    if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
            LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
        {
            if (IMC_Inventory)
            {
                Subsystem->RemoveMappingContext(IMC_Inventory);
                UE_LOG(LogTemp, Warning, TEXT("IMC_Inventory Removed"));
            }

            if (IMC_Main)
            {
                Subsystem->AddMappingContext(IMC_Main, 0);
                UE_LOG(LogTemp, Warning, TEXT("IMC_Main Added Again"));
            }
        }
    }

    SetIgnoreLookInput(false);
    SetIgnoreMoveInput(false);

    if (InventoryWidget)
    {
        InventoryWidget->CloseWidget();
    }

    bInventoryOpen = false;

    bShowMouseCursor = false;
    SetInputMode(FInputModeGameOnly());
    UE_LOG(LogTemp, Warning, TEXT("Inventory Close Complete"));

    AVL_PlayerState* PS = GetPlayerState<AVL_PlayerState>();
    if (PS)
    {
        if (UVL_InventoryComponent* InvComp = PS->GetInventoryComponent())
        {
            if (InvComp->HasHeldItem())
            {
                InvComp->ClearHeldItem();
                UE_LOG(LogTemp, Warning, TEXT("CloseInventory -> HeldItem Cleared"));
            }
        }
    }
}

void AVL_PlayerController::ToggleInventory()
{
    UE_LOG(LogTemp, Warning, TEXT("ToggleInventory Called"));

    if (bInventoryOpen)
    {
        CloseInventory();
    }
    else
    {
        OpenInventory();
    }
}

void AVL_PlayerController::SetSelectedInventorySlot(EVLSlotType InSlotType, int32 InSlotIndex)
{
    SelectedSlotType = InSlotType;
    SelectedSlotIndex = InSlotIndex;

    UE_LOG(LogTemp, Warning, TEXT("Selected Slot Updated | Type=%d Index=%d"),
        (int32)SelectedSlotType, SelectedSlotIndex);
}

void AVL_PlayerController::RefreshPlayerInfoUI()
{
    UE_LOG(LogTemp, Warning, TEXT("[PlayerInfoUI] Refresh Called | Local=%d Pawn=%s"),
        IsLocalController(),
        *GetNameSafe(GetPawn()));

    if (!IsLocalController()) return;
    if (!PlayerBaseALLWidget) return;

    UVL_PlayerInfoUMG* PlayerInfoWidget = PlayerBaseALLWidget->GetPlayerInfoWidget();
    if (!PlayerInfoWidget) return;

    PlayerInfoWidget->RebindAndRefresh();
}

void AVL_PlayerController::ClientRefreshPlayerInfoUI_Implementation()
{
    GetWorldTimerManager().SetTimerForNextTick(
        this,
        &AVL_PlayerController::RefreshPlayerInfoUI
    );
}
