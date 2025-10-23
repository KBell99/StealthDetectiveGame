// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SettingTypes.h"
#include "SettingApplicationStrategy.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FSettingAppliedValueModifiedSignature, const FSettingReference&, Setting);

/**
 * Base class for implementing how settings are applied to the game.
 * 
 * This class uses the Strategy pattern to allow different application methods for different
 * types of settings (console variables, input mappings, custom game logic, etc.).
 * 
 * Inherit from this class and implement OnApply to create custom application strategies.
 */
UCLASS(Abstract, EditInlineNew, Blueprintable)
class AUTOSETTINGS_API USettingApplicationStrategy : public UObject
{
	GENERATED_BODY()

public:
	/** Determine if this strategy can register for the given setting context */
	UFUNCTION(BlueprintNativeEvent)
	bool CanRegister(const FSettingContext& Context);

	/** Get the currently applied value for this setting from the underlying system */
	UFUNCTION(BlueprintNativeEvent)
	FString GetAppliedValue(const FSettingReference& Setting);

	/** Validate that this setting is properly configured and can be applied */
	UFUNCTION(BlueprintNativeEvent)
	bool ValidateConfiguration(const FSettingReference& Setting);

	/** Register a callback to be notified when the applied value changes externally */
	UFUNCTION(BlueprintNativeEvent)
	void RegisterAppliedValueModifiedCallback(const FSettingReference& Setting,
		const FSettingAppliedValueModifiedSignature& Callback);

	/** Remove a previously registered callback for applied value changes */
	UFUNCTION(BlueprintNativeEvent)
	void RemoveAppliedValueModifiedCallback(const UObject* UserObject, const FSettingReference& Setting);

protected:
	friend class USettingType;

	/** Apply a setting value using this strategy - Only USettingType should call this */
	UFUNCTION()
	void Apply(const FSetSettingEvent& Event);

	UFUNCTION(BlueprintNativeEvent)
	void OnApply(const FSetSettingEvent& Event);
};