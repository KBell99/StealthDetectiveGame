// Copyright Sam Bonifacio. All Rights Reserved.

#include "AutoSettingsEditor.h"
#include "AutoSettingsWidgetDetails.h"
#include "PropertyEditorModule.h"
#include "AutoSettingsInput/Public/Widgets/InputMappingSettingWidget.h"
#include "Widgets/AutoSettingWidget.h"
#include "SettingStrategyDetailRegistry.h"
#include "StrategyUIHandlers/ConsoleVariableDetailCustomizer.h"

#define LOCTEXT_NAMESPACE "FAutoSettingsEditorModule"

void FAutoSettingsEditorModule::StartupModule()
{
	// Register setting widget details
	FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomClassLayout(UAutoSettingWidget::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FAutoSettingsWidgetDetails::MakeInstance));
	PropertyModule.RegisterCustomClassLayout(UInputMappingSettingWidget::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FAutoSettingsWidgetDetails::MakeInstance));
	
	// Register strategy detail customizers
	FSettingStrategyDetailRegistry& Registry = FSettingStrategyDetailRegistry::Get();
	Registry.RegisterCustomizer(MakeShareable(new FConsoleVariableDetailCustomizer()));
}

void FAutoSettingsEditorModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAutoSettingsEditorModule, AutoSettingsEditor)