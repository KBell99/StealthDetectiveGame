// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SettingControls/SelectSettingWidget.h"

#include "ButtonGroupSetting.generated.h"

class UCommonButtonGroupBase;

/**
 * 
 */
UCLASS(abstract)
class AUTOSETTINGSCOMMONUI_API UButtonGroupSetting : public USelectSettingWidget
{
	GENERATED_BODY()

public:
	// Button class to create for each option
	UPROPERTY(EditAnywhere, BlueprintReadWrite, NoClear, Category = "Radio Select")
	TSubclassOf<UCommonButtonBase> ButtonClass;

	UFUNCTION(BlueprintPure, Category = "Radio Select")
	UCommonButtonGroupBase* GetButtonGroup() const
	{
		return ButtonGroup;
	}

protected:
	virtual void NativePreConstruct() override;

	virtual void UpdateOptions_Implementation(const TArray<FSettingOption>& InOptions) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Radio Select")
	void HandleButtonCreation(UCommonButtonBase* NewButton, const FSettingOption& Option);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Radio Select")
	void HandleButtonRemoval(UCommonButtonBase* NewButton);

	virtual void UpdateSelection_Implementation(const FString& Value) override;

	virtual void NativeDestruct() override;

private:
	UPROPERTY()
	TObjectPtr<UCommonButtonGroupBase> ButtonGroup;

	UPROPERTY()
	TMap<UCommonButtonBase*, FString> ButtonValueMap;

	void RemoveAllButtons();
};