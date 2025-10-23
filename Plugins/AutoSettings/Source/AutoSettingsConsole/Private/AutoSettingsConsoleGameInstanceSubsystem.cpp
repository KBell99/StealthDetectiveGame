// Copyright Sam Bonifacio. All Rights Reserved.

#include "AutoSettingsConsoleGameInstanceSubsystem.h"

#include "AutoSettingsConsoleEngineSubsystem.h"
#include "AutoSettingsLogs.h"
#include "ConsoleUtilities.h"
#include "ConsoleVariableObjectUtils.h"
#include "AutoSettingsConsoleVariable.h"
#include "Engine/AssetManager.h"
#include "Engine/Engine.h"
#include "Logging/StructuredLog.h"

void UAutoSettingsConsoleGameInstanceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOGFMT(LogAutoSettings, Log, "AutoSettings Console Game Instance Subsystem initializing");

	UAssetManager::CallOrRegister_OnCompletedInitialScan(FSimpleMulticastDelegate::FDelegate::CreateLambda([this]()
	{
		RegisterConsoleVariableClasses();
	}));
}

void UAutoSettingsConsoleGameInstanceSubsystem::RegisterConsoleVariableClasses()
{
	if (bHasRegisteredConsoleVariableClasses)
	{
		UE_LOGFMT(LogAutoSettings, VeryVerbose, "Console variable classes already registered, skipping");
		return;
	}

	UE_LOGFMT(LogAutoSettings, Log, "Creating console variable objects for game instance");

	// Verify engine subsystem has done its job
	UAutoSettingsConsoleEngineSubsystem* EngineSubsystem = GEngine->GetEngineSubsystem<UAutoSettingsConsoleEngineSubsystem>();
	if (!EngineSubsystem)
	{
		UE_LOGFMT(LogAutoSettings, Error, "Engine subsystem not available for console variable registration");
		return;
	}

	// Get all console variable classes
	TArray<UClass*> ConsoleVariableClasses = SB::AutoSettings::Console::GetAllConsoleVariableClasses();
	UE_LOGFMT(LogAutoSettings, Log, "Found {0} console variable classes to process", ConsoleVariableClasses.Num());

	if (ConsoleVariableClasses.Num() == 0)
	{
		UE_LOGFMT(LogAutoSettings, Warning, "No console variable classes found - AutoSettings console features may not be available");
		bHasRegisteredConsoleVariableClasses = true;
		return;
	}

	// Count expected vs available CVars for validation
	int32 ExpectedCount = 0;
	int32 AvailableCount = 0;
	int32 CreatedObjectCount = 0;
	int32 FailedObjectCount = 0;

	// Create UObjects for runtime callbacks
	// Note: Native console variables should already be registered by the engine subsystem
	for (const UClass* Class : ConsoleVariableClasses)
	{
		UAutoSettingsConsoleVariable* CDO = Class->GetDefaultObject<UAutoSettingsConsoleVariable>();
		if (!CDO || CDO->Name.IsNone())
		{
			UE_LOGFMT(LogAutoSettings, Warning, "Console variable class {0} has invalid CDO or empty name, skipping", GetNameSafe(Class));
			FailedObjectCount++;
			continue;
		}

		ExpectedCount++;

		// Check if native CVar exists
		bool bCVarExists = SB::AutoSettings::Console::IsVariableRegistered(CDO->Name);
		if (bCVarExists)
		{
			AvailableCount++;
		}
		else
		{
			UE_LOGFMT(LogAutoSettings, Warning, "Expected console variable '{0}' is not registered - engine subsystem may not have run yet", CDO->Name);
		}

		// Create UObject regardless of CVar availability (for future registration)
		UAutoSettingsConsoleVariable* NewConsoleVariable = NewObject<UAutoSettingsConsoleVariable>(this, Class);
		if (!ensure(NewConsoleVariable))
		{
			UE_LOGFMT(LogAutoSettings, Error, "Failed to create console variable object for class {0}", GetNameSafe(Class));
			FailedObjectCount++;
			continue;
		}

		// Register callbacks only (native CVar should already exist from engine subsystem)
		NewConsoleVariable->RegisterCallback();

		ConsoleVariableObjects.Add(NewConsoleVariable);
		CreatedObjectCount++;

		UE_LOGFMT(LogAutoSettings, VeryVerbose, "Created console variable object: {0} (CVar exists: {1})", 
			NewConsoleVariable->Name, bCVarExists ? TEXT("Yes") : TEXT("No"));
	}

	// Report comprehensive results
	UE_LOGFMT(LogAutoSettings, Log, "Console variable object creation complete - Created: {0}, Failed: {1}, Total Classes: {2}", 
		CreatedObjectCount, FailedObjectCount, ConsoleVariableClasses.Num());

	UE_LOGFMT(LogAutoSettings, Log, "Console variable availability - Expected: {0}, Available: {1}, Missing: {2}", 
		ExpectedCount, AvailableCount, ExpectedCount - AvailableCount);

	if (AvailableCount < ExpectedCount)
	{
		UE_LOGFMT(LogAutoSettings, Warning, "{0} console variables are missing - some AutoSettings features may not work correctly", 
			ExpectedCount - AvailableCount);
	}

	bHasRegisteredConsoleVariableClasses = true;
	UE_LOGFMT(LogAutoSettings, Log, "AutoSettings Console Game Instance Subsystem registration complete");
}