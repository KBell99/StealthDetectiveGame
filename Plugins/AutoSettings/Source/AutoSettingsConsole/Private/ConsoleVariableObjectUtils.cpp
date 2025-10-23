// Copyright Sam Bonifacio. All Rights Reserved.

#include "ConsoleVariableObjectUtils.h"
#include "AutoSettingsConsoleVariable.h"
#include "Utility/AutoSettingsAssetUtils.h"

namespace SB::AutoSettings::Console
{
	TArray<UClass*> GetAllConsoleVariableClasses()
	{
		TArray<UClass*> Classes = FAutoSettingsAssetUtils::LoadAndGetAllAssets<UClass>(
			UAutoSettingsConsoleVariable::ConsoleVariablePrimaryAssetTypeName, false);
		Classes.RemoveAll([](const UClass* Class)
		{
			// Don't include abstract classes
			return Class->HasAnyClassFlags(CLASS_Abstract);
		});
		return Classes;
	}

	TArray<UAutoSettingsConsoleVariable*> GetAllConsoleVariableDefaultObjects()
	{
		TArray<UAutoSettingsConsoleVariable*> DefaultObjects;
		for (UClass* Class : GetAllConsoleVariableClasses())
		{
			DefaultObjects.Add(Class->GetDefaultObject<UAutoSettingsConsoleVariable>());
		}
		return DefaultObjects;
	}
}