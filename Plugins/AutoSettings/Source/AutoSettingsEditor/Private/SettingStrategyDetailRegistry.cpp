// Copyright Sam Bonifacio. All Rights Reserved.

#include "SettingStrategyDetailRegistry.h"
#include "Framework/SettingType.h"

FSettingStrategyDetailRegistry& FSettingStrategyDetailRegistry::Get()
{
	static FSettingStrategyDetailRegistry Instance;
	return Instance;
}

void FSettingStrategyDetailRegistry::RegisterCustomizer(TSharedPtr<ISettingStrategyDetailCustomizer> Customizer)
{
	if (Customizer.IsValid())
	{
		Customizers.Add(Customizer);
	}
}

void FSettingStrategyDetailRegistry::AddFieldsForStrategy(
	IDetailCategoryBuilder& CategoryBuilder,
	const USettingApplicationStrategy* Strategy,
	TSharedRef<IPropertyHandle> SubKeyHandle,
	const USettingType* SettingType) const
{
	if (!Strategy)
	{
		return;
	}
	
	// Find the first customizer that can handle this strategy
	for (const auto& Customizer : Customizers)
	{
		if (Customizer.IsValid() && Customizer->CanHandle(Strategy))
		{
			Customizer->AddFields(CategoryBuilder, Strategy, SubKeyHandle, SettingType);
			break; // Use first matching customizer
		}
	}
}