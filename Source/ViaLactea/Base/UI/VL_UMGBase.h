#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VL_UMGBase.generated.h"

class USoundBase;

UCLASS()
class VIALACTEA_API UVL_UMGBase : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "UI")
	virtual void OpenWidget();

	UFUNCTION(BlueprintCallable, Category = "UI")
	virtual void CloseWidget();

	UFUNCTION(BlueprintCallable, Category = "UI|Sound")
	void PlayUISound(USoundBase* Sound);

public:
	virtual void NativeConstruct() override;
};