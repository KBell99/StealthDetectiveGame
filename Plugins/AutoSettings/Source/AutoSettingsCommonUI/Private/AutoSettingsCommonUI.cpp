// Copyright Sam Bonifacio. All Rights Reserved.
#include "AutoSettingsCommonUI.h"
#include "AutoSettingsLogs.h"

#define LOCTEXT_NAMESPACE "FAutoSettingsCommonUIModule"

void FAutoSettingsCommonUIModule::StartupModule()
{
	UE_LOG(LogAutoSettings, Log, TEXT("AutoSettingsCommonUI loaded"));
}

void FAutoSettingsCommonUIModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAutoSettingsCommonUIModule, AutoSettingsCommonUI)