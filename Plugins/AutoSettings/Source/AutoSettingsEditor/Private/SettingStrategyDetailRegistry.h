// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "StrategyUIHandlers/ISettingStrategyDetailCustomizer.h"

class IDetailCategoryBuilder;
class IPropertyHandle;
class USettingApplicationStrategy;
class USettingType;

/**
 * Registry for setting strategy detail customizers that manages and dispatches to appropriate customizers
 * based on strategy type
 */
class FSettingStrategyDetailRegistry
{
public:
	/**
	 * Get the singleton instance of the registry
	 */
	static FSettingStrategyDetailRegistry& Get();
	
	/**
	 * Register a detail customizer for strategies
	 * @param Customizer The customizer to register
	 */
	void RegisterCustomizer(TSharedPtr<ISettingStrategyDetailCustomizer> Customizer);
	
	/**
	 * Add UI fields for the given strategy by finding and delegating to the appropriate handler
	 * @param CategoryBuilder The category builder for adding strategy-specific fields
	 * @param Strategy The strategy to add UI for
	 * @param SubKeyHandle Handle to the SubKey property
	 * @param SettingType The setting type containing the strategy
	 */
	void AddFieldsForStrategy(
		IDetailCategoryBuilder& CategoryBuilder,
		const USettingApplicationStrategy* Strategy,
		TSharedRef<IPropertyHandle> SubKeyHandle,
		const USettingType* SettingType) const;
	
private:
	/** Registered detail customizers */
	TArray<TSharedPtr<ISettingStrategyDetailCustomizer>> Customizers;
};