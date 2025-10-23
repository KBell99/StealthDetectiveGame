// Copyright Sam Bonifacio. All Rights Reserved.


#include "AutoSettingsWidgetDetails.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "PropertyCustomizationHelpers.h"
#include "Widgets/AutoSettingWidget.h"
#include "Framework/SettingType.h"
#include "Framework/SettingKey.h"
#include "SettingStrategyDetailRegistry.h"


TSharedRef<IDetailCustomization> FAutoSettingsWidgetDetails::MakeInstance()
{
	return MakeShareable(new FAutoSettingsWidgetDetails);
}

void FAutoSettingsWidgetDetails::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	// Unfortunately the widget detail panel seems to show base widget categories before subclass ones
	// So sort categories here to show our own Auto Settings categories before anything else
	DetailLayout.SortCategories([](const TMap<FName, IDetailCategoryBuilder*>& AllCategoryMap)
	{
		static const TArray<FName> CategoryOrder =
		{
			// We add every relevant category here and then use the one layout class for multiple widgets
			TEXT("Target"),
			TEXT("Strategy"),
			TEXT("Setting"),
			TEXT("Select Setting"),
			TEXT("Radio Select Setting"),
			TEXT("Input Mapping Selector")
		};

		for (const TPair<FName, IDetailCategoryBuilder*>& Pair : AllCategoryMap)
		{
			int32 CurrentSortOrder = Pair.Value->GetSortOrder();

			int32 DesiredSortOrder;
			if (CategoryOrder.Find(Pair.Key, DesiredSortOrder))
			{
				CurrentSortOrder = DesiredSortOrder;
			}
			else
			{
				CurrentSortOrder += CategoryOrder.Num();
			}

			Pair.Value->SetSortOrder(CurrentSortOrder);
		}
	});

	// Get the object being customized
	TArray<TWeakObjectPtr<UObject>> CustomizedObjects;
	DetailLayout.GetObjectsBeingCustomized(CustomizedObjects);

	if (CustomizedObjects.Num() == 1)
	{
		if (UAutoSettingWidget* AutoSettingWidget = Cast<UAutoSettingWidget>(CustomizedObjects[0].Get()))
		{
			// Add strategy-specific UI fields
			AddStrategyFields(DetailLayout);
		}
	}
}

void FAutoSettingsWidgetDetails::AddStrategyFields(IDetailLayoutBuilder& DetailLayout)
{
	// Get references to the properties we need
	TSharedRef<IPropertyHandle> SettingTypeHandle = DetailLayout.GetProperty("SettingType");
	TSharedRef<IPropertyHandle> SubKeyHandle = DetailLayout.GetProperty("SubKey");
	
	UObject* SettingTypeValue = nullptr;
	if (SettingTypeHandle->GetValue(SettingTypeValue) != FPropertyAccess::Success || !SettingTypeValue)
	{
		return;
	}
	
	UClass* SettingTypeClass = Cast<UClass>(SettingTypeValue);
	if (!SettingTypeClass)
	{
		return;
	}
	
	const USettingType* SettingTypeCDO = GetDefault<USettingType>(SettingTypeClass);
	if (!SettingTypeCDO)
	{
		return;
	}
	
	// Let the registry handle UI for application strategy
	if (SettingTypeCDO->ApplicationStrategy)
	{
		// Get the Strategy category for strategy-specific fields
		IDetailCategoryBuilder& StrategyCategory = DetailLayout.EditCategory("Strategy");
		
		FSettingStrategyDetailRegistry::Get().AddFieldsForStrategy(
			StrategyCategory,
			SettingTypeCDO->ApplicationStrategy,
			SubKeyHandle,
			SettingTypeCDO
		);
	}
	
	// TODO: Add support for persistence strategy handlers in the future
	// (PersistenceStrategy is a different base class than ApplicationStrategy)
}
