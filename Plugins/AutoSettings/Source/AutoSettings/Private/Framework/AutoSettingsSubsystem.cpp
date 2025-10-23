// Copyright Sam Bonifacio. All Rights Reserved.

#include "Framework/AutoSettingsSubsystem.h"

#include "AutoSettingsConsoleGameInstanceSubsystem.h"
#include "AutoSettingsError.h"
#include "AutoSettingsLogs.h"
#include "Engine/AssetManager.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Logging/StructuredLog.h"
#include "Framework/SettingType.h"
#include "Framework/SettingRegistry.h"
#include "Misc/AutoSettingsGameUserSettings.h"
#include "Subsystems/SubsystemCollection.h"
#include "Utility/SettingUtils.h"

UAutoSettingsSubsystem::UAutoSettingsSubsystem()
{
	GameSettingRegistry = CreateDefaultSubobject<USettingRegistry>(TEXT("GameSettingRegistry"), true);
	GameSettingRegistry->BaseKey.Fragments = { { "Global" } };
}

void UAutoSettingsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Collection.InitializeDependency<UAutoSettingsConsoleGameInstanceSubsystem>();

	Super::Initialize(Collection);

	// Only initialize settings after the initial asset scan is complete
	UAssetManager::CallOrRegister_OnCompletedInitialScan(FSimpleMulticastDelegate::FDelegate::CreateLambda([this]()
	{
		GetGameInstance()->GetSubsystem<UAutoSettingsConsoleGameInstanceSubsystem>()->RegisterConsoleVariableClasses();

		// Find all game settings
		const TArray<USettingType*> Settings = FSettingUtils::GetAllSettingTypes().FilterByPredicate(
			[](const USettingType* Setting)
			{
				return Setting->SettingDomain == ESettingDomain::Global;
			});

		UE_LOGFMT(LogAutoSettings, Log, "{0}: Initializing global registry", GetNameSafe(GetGameInstance()));
		GameSettingRegistry->Initialize(Settings);
	}));

	SB::AutoSettings::Errors::ValidateSettingsProperty(
		"Engine - General Settings",
		GET_MEMBER_NAME_STRING_CHECKED(UEngine, GameUserSettingsClassName),
		GEngine->GameUserSettings->GetClass(),
		UAutoSettingsGameUserSettings::StaticClass()
		);

}

void UAutoSettingsSubsystem::Deinitialize()
{
	if (GameSettingRegistry->IsInitialized())
	{
		UE_LOGFMT(LogAutoSettings, Log, "{0}: Deinitializing global registry", GetNameSafe(GetGameInstance()));
		GameSettingRegistry->Deinitialize();
	}

	Super::Deinitialize();
}

