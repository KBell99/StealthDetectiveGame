// Copyright Sam Bonifacio. All Rights Reserved.

#include "AutoSettingsConsoleEngineSubsystem.h"

#include "AutoSettingsError.h"
#include "AutoSettingsLogs.h"
#include "ConsoleUtilities.h"
#include "ConsoleVariableObjectUtils.h"
#include "AutoSettingsConsoleVariable.h"
#include "AssetRegistry/AssetData.h"
#include "Engine/AssetManager.h"
#include "Engine/Engine.h"
#include "Logging/StructuredLog.h"

#if WITH_EDITOR
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/Blueprint.h"
#endif

void UAutoSettingsConsoleEngineSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOGFMT(LogAutoSettings, Log, "AutoSettings Console Engine Subsystem initializing");

	// Delay registration until Asset Manager completes initial scan
	// This ensures all console variable assets are discovered before attempting registration
	UAssetManager::CallOrRegister_OnCompletedInitialScan(FSimpleMulticastDelegate::FDelegate::CreateLambda([this]()
	{
		RegisterConsoleVariables();
	}));

#if WITH_EDITOR
	// Register for asset registry notifications to handle console variable asset changes in editor
	if (FModuleManager::Get().IsModuleLoaded("AssetRegistry"))
	{
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		AssetRegistryModule.Get().OnAssetAdded().AddUObject(this, &ThisClass::OnAssetChanged);
		AssetRegistryModule.Get().OnAssetRemoved().AddUObject(this, &ThisClass::OnAssetRemoved);
		AssetRegistryModule.Get().OnAssetUpdated().AddUObject(this, &ThisClass::OnAssetChanged);
		UE_LOGFMT(LogAutoSettings, Log, "Registered for asset registry notifications in editor");
	}
	else
	{
		UE_LOGFMT(LogAutoSettings, Warning, "Asset Registry module not loaded, asset change monitoring disabled");
	}
#endif

	UE_LOGFMT(LogAutoSettings, Log, "AutoSettings Console Engine Subsystem initialization complete - console variable registration will occur after asset scan");
}

bool UAutoSettingsConsoleEngineSubsystem::IsRegistered(FName NameToCheck) const
{
	for (IConsoleVariable* ConsoleVariable : ConsoleVariables)
	{
		if (!ensure(ConsoleVariable))
		{
			continue;
		}

		if (GetConsoleVariableName(ConsoleVariable) == NameToCheck)
		{
			return true;
		}
	}
	return false;
}

void UAutoSettingsConsoleEngineSubsystem::UnregisterOldConsoleVariables(TArray<FName> CVarNamesToKeep)
{
	// Clean up orphaned console variables when assets are rescanned
	// This removes CVars for deleted assets to prevent accumulation during development
	// Get list of currently registered CVar names
	TArray<FName> CurrentlyRegisteredNames;
	for (IConsoleVariable* ConsoleVariable : ConsoleVariables)
	{
		CurrentlyRegisteredNames.Add(GetConsoleVariableName(ConsoleVariable));
	}

	// Unregister any console variables that are no longer backed by assets
	// This prevents accumulation of orphaned CVars during development
	for (const FName& Name : CurrentlyRegisteredNames)
	{
		if (!CVarNamesToKeep.Contains(Name))
		{
			UE_LOG(LogAutoSettings, Log, TEXT("Unregistering orphaned console variable '%s'"), *Name.ToString());
			UnregisterConsoleVariableByName(Name);

			// This should only be called when no classes need this CVar anymore
			// The tracking maps should already be empty for this name
			ensure(!ConsoleVariableNameToClasses.Contains(Name));
		}
	}
}

void UAutoSettingsConsoleEngineSubsystem::Add(IConsoleVariable* ConsoleVariable)
{
	// Prevent duplicates
	ConsoleVariables.AddUnique(ConsoleVariable);
}

bool UAutoSettingsConsoleEngineSubsystem::UnregisterConsoleVariableByName(FName CVarName)
{
	bool bFound = false;

	ConsoleVariables.RemoveAll([this, CVarName, &bFound](IConsoleVariable* CVar)
	{
		if (GetConsoleVariableName(CVar) == CVarName)
		{
			IConsoleManager::Get().UnregisterConsoleObject(CVar);
			bFound = true;
			return true;
		}
		return false;
	});

	return bFound;
}

FName UAutoSettingsConsoleEngineSubsystem::GetConsoleVariableName(IConsoleVariable* ConsoleVariable)
{
	if (!ConsoleVariable)
	{
		return NAME_None;
	}

	return FName(IConsoleManager::Get().FindConsoleObjectName(ConsoleVariable));
}

void UAutoSettingsConsoleEngineSubsystem::RemoveClassFromCVarTracking(TSoftClassPtr<UAutoSettingsConsoleVariable> ConsoleVariableClass, FName CVarName)
{
	TSet<TSoftClassPtr<UAutoSettingsConsoleVariable>>* ClassesUsingName = ConsoleVariableNameToClasses.Find(CVarName);
	if (!ClassesUsingName)
	{
		return;
	}

	ClassesUsingName->Remove(ConsoleVariableClass);

	// Reference counting: Only unregister CVar when no other classes use this name
	// This handles cases where multiple assets share the same console variable name
	if (ClassesUsingName->Num() == 0)
	{
		UE_LOGFMT(LogAutoSettings, Log, "No other classes using '{0}', unregistering console variable", CVarName);
		UnregisterConsoleVariableByName(CVarName);
		ConsoleVariableNameToClasses.Remove(CVarName);
	}
	else
	{
		UE_LOGFMT(LogAutoSettings, Log, "Console variable '{0}' still in use by {1} other class(es)", CVarName, ClassesUsingName->Num());
	}
}

void UAutoSettingsConsoleEngineSubsystem::RegisterConsoleVariables()
{
	UE_LOGFMT(LogAutoSettings, Log, "Starting console variable registration from engine subsystem");

	// Use the internal registration method
	auto Result = RegisterNativeConsoleVariables();

	// Report results with a detailed breakdown
	UE_LOGFMT(LogAutoSettings, Log, "Console variable registration complete - Registered: {0}, Skipped: {1}, Failed: {2}",
		Result.RegisteredCVars.Num(), Result.SkippedNames.Num(), Result.FailedNames.Num());

	if (Result.FailedNames.Num() > 0)
	{
		UE_LOGFMT(LogAutoSettings, Warning, "Some console variables failed to register - this may affect settings functionality");
		for (const FName& FailedName : Result.FailedNames)
		{
			UE_LOGFMT(LogAutoSettings, Warning, "Failed to register console variable: {0}", FailedName);
		}
	}

	if (Result.RegisteredCVars.Num() > 0)
	{
		UE_LOGFMT(LogAutoSettings, Log, "Successfully registered {0} console variables for AutoSettings", Result.RegisteredCVars.Num());
	}
}

UAutoSettingsConsoleEngineSubsystem::FConsoleVariableRegistrationResult UAutoSettingsConsoleEngineSubsystem::RegisterNativeConsoleVariables()
{
	FConsoleVariableRegistrationResult Result;

	UE_LOG(LogAutoSettings, Log, TEXT("Registering native console variables"));

	TArray<TSubclassOf<UAutoSettingsConsoleVariable>> ConsoleVariableClasses;
	for (UClass* Class : SB::AutoSettings::Console::GetAllConsoleVariableClasses())
	{
		ConsoleVariableClasses.Add(Class);
	}

	UE_LOGFMT(LogAutoSettings, Log, "Found {0} console variable class(es) for registration", ConsoleVariableClasses.Num());

	// Register each console variable using the single registration method
	for (TSubclassOf<UAutoSettingsConsoleVariable> Class : ConsoleVariableClasses)
	{
		if (RegisterConsoleVariable(Class))
		{
			// Get the CVar that was just registered
			UAutoSettingsConsoleVariable* CDO = Class->GetDefaultObject<UAutoSettingsConsoleVariable>();
			FName Name = CDO->Name;

			// Find the IConsoleVariable* that was just added
			for (IConsoleVariable* CVar : ConsoleVariables)
			{
				if (GetConsoleVariableName(CVar) == Name)
				{
					Result.RegisteredCVars.Add(CVar);
					break;
				}
			}
		}
		else
		{
			// Determine if it was skipped or failed
			UAutoSettingsConsoleVariable* CDO = Class->GetDefaultObject<UAutoSettingsConsoleVariable>();
			if (CDO)
			{
				FName Name = CDO->Name;
				if (IsRegistered(Name))
				{
					Result.SkippedNames.Add(Name);
				}
				else
				{
					Result.FailedNames.Add(Name);
				}
			}
		}
	}

	// Clean up orphaned CVars (those not in current set)
	TArray<FName> CurrentNames;
	for (const auto& Pair : ConsoleVariableNameToClasses)
	{
		CurrentNames.Add(Pair.Key);
	}
	UnregisterOldConsoleVariables(CurrentNames);

	return Result;
}

bool UAutoSettingsConsoleEngineSubsystem::RegisterConsoleVariable(TSubclassOf<UAutoSettingsConsoleVariable> ConsoleVariableClass)
{
	if (!ConsoleVariableClass)
	{
		return false;
	}

	UAutoSettingsConsoleVariable* CDO = ConsoleVariableClass->GetDefaultObject<UAutoSettingsConsoleVariable>();
	if (!CDO || CDO->Name.IsNone())
	{
		UE_LOGFMT(LogAutoSettings, Warning, "Console variable class {0} has invalid CDO or empty name, skipping", GetNameSafe(ConsoleVariableClass));
		return false;
	}

	FName NewName = CDO->Name;

	// Convert to soft reference for tracking
	TSoftClassPtr<UAutoSettingsConsoleVariable> SoftClassPtr(ConsoleVariableClass);

	// Check if this class previously registered a different name
	FName* OldName = ConsoleVariableClassToName.Find(SoftClassPtr);

	// If name hasn't changed and it's already registered, skip
	if (OldName && *OldName == NewName)
	{
		UE_LOGFMT(LogAutoSettings, VeryVerbose, "Console variable '{0}' already registered with same name, skipping", NewName);
		return false;
	}

	// Handle name changes during development: remove class from old name's tracking set
	// This ensures proper cleanup when console variable assets are renamed
	if (OldName && *OldName != NewName)
	{
		UE_LOGFMT(LogAutoSettings, Log, "Console variable name changed from '{0}' to '{1}' for class {2}", *OldName, NewName, GetNameSafe(ConsoleVariableClass));
		RemoveClassFromCVarTracking(SoftClassPtr, *OldName);
	}

	// Check if the new name is already registered
	bool bNeedToCreateCVar = false;
	if (SB::AutoSettings::Console::IsVariableRegistered(NewName))
	{
		// Check if it's registered by us
		if (!IsRegistered(NewName))
		{
			UE_LOGFMT(LogAutoSettings, Warning, "Console variable '{0}' is already registered by another system, skipping registration", NewName);
			return false;
		}
		// If we registered it, we'll just add this class to the tracking
	}
	else
	{
		// Need to create the CVar
		bNeedToCreateCVar = true;
	}

	// Create the console variable if needed
	if (bNeedToCreateCVar)
	{
		IConsoleVariable* NewCVar = CDO->CreateConsoleVariable();
		if (!NewCVar)
		{
			UE_LOGFMT(LogAutoSettings, Error, "Failed to create console variable: {0}", NewName);
			return false;
		}

		Add(NewCVar);
		UE_LOGFMT(LogAutoSettings, Log, "Created native console variable: {0}", NewName);
	}

	// Update tracking maps
	ConsoleVariableClassToName.Add(SoftClassPtr, NewName);
	ConsoleVariableNameToClasses.FindOrAdd(NewName).Add(SoftClassPtr);

	UE_LOGFMT(LogAutoSettings, Log, "Registered class {0} for console variable '{1}'", GetNameSafe(ConsoleVariableClass), NewName);
	return true;
}

#if WITH_EDITOR
void UAutoSettingsConsoleEngineSubsystem::OnAssetChanged(const FAssetData& AssetData)
{
	// Only respond to Blueprint assets that might generate console variable classes
	if (AssetData.AssetClassPath == UBlueprint::StaticClass()->GetClassPathName())
	{
		// Check if Asset Manager is available and initial scan is complete before trying to scan assets
		if (!UAssetManager::IsInitialized())
		{
			UE_LOGFMT(LogAutoSettings, VeryVerbose, "Asset Manager not initialized, skipping console variable re-registration for: {0}", AssetData.GetObjectPathString());
			return;
		}

		if (!UAssetManager::Get().HasInitialScanCompleted())
		{
			UE_LOGFMT(LogAutoSettings, VeryVerbose, "Asset Manager initial scan not complete, skipping console variable re-registration for: {0}", AssetData.GetObjectPathString());
			return;
		}

		// Load the Blueprint and check its generated class
		if (UBlueprint* Blueprint = Cast<UBlueprint>(AssetData.GetAsset()))
		{
			if (Blueprint->GeneratedClass && Blueprint->GeneratedClass->IsChildOf<UAutoSettingsConsoleVariable>())
			{
				UE_LOGFMT(LogAutoSettings, VeryVerbose, "Console variable Blueprint changed: {0}", AssetData.GetObjectPathString());

				// Only re-register the specific console variable that changed
				TSubclassOf<UAutoSettingsConsoleVariable> ConsoleVariableClass = Cast<UClass>(Blueprint->GeneratedClass);
				if (ConsoleVariableClass && RegisterConsoleVariable(ConsoleVariableClass))
				{
					UE_LOGFMT(LogAutoSettings, Log, "Successfully re-registered console variable for changed Blueprint: {0}", AssetData.GetObjectPathString());
				}
			}
		}
	}
}

void UAutoSettingsConsoleEngineSubsystem::OnAssetRemoved(const FAssetData& AssetData)
{
	// Only respond to Blueprint assets that might generate console variable classes
	if (AssetData.AssetClassPath != UBlueprint::StaticClass()->GetClassPathName())
	{
		return;
	}

	// Convert the asset path to the expected Blueprint generated class path
	// Asset path: /Game/Path/To/CVar_Name.CVar_Name
	// Generated class path: /Script/Engine.BlueprintGeneratedClass'/Game/Path/To/CVar_Name.CVar_Name_C'
	FString AssetPath = AssetData.GetObjectPathString();
	FString GeneratedClassPath = FString::Printf(TEXT("/Script/Engine.BlueprintGeneratedClass'%s_C'"), *AssetPath);
	TSoftClassPtr<UAutoSettingsConsoleVariable> ClassToRemove(GeneratedClassPath);

	// Look up this soft reference in our tracking map
	FName* CVarNamePtr = ConsoleVariableClassToName.Find(ClassToRemove);
	if (!CVarNamePtr)
	{
		// This asset wasn't a console variable class we were tracking
		UE_LOGFMT(LogAutoSettings, VeryVerbose, "Asset {0} was not tracked as a console variable class", AssetData.GetObjectPathString());
		return;
	}

	FName CVarNameToCheck = *CVarNamePtr;
	UE_LOGFMT(LogAutoSettings, Log, "Console variable Blueprint removed: {0}, unregistering CVar '{1}'", AssetData.GetObjectPathString(), CVarNameToCheck);

	// Remove this class from tracking
	ConsoleVariableClassToName.Remove(ClassToRemove);

	// Remove this class from the name tracking
	RemoveClassFromCVarTracking(ClassToRemove, CVarNameToCheck);
}
#endif