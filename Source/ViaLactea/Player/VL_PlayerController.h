// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/FrameWork/VL_PlayerControllerBase.h"
#include "InputActionValue.h"
#include "Base/Data/EVLSlotType.h"
#include "VL_PlayerController.generated.h"

/**
 *
 */

class UVL_PlayerQuickSlot; // 퀵슬롯클래스
class UVL_PlayerBaseALLWidget;
class UVL_PlayerCrosshairWidget;
class UInputAction;
class UInputMappingContext;
class UVL_UMGBase;
class UVL_HeldItemWidget;
class USoundBase;
class AVL_Boss1;
class UVL_BossInfoUMG;
class UUserWidget;

UCLASS()
class VIALACTEA_API AVL_PlayerController : public AVL_PlayerControllerBase
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;


protected:
    // 현재 선택 슬롯
    UPROPERTY(BlueprintReadOnly, Category = "QuickSlot")
    int32 CurrentSlotIndex = 0;

    void SetSlotIndex(int32 NewIndex);

protected:
    // 인벤토리 열기/닫기
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UVL_UMGBase> InventoryWidgetClass;

    UPROPERTY()
    UVL_UMGBase* InventoryWidget = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
    bool bInventoryOpen = false;

    UPROPERTY(EditDefaultsOnly, Category = "Sound|UI")
    TObjectPtr<USoundBase> InventoryOpenSound;

    //인벤토리에서 현재 선택된 슬롯 칸
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    EVLSlotType SelectedSlotType = EVLSlotType::None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    int32 SelectedSlotIndex = INDEX_NONE;

public:
    UFUNCTION(BlueprintCallable)
    void OpenInventory();

    UFUNCTION(BlueprintCallable)
    void CloseInventory();

    UFUNCTION(BlueprintCallable)
    void ToggleInventory();
    //선택 슬롯 설정 함수
    UFUNCTION(BlueprintCallable)
    void SetSelectedInventorySlot(EVLSlotType InSlotType, int32 InSlotIndex);

public:
    UFUNCTION(Client, Reliable)
    void ClientRefreshPlayerInfoUI();

    UFUNCTION(BlueprintCallable)
    void RefreshPlayerInfoUI();

protected:
    // Enhanced Input 에셋 지정용 (에디터에넣어줘야함)
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    UInputMappingContext* IMC_Main = nullptr;
    //인벤토리 입력매핑
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    UInputMappingContext* IMC_Inventory = nullptr;
    //인벤토리 닫기
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    UInputAction* IA_CloseInventory = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    UInputAction* IA_ToggleInventory = nullptr;
    //인벤토리에서 아이템 사용
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    UInputAction* IA_InventoryUse = nullptr;
    //인벤토리에서 아이템 분할
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    UInputAction* IA_InventorySplit = nullptr;
    //인벤토리에서 아이템 버리기
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    UInputAction* IA_InventoryDrop = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    UInputAction* IA_QuickSlotSelect1 = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    UInputAction* IA_QuickSlotSelect2 = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    UInputAction* IA_QuickSlotSelect3 = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    UInputAction* IA_QuickSlotSelect4 = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    UInputAction* IA_QuickSlotSelect5 = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    UInputAction* IA_QuickSlotSelect6 = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    UInputAction* IA_QuickSlotSelect7 = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    UInputAction* IA_QuickSlotSelect8 = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    UInputAction* IA_QuickSlotSelect9 = nullptr;

    //입력 핸들러
    void OnQuickSlot1(const FInputActionValue& Value);
    void OnQuickSlot2(const FInputActionValue& Value);
    void OnQuickSlot3(const FInputActionValue& Value);
    void OnQuickSlot4(const FInputActionValue& Value);
    void OnQuickSlot5(const FInputActionValue& Value);
    void OnQuickSlot6(const FInputActionValue& Value);
    void OnQuickSlot7(const FInputActionValue& Value);
    void OnQuickSlot8(const FInputActionValue& Value);
    void OnQuickSlot9(const FInputActionValue& Value);

protected:
    void OnInventoryUse(const FInputActionValue& Value);

    UFUNCTION(Server, Reliable)
    void ServerUseInventoryItem(EVLSlotType InSlotType, int32 InSlotIndex);

public:
    UFUNCTION(Client, Reliable)
    void ClientPlayDrinkPotionSound(USoundBase* Sound);

    UFUNCTION(Server, Reliable)
    void ServerMoveOrSwapSlot(EVLSlotType FromType, int32 FromIndex, EVLSlotType ToType, int32 ToIndex);

protected:
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UVL_PlayerBaseALLWidget> PlayerBaseALLWidgetClass;

    UPROPERTY()
    UVL_PlayerBaseALLWidget* PlayerBaseALLWidget = nullptr;

    UPROPERTY()
    UVL_PlayerCrosshairWidget* PlayerCrosshairWidget = nullptr;

    UPROPERTY()
    UVL_PlayerQuickSlot* PlayerBaseALLQuickSlotWidget = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UVL_HeldItemWidget> HeldItemWidgetClass;

    UPROPERTY()
    UVL_HeldItemWidget* HeldItemWidget = nullptr;

    void TryBindPlayerBaseALLWidgets();

    void TryBindHeldItemWidget();

protected:
    UPROPERTY(EditDefaultsOnly, Category = "UI|Boss")
    TSubclassOf<UVL_BossInfoUMG> BossInfoWidgetClass;

    UPROPERTY()
    UVL_BossInfoUMG* BossInfoWidget = nullptr;

    UPROPERTY()
    AVL_Boss1* CurrentBoss = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "UI|Boss")
    float BossHPDetectRange = 3000.f;

    void UpdateBossHPWidget();

    void BindBossHP(AVL_Boss1* NewBoss);

    void UnbindBossHP();

    UFUNCTION()
    void OnBossHPChanged(float NewHP, float MaxHP);


public:
    virtual void Tick(float DeltaTime) override;


public:
    UFUNCTION(Client, Reliable, BlueprintCallable, Category = "Loading")
    void Client_BeginLoadingTransition();

    UFUNCTION(Client, Reliable, BlueprintCallable, Category = "Loading")
    void Client_EndLoadingTransition();


protected:
    UPROPERTY(EditDefaultsOnly, Category = "UI|Loading")
    TSubclassOf<UUserWidget> LoadingWidgetClass;

    UPROPERTY()
    UUserWidget* LoadingWidget = nullptr;

};
