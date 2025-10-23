// Copyright Sam Bonifacio. All Rights Reserved.


#include "Framework/SettingRegistry.h"
#include "AutoSettingsLogs.h"
#include "Algo/TopologicalSort.h"
#include "Logging/StructuredLog.h"
#include "Framework/SettingType.h"
#include "Framework/SettingApplicationStrategy.h"
#include "Framework/SettingKey.h"
#include "Framework/SettingDefaultProvider.h"

#if WITH_EDITOR
#include "Editor.h"
#endif

void USettingRegistry::Initialize(TArray<USettingType*> Settings)
{
	if (!ensure(RegistryState == ESettingRegistryState::Uninitialized))
	{
		// Don't allow initializing more than once
		return;
	}

	UE_LOGFMT(LogAutoSettings, Log, "{0} initializing", GetName());

	SettingAssets = Settings;

	// Register all SettingTypes early to ensure providers run before settings are registered
	// This ensures that provider-generated default values are available when settings initialize
	UE_LOGFMT(LogAutoSettings, Log, "Pre-registering {0} SettingTypes to execute default providers", Settings.Num());
	for (USettingType* SettingAsset : Settings)
	{
		if (SettingAsset && SettingAsset->bEnabled && !IsSettingTypeRegistered(SettingAsset))
		{
			RegisterSettingType(SettingAsset);
		}
	}
	UE_LOGFMT(LogAutoSettings, Log, "Completed pre-registration of SettingTypes");

	// Key discovery algorithm: Build a map of all setting keys to their most specific setting asset
	// This handles inheritance where child settings can override parent settings for the same key
	TMap<FFullSettingKey, USettingType*> KeyMap;

	for (USettingType* Setting : Settings)
	{
		TArray<FFullSettingKey> Keys;

		// Add the setting's own key if it has one
		if (!Setting->KeyFragment.IsNone())
		{
			Keys.Add(GetSettingKey(Setting));
		}

		// Add any explicitly allowed child keys
		Keys.Append(GetAllowedChildKeys(Setting));

		// Let the persistence strategy discover additional keys (e.g., from config files)
		if (ensure(Setting->PersistenceStrategy))
		{
			Keys.Append(Setting->PersistenceStrategy->FindChildKeys(GetSettingKey(Setting)));
		}
		
		// For each discovered key, assign it to the most specific (child-most) setting
		// This ensures child settings override parent settings for the same key
		for (FFullSettingKey& Key : Keys)
		{
			const bool bShouldUpdate = !KeyMap.Contains(Key) || GetSettingKey(Setting).IsChildOf(GetSettingKey(KeyMap[Key]));
			if (!bShouldUpdate)
			{
				continue;
			}
			USettingType*& KeySetting = KeyMap.FindOrAdd(Key);
			KeySetting = Setting;
		}
	}

	for (const auto& KeyValue : KeyMap)
	{
		if (IsCategorySetting(KeyValue.Key, KeyValue.Value))
		{
			// Key is a category, skip
			continue;
		}
		SettingsPendingRegister.Add(KeyValue.Key, KeyValue.Value);
	}

	TryRegisterPendingSettings();

	// Provider execution now happens automatically during RegisterSetting via RegisterSettingType

#if WITH_EDITOR
	if (!bDesignTime)
	{
		FEditorDelegates::EndPIE.AddWeakLambda(this, [this](const bool bSimulatingInEditor)
		{
			// On end PIE, deinitialize early so that initial values can be restored while editor systems are still working
			UE_LOGFMT(LogAutoSettings, Log, "{0} end PIE detected", GetName());
			if (RegistryState == ESettingRegistryState::Initialized)
			{
				Deinitialize();
			}
		});
	}
#endif

	RegistryState = ESettingRegistryState::Initialized;

	// Broadcast delegate to tell other systems about initialization
	OnInitialized.Broadcast();
	// Clear delegate list
	OnInitialized.Clear();
}

void USettingRegistry::BeginDestroy()
{
	UObject::BeginDestroy();

	if (RegistryState == ESettingRegistryState::Initialized)
	{
		Deinitialize();
	}
}

void USettingRegistry::Deinitialize()
{
	if (!ensure(RegistryState == ESettingRegistryState::Initialized))
	{
		return;
	}

	UE_LOGFMT(LogAutoSettings, Log, "{0} deinitializing", GetName());

#if WITH_EDITOR
	if (!IsRunningGame())
	{
		UE_LOGFMT(LogAutoSettings, Log, "{0}: Deinitializing in editor, restore initial applied values", GetName());
		RestoreInitialValues();
	}
#endif

	RegistryState = ESettingRegistryState::Deinitialized;
}

void USettingRegistry::CallOrRegister_OnInitialized(const FSimpleDelegate& Delegate)
{
	if (IsInitialized())
	{
		Delegate.Execute();
	}
	else
	{
		OnInitialized.Add(Delegate);
	}
}

FString USettingRegistry::GetInitialDefaultValue(const FFullSettingKey& Key) const
{
	if (!ensure(IsSettingRegistered(Key)))
	{
		return FString();
	}
	return RegisteredSettings[Key].InitialDefaultValue;
}

bool USettingRegistry::HasControllerSetDefaultValue(const FFullSettingKey& Key) const
{
	if (!ensure(IsSettingRegistered(Key)))
	{
		return false;
	}
	return !RegisteredSettings[Key].ControllerSetDefaultValue.IsEmpty();
}

FString USettingRegistry::GetControllerSetDefaultValue(const FFullSettingKey& Key) const
{
	if (!ensure(IsSettingRegistered(Key)))
	{
		return FString();
	}
	return RegisteredSettings[Key].ControllerSetDefaultValue;
}

FString USettingRegistry::GetCurrentDefaultValue(const FFullSettingKey& Key) const
{
	if (!ensure(IsSettingRegistered(Key)))
	{
		return "";
	}

	if (HasControllerSetDefaultValue(Key))
	{
		// The default value was overridden by another setting
		return GetControllerSetDefaultValue(Key);
	}

	const USettingType* SettingAsset = FindSettingTypeForKey(Key);

	if (SettingAsset->bOverrideDefaultValue)
	{
		// The setting asset has provided its own default value
		return SettingAsset->DefaultValue;
	}

	// Get the initial value of the setting that was stored by the registry
	return GetInitialDefaultValue(Key);
}

bool USettingRegistry::SetInitialDefaultValue(const FFullSettingKey& Key, const FString& Value)
{
	if (!IsSettingRegistered(Key))
	{
		UE_LOGFMT(LogAutoSettings, Warning, "Cannot set initial default for unregistered setting: {0}", Key.ToString());
		return false;
	}

	FRegisteredSetting& RegisteredSetting = RegisteredSettings[Key];
	RegisteredSetting.InitialDefaultValue = Value;

	UE_LOGFMT(LogAutoSettings, Verbose, "Set initial default for '{0}' = '{1}'", Key.ToString(), Value);
	return true;
}

bool USettingRegistry::IsSettingRegistered(const FFullSettingKey& Key) const
{
	return RegisteredSettings.Contains(Key);
}

bool USettingRegistry::RegisterSetting(const FFullSettingKey& Key, const USettingType* SettingAsset)
{
	if (!ensure(!IsSettingRegistered(Key)))
	{
		return false;
	}

	if (!ensure(SettingAsset))
	{
		return false;
	}

	// Ensure the SettingType was already registered during initialization
	// All SettingTypes are now registered early to ensure providers run first
	if (!ensure(IsSettingTypeRegistered(SettingAsset)))
	{
		UE_LOGFMT(LogAutoSettings, Error, "SettingType '{0}' was not registered during initialization. This is unexpected and may indicate a logic error.", 
			   SettingAsset->GetName());
		return false;
	}

	if (!ensure(SettingAsset->ApplicationStrategy->CanRegister(this)))
	{
		return false;
	}

	auto AllowedChildKeys = GetAllowedChildKeys(SettingAsset);
	if (!AllowedChildKeys.IsEmpty() && !AllowedChildKeys.Contains(Key))
	{
		UE_LOGFMT(LogAutoSettings, Error, "Ignoring '{0}' because it is not in {1} AllowedChildFragments", Key.ToString(), GetNameSafe(SettingAsset));
		return false;
	}

	SettingAsset->ApplicationStrategy->ValidateConfiguration({ Key, this });
	
	FRegisteredSetting RegisteredSetting;
	RegisteredSetting.Key = Key;
	RegisteredSetting.SettingType = SettingAsset;
	RegisteredSetting.InitialDefaultValue = SettingAsset->ApplicationStrategy->GetAppliedValue({ Key, this });
	RegisteredSettings.Add(Key, RegisteredSetting);

	// Bind applied value modified callback so that we know when the applied value is updated
	FSettingAppliedValueModifiedSignature Delegate;
	Delegate.BindDynamic(this, &ThisClass::SettingAppliedValueChanged);
	SettingAsset->ApplicationStrategy->RegisterAppliedValueModifiedCallback({ Key, this }, Delegate);

	// Register control relationships with other settings
	TArray<FRegisteredSetting> OtherSettings;
	RegisteredSettings.GenerateValueArray(OtherSettings);
	for (auto& OtherSetting : OtherSettings)
	{
		if (OtherSetting.Key != Key)
		{
			ConditionalRegisterControlledSetting(RegisteredSetting, OtherSetting);
			ConditionalRegisterControlledSetting(OtherSetting, RegisteredSetting);
		}
	}


	UE_LOGFMT(LogAutoSettings, Log, "Registered setting '{0}' with setting asset '{1}'", Key.ToString(),
		SettingAsset->GetName());

	return true;
}

void USettingRegistry::RegisterControlledSetting(const FFullSettingKey& Setting, const FFullSettingKey& ControlledSetting)
{
	if (!ensure(RegisteredSettings.Contains(Setting)))
	{
		return;
	}

	if (!ensure(RegisteredSettings.Contains(ControlledSetting)))
	{
		return;
	}

	if (RegisteredSettings[Setting].ControlledSettings.Contains(ControlledSetting))
	{
		// Already registered
		return;
	}

	// Prevent circular control dependencies: A cannot control B if B already controls A
	// This maintains a directed acyclic graph of control relationships
	if (!ensure(!RegisteredSettings[ControlledSetting].ControlledSettings.Contains(Setting)))
	{
		// Circular dependency detected
		return;
	}

	RegisteredSettings[Setting].ControlledSettings.Add(ControlledSetting);

	UE_LOGFMT(LogAutoSettings, Log, "Registered controlled setting: '{0}' -> '{1}'", Setting.ToString(), ControlledSetting.ToString());
}

FFullSettingKey USettingRegistry::GetSettingKey(const USettingType* SettingAsset) const
{
	if (!ensure(SettingAsset))
	{
		return {};
	}

	FFullSettingKey Key;

	// Start with the registry base key if any
	Key += BaseKey;

	// Add the setting type's partial key (includes inheritance hierarchy)
	Key += SettingAsset->GetPartialSettingKey();

	ensure(Key.IsValid());
	return Key;
}

void USettingRegistry::ApplySettingByType(const TSubclassOf<USettingType>& SettingClass, const FPartialSettingKey& SubKey, const FString& Value, bool bSave)
{
	// Delegate to new unified API
	FPartialSettingKey Key = SB::AutoSettings::SettingKeys::MakeSettingKey(SettingClass, SubKey);
	ESettingAction Action = bSave ? ESettingAction::ApplyAndSave : ESettingAction::ApplyOnly;
	SetSetting(Key, Value, Action);
}

void USettingRegistry::ApplySettingByKey(const FFullSettingKey& Key, const FString& Value, bool bSave)
{
	// Delegate to new unified API
	ESettingAction Action = bSave ? ESettingAction::ApplyAndSave : ESettingAction::ApplyOnly;
	SetSetting(Key, Value, Action);
}

void USettingRegistry::SaveSettingByType(const TSubclassOf<USettingType>& SettingClass, const FPartialSettingKey& SubKey, const FString& Value)
{
	// Delegate to new unified API
	FPartialSettingKey Key = SB::AutoSettings::SettingKeys::MakeSettingKey(SettingClass, SubKey);
	SetSetting(Key, Value, ESettingAction::ApplyAndSave);
}

void USettingRegistry::SaveSettingByKey(const FFullSettingKey& Key, const FString& Value)
{
	// Delegate to new unified API
	SetSetting(Key, Value, ESettingAction::ApplyAndSave);
}

TArray<FFullSettingKey> USettingRegistry::GetControlledSettings(const FFullSettingKey& Key) const
{
	if (const FRegisteredSetting* RegisteredSetting = RegisteredSettings.Find(Key))
	{
		return RegisteredSetting->ControlledSettings;
	}
	return {};
}

// ========================================
// NEW SIMPLIFIED API IMPLEMENTATION
// ========================================

/**
 * Resolves partial setting keys to full keys by combining with the registry's base key.
 * Enables short-hand references (e.g. "Volume" -> "Player.0.Volume").
 */
FFullSettingKey USettingRegistry::ResolveSettingKey(const FSettingKey& Key) const
{
	FFullSettingKey FullKey;
	if (Key.Fragments.Num() > 0 && BaseKey.Fragments.Num() > 0 && !Key.IsChildOf(BaseKey))
	{
		// Input is a partial key that needs the base key prepended
		// Combine BaseKey + Key to create full hierarchical path
		FullKey += BaseKey;
		FullKey += Key;
	}
	else
	{
		// Input is already a full key (contains base) or base key is empty
		// Use the key fragments as-is
		FullKey.Fragments = Key.Fragments;
	}
	return FullKey;
}

void USettingRegistry::SetSetting(const FSettingKey& Key, const FString& Value, ESettingAction Action)
{
	if (RegistryState == ESettingRegistryState::Deinitialized)
	{
		UE_LOGFMT(LogAutoSettings, Warning, "Attempted to set setting '{0}' on deinitialized registry '{1}'. Operation ignored to prevent saving default values.", Key.ToString(), GetName());
		return;
	}

	const FFullSettingKey FullKey = ResolveSettingKey(Key);

	if (!ensure(IsSettingRegistered(FullKey)))
	{
		return;
	}

	const USettingType* SettingType = FindSettingTypeForKey(FullKey);

	// Apply the setting with proper scoping to manage controlled settings
	{
		FScopedInProgressSettingApplication SettingApplication(this, FullKey);
		SettingType->Apply(MakeSetSettingEvent(FullKey, Value));
		
		// Update ControllerSetDefaultValue for all controlled settings while in scope
		UpdateControlledSettingsDefaults(FullKey);
	}

	// Handle saving based on action
	switch (Action)
	{
		case ESettingAction::ApplyOnly:
			// Nothing more to do - just applied
			break;

		case ESettingAction::ApplyAndSave:
			// Save the main setting directly using the setting type
			SettingType->Save(MakeSetSettingEvent(FullKey, Value));

		// Handle controlled settings cascade: when a controller setting is saved,
			// we also need to save all the settings it controls with their current values
			// Example: When "Overall Quality" is saved, save "Shadow Quality", "Texture Quality", etc.
			const TArray<FFullSettingKey> ControlledSettingKeys = GetControlledSettings(FullKey);

			// Save all controlled settings with recursive saving to handle control chains
			// This ensures that if Setting A controls Setting B, and Setting B controls Setting C,
			// then saving Setting A will also save both B and C with their current applied values
			for (const FFullSettingKey& ControlledKey : ControlledSettingKeys)
			{
				if (IsSettingRegistered(ControlledKey))
				{
					const USettingType* ControlledSettingType = FindSettingTypeForKey(ControlledKey);
					if (ControlledSettingType)
					{
						// Get the current applied value of the controlled setting (not the config value)
						// This is important because the controlled setting might have been changed by the controller
						const FString ControlledValue = ControlledSettingType->ApplicationStrategy->GetAppliedValue({ ControlledKey, this });
						// Recursively save controlled settings to handle multi-level control chains
						SetSetting(ControlledKey, ControlledValue, ESettingAction::ApplyAndSave);
					}
				}
			}
			break;
	}
}


FString USettingRegistry::GetSettingValue(const FSettingKey& Key, ESettingValuePreference Preference) const
{
	const FFullSettingKey FullKey = ResolveSettingKey(Key);

	if (!ensure(IsSettingRegistered(FullKey)))
	{
		return FString();
	}

	// At design time, always return the default value since Applied/Saved values don't exist
	if (bDesignTime)
	{
		return GetCurrentDefaultValue(FullKey);
	}

	const USettingType* SettingType = FindSettingTypeForKey(FullKey);
	const FSettingContext Context(const_cast<USettingRegistry*>(this));

	FString PreferredValue;
	FString FallbackValue;

	// Get both values
	const FString AppliedValue = SettingType->ApplicationStrategy->GetAppliedValue({ FullKey, Context });
	const FSettingData SavedData = SettingType->PersistenceStrategy->GetSavedValue({ FullKey, Context });
	const FString SavedValue = SavedData.IsValid() ? SavedData.Value : FString();

	// Determine preferred and fallback based on preference
	switch (Preference)
	{
		case ESettingValuePreference::Applied:
			PreferredValue = AppliedValue;
			FallbackValue = SavedValue;
			break;

		case ESettingValuePreference::Saved:
			PreferredValue = SavedValue;
			FallbackValue = AppliedValue;
			break;
	}

	// Return preferred value if available, otherwise fallback
	return !PreferredValue.IsEmpty() ? PreferredValue : FallbackValue;
}

void USettingRegistry::Tick(float DeltaTime)
{
	if (SettingsPendingRegister.Num() > 0)
	{
		TryRegisterPendingSettings();
	}
}

TStatId USettingRegistry::GetStatId() const
{
	return {};
}

TArray<FFullSettingKey> USettingRegistry::GetRegisteredKeysForSetting(const USettingType* Setting) const
{
	if (!ensure(Setting))
	{
		return {};
	}

	TArray<FFullSettingKey> Result;
	for (auto& Key : RegisteredSettings)
	{
		if (Key.Value.SettingType == Setting)
		{
			Result.Add(Key.Key);
		}
	}
	return Result;
}

const USettingType* USettingRegistry::FindSettingTypeForKey(const FFullSettingKey& Key) const
{
	if (!ensure(IsSettingRegistered(Key)))
	{
		return {};
	}

	return RegisteredSettings[Key].SettingType;
}

TArray<FFullSettingKey> USettingRegistry::GetAllowedChildKeys(const USettingType* Setting) const
{
	const FFullSettingKey FullBaseKey = GetSettingKey(Setting);

	TArray<FFullSettingKey> Result;
	for (const FSettingKeyFragment& ChildKeyName : Setting->AllowedChildKeyFragments)
	{
		FFullSettingKey ChildFullKey = FullBaseKey;
		ChildFullKey += ChildKeyName;
		Result.Add(ChildFullKey);
	}

	return Result;
}

bool USettingRegistry::IsCategorySetting(const FFullSettingKey& Key, const USettingType* Setting) const
{
	// Determine if the setting key is a category, if so we shouldn't register it

	// If the key isn't the base key if the setting, it's not a category
	if (GetSettingKey(Setting) != Key)
	{
		// If it's not the same, it should be a child
		ensure(Key.IsChildOf(GetSettingKey(Setting)));
		return false;
	}

	// If the setting asset has any children, it's a category
	for (const auto OtherSetting : SettingAssets)
	{
		if (OtherSetting != Setting && OtherSetting->IsA(Setting->GetClass()))
		{
			return true;
		}
	}

	return false;
}

FSetSettingEvent USettingRegistry::MakeSetSettingEvent(const FFullSettingKey& Key, const FString& Value)
{
	const USettingType* Setting = FindSettingTypeForKey(Key);
	const FSettingData SettingValue{ Value };
	return FSetSettingEvent({ Key, FSettingContext(this) }, SettingValue, Setting->SettingTags);
}

/**
 * Sorts settings by control relationships to ensure controllers are applied before their controlled settings.
 * Uses topological sorting to handle dependency chains and detect circular dependencies.
 */
void SortSettings(const USettingRegistry* Registry, TArray<FFullSettingKey>& Settings)
{
	auto& RegisteredSettings = Registry->GetRegisteredSettings();

	// Create dependency resolver: For each setting, return the settings it controls
	// This builds the directed graph needed for topological sorting
	auto GetControlledSettings = [&](const FFullSettingKey& Setting) -> TArray<FFullSettingKey>
	{
		if (auto RegisteredSetting = RegisteredSettings.Find(Setting))
		{
			return RegisteredSetting->ControlledSettings;
		}
		return {};
	};

	// Apply Kahn's topological sorting algorithm to resolve dependency order
	// Returns false if circular dependencies are detected (impossible to sort)
	if (!Algo::TopologicalSort(Settings, GetControlledSettings))
	{
		// Circular dependency detected - this is a critical error that prevents
		// safe setting application order and could cause infinite update loops
		UE_LOGFMT(LogAutoSettings, Error, "Circular control relationship detected in settings");
	}

	// UE's TopologicalSort returns dependencies-first order (controlled before controllers)
	// We need controllers-first order for proper application sequence, so reverse the array
	Algo::Reverse(Settings);
}

void USettingRegistry::TryRegisterPendingSettings()
{
	auto SettingsToRegister = SettingsPendingRegister;

	for (const auto& KeyValue : SettingsToRegister)
	{
		if (!KeyValue.Value->ApplicationStrategy->CanRegister(this))
		{
			return;
		}
	}

	TArray<FFullSettingKey> RegisteredSettingKeys;
	for (const auto& KeyValue : SettingsToRegister)
	{
		if (RegisterSetting(KeyValue.Key, KeyValue.Value))
		{
			RegisteredSettingKeys.Add(KeyValue.Key);
		}
		else
		{
			UE_LOGFMT(LogAutoSettings, Error, "Failed to register setting '{0}' of type '{1}'", KeyValue.Key.ToString(), GetNameSafe(KeyValue.Value));
		}
		SettingsPendingRegister.Remove(KeyValue.Key);
	}

	// Load all saved values for the newly registered settings
	TMap<FFullSettingKey, FSettingData> ValueMap;
	for (const auto& SettingKey : RegisteredSettingKeys)
	{
		TMap<FFullSettingKey, FSettingData> SettingValues;
		const FSettingData SettingData = RegisteredSettings[SettingKey].SettingType->PersistenceStrategy->GetSavedValue({ SettingKey, this });
		if (SettingData.IsValid())
		{
			ValueMap.Add(SettingKey, SettingData);
		}
	}

	// Sort setting values to apply by known dependencies
	TArray<FFullSettingKey> Keys;
	ValueMap.GetKeys(Keys);

	SortSettings(this, Keys);

	TMap<FFullSettingKey, FSettingData> SortedValues;
	for (const auto& Key : Keys)
	{
		SortedValues.Add(Key, ValueMap[Key]);
	}

	ApplySettings(SortedValues);
}

bool USettingRegistry::ShouldRegisterControlledSetting(const FRegisteredSetting& Setting, const FRegisteredSetting& ControlledSetting)
{
	for (auto& SettingTypeControlledKey : Setting.SettingType->ControlledSettingKeys)
	{
		if (!SettingTypeControlledKey.IsValid())
		{
			continue;
		}

		if (ControlledSetting.Key.EndsWith(SettingTypeControlledKey))
		{
			return true;
		}
	}

	FSettingControlResult ControlsOtherSetting;
	OnCheckControlRelationship.Broadcast(Setting, ControlledSetting, ControlsOtherSetting);
	return ControlsOtherSetting.IsControlled();
}

void USettingRegistry::ConditionalRegisterControlledSetting(const FRegisteredSetting& Setting, const FRegisteredSetting& ControlledSetting)
{
	const bool bIsControlled = ShouldRegisterControlledSetting(Setting, ControlledSetting);
	UE_LOGFMT(LogAutoSettings, VeryVerbose, "Controlled Setting: {0}: '{1}' -> '{2}'", bIsControlled, Setting.Key.ToString(), ControlledSetting.Key.ToString());
	if (bIsControlled)
	{
		RegisterControlledSetting(Setting.Key, ControlledSetting.Key);
	}
}

void USettingRegistry::ApplySettings(TMap<FFullSettingKey, FSettingData> SettingsPendingApply)
{
	// Apply values
	for (const auto KeyValue : SettingsPendingApply)
	{
		const FFullSettingKey SettingKey = KeyValue.Key;
		const FSettingData SettingData = KeyValue.Value;
		const USettingType* SettingAsset = RegisteredSettings[SettingKey].SettingType;
		UE_LOGFMT(LogAutoSettings, Log, "Applying saved setting key: {0}, asset: {1}", SettingKey.ToString(),
			SettingAsset->GetName());
		SetSetting(SettingKey, SettingData.Value, ESettingAction::ApplyOnly);
	}
}

void USettingRegistry::SettingAppliedValueChanged(const FSettingReference& Setting)
{
	const USettingType* SettingAsset = FindSettingTypeForKey(Setting.Key);
	FString AppliedValue = SettingAsset->ApplicationStrategy->GetAppliedValue(Setting);

	if (InProgressSettingApplications.Num() > 0)
	{
		// If there's any settings currently being applied
		ensure(InProgressSettingApplications.Num() == 1);
		const FFullSettingKey InProgressSetting = InProgressSettingApplications[0];
		if (Setting.Key != InProgressSetting)
		{
			// A child setting is being applied
			// Update the contextual default value of the child setting to the current value that was set by the parent
			FString& Default = RegisteredSettings[Setting.Key].ControllerSetDefaultValue;
			const FString OldDefault = Default;
			Default = AppliedValue;
			UE_LOGFMT(LogAutoSettings, Verbose, "Updated ControllerSetDefaultValue for '{0}': '{1}' -> '{2}' (triggered by controller '{3}')",
				   Setting.Key.ToString(), OldDefault, Default, InProgressSetting.ToString());
		}
	}
	else
	{
		UE_LOGFMT(LogAutoSettings, Verbose, "SettingAppliedValueChanged for '{0}' but no in-progress application - ControllerSetDefaultValue NOT updated",
			   Setting.Key.ToString());
	}

	const FSetSettingEvent SetSettingEvent = MakeSetSettingEvent(Setting.Key, AppliedValue);
	OnSettingAppliedValueChanged.Broadcast(SetSettingEvent);
}

void USettingRegistry::PushInProgressApplication(const FFullSettingKey& Setting)
{
	ensure(InProgressSettingApplications.Num() == 0);
	InProgressSettingApplications.Add(Setting);
	UE_LOGFMT(LogAutoSettings, Verbose, "[SCOPED] Starting in-progress application for '{0}'", Setting.ToString());
}

void USettingRegistry::PopInProgressApplication()
{
	ensure(InProgressSettingApplications.Num() == 1);
	const FFullSettingKey PoppedSetting = InProgressSettingApplications.Last();
	InProgressSettingApplications.RemoveAt(InProgressSettingApplications.Num() - 1);
	UE_LOGFMT(LogAutoSettings, Verbose, "[SCOPED] Ending in-progress application for '{0}'", PoppedSetting.ToString());
}

void USettingRegistry::RestoreInitialValues()
{
	// Restore initial setting values
	// This is so when a PIE session ends, the CVars are restored to their original values, which is important because they could be stored again as the initial values in the next session

	// Should only run in editor
	check(!IsRunningGame());

	TArray<FFullSettingKey> RemainingSettings;
	RegisteredSettings.GenerateKeyArray(RemainingSettings);

	FFullSettingKey CurrentSettingBeingApplied;

	// Register a delegate so that we know when child settings are modified while restoring initials
	const FDelegateHandle AppliedCallbackHandle = OnSettingAppliedValueChanged.AddWeakLambda(this,
		[this, &RemainingSettings, &CurrentSettingBeingApplied](const FSetSettingEvent& SetSettingEvent)
		{
			const FFullSettingKey ModifiedSettingKey = SetSettingEvent.Setting.Key;
			if (ModifiedSettingKey != CurrentSettingBeingApplied)
			{
				// A child setting was modified as a result of restoring the parent setting
				if (!RemainingSettings.Contains(ModifiedSettingKey) &&
					FindSettingTypeForKey(ModifiedSettingKey)->ApplicationStrategy->GetAppliedValue({ ModifiedSettingKey, this }) != RegisteredSettings[ModifiedSettingKey].InitialDefaultValue)
				{
					// The child is no longer in the queue (was already restored) but its value now differs from its default
					// Add the child setting to the queue so it is restored to its real initial value again
					RemainingSettings.AddUnique(CurrentSettingBeingApplied);
				}
			}
		});

	// Process queue and apply
	while (RemainingSettings.Num() > 0)
	{
		CurrentSettingBeingApplied = RemainingSettings[0];
		const auto& RegisteredSetting = RegisteredSettings[CurrentSettingBeingApplied];
		const USettingType* Asset = RegisteredSetting.SettingType;
		if (Asset->ApplicationStrategy->GetAppliedValue({ CurrentSettingBeingApplied, this }) != RegisteredSetting.InitialDefaultValue)
		{
			UE_LOGFMT(LogAutoSettings, Log, "Restoring initial value of setting key: {0}, asset: {1}",
				CurrentSettingBeingApplied.ToString(), Asset->GetName());
			SetSetting(CurrentSettingBeingApplied, RegisteredSetting.InitialDefaultValue, ESettingAction::ApplyOnly);
		}
		RemainingSettings.RemoveAt(0);
	}

	OnSettingAppliedValueChanged.Remove(AppliedCallbackHandle);

	UE_LOGFMT(LogAutoSettings, Log, "Finished restoring initial applied values");
}

void USettingRegistry::UpdateControlledSettingsDefaults(const FFullSettingKey& ParentKey)
{
	// Only update if we're within a scoped application (the parent is being applied)
	if (InProgressSettingApplications.Num() == 0)
	{
		return;
	}

	const TArray<FFullSettingKey> ControlledSettings = GetControlledSettings(ParentKey);
	
	for (const FFullSettingKey& ControlledKey : ControlledSettings)
	{
		if (RegisteredSettings.Contains(ControlledKey))
		{
			// Get the current applied value of the controlled setting
			const USettingType* ControlledType = FindSettingTypeForKey(ControlledKey);
			if (ControlledType)
			{
				const FString CurrentValue = ControlledType->ApplicationStrategy->GetAppliedValue({ControlledKey, this});
				
				// Update the ControllerSetDefaultValue to reflect the new baseline set by the controller
				FString& Default = RegisteredSettings[ControlledKey].ControllerSetDefaultValue;
				const FString OldDefault = Default;
				Default = CurrentValue;
				
				UE_LOGFMT(LogAutoSettings, Verbose, "Updated ControllerSetDefaultValue for controlled setting '{0}': '{1}' -> '{2}' (controller: '{3}')",
					ControlledKey.ToString(), OldDefault, CurrentValue, ParentKey.ToString());
			}
		}
	}
}

bool USettingRegistry::IsSettingTypeRegistered(const USettingType* SettingType) const
{
	return RegisteredSettingTypes.Contains(SettingType);
}

void USettingRegistry::RegisterSettingType(USettingType* SettingType)
{
	if (!ensure(SettingType))
	{
		return;
	}

	if (!ensure(!IsSettingTypeRegistered(SettingType)))
	{
		UE_LOGFMT(LogAutoSettings, Error, "SettingType '{0}' is already registered - this indicates a logic error", 
			   SettingType->GetName());
		return;
	}

	UE_LOGFMT(LogAutoSettings, Log, "Registering SettingType '{0}' with {1} default providers", 
		   SettingType->GetName(), SettingType->DefaultProviders.Num());

	// Mark this SettingType as registered
	RegisteredSettingTypes.Add(SettingType);

	UE_LOGFMT(LogAutoSettings, Log, "Getting default values for SettingType '{0}'", 
		   SettingType->GetName());

	// Get all default values from the SettingType (SettingType handles key combination)
	TMap<FPartialSettingKey, FString> AllDefaults = SettingType->GetDefaultValues();

	// Process each default value from the SettingType
	for (const auto& DefaultPair : AllDefaults)
	{
		const FPartialSettingKey& SettingTypeKey = DefaultPair.Key;
		const FString& Value = DefaultPair.Value;

		// Resolve to full key by prepending registry base key
		FFullSettingKey FullKey = ResolveSettingKey(SettingTypeKey);

		// Register setting if it doesn't exist
		if (!IsSettingRegistered(FullKey))
		{
			UE_LOGFMT(LogAutoSettings, Log, "Registering setting '{0}' from SettingType defaults", 
				   FullKey.ToString());
			
			if (!RegisterSetting(FullKey, SettingType))
			{
				UE_LOGFMT(LogAutoSettings, Warning, "Failed to register setting '{0}' from SettingType defaults", 
					   FullKey.ToString());
				continue;
			}
		}

		// Apply the default value
		if (!SetInitialDefaultValue(FullKey, Value))
		{
			UE_LOGFMT(LogAutoSettings, Warning, "Failed to set default for '{0}'", 
				   FullKey.ToString());
			continue;
		}

		UE_LOGFMT(LogAutoSettings, Verbose, "Set default for '{0}' = '{1}'", 
			   FullKey.ToString(), Value);
	}

	UE_LOGFMT(LogAutoSettings, Log, "SettingType '{0}' generated {1} default settings", 
		   SettingType->GetName(), AllDefaults.Num());
}