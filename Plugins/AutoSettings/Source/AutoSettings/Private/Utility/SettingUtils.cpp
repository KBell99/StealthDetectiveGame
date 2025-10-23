// Copyright Sam Bonifacio. All Rights Reserved.


#include "Utility/SettingUtils.h"

#include "Framework/SettingType.h"
#include "Utility/AutoSettingsAssetUtils.h"
#include "AutoSettingsLogs.h"
#include "HAL/IConsoleManager.h"
#include "Logging/StructuredLog.h"

static FAutoConsoleCommandWithWorld LogSettingsCommand(
	TEXT("AutoSettings.LogSettings"),
	TEXT("Logs all settings found"),
	FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
	{
		TArray<USettingType*> Assets = FSettingUtils::GetAllSettingTypes();
		UE_LOGFMT(LogAutoSettings, Display, "Found {0} settings", Assets.Num());
		for (const USettingType* Asset : Assets)
		{
			UE_LOGFMT(LogAutoSettings, Display, "    {0}", Asset->GetName());
		}
	}),
	ECVF_Default);

TArray<USettingType*> FSettingUtils::GetAllSettingTypes()
{
	TArray<USettingType*> Settings = FAutoSettingsAssetUtils::LoadAndGetAllAssets<USettingType>(
		USettingType::SettingPrimaryAssetTypeName, true);
	// Remove disabled settings
	Settings.RemoveAll([](const USettingType* Setting)
	{
		return !Setting->bEnabled;
	});
	return Settings;
}