// Copyright Sam Bonifacio. All Rights Reserved.

#include "Framework/SettingType.h"

#include "Misc/ConsoleVariableSettingApplicationStrategy.h"
#include "Framework/SettingDefaultProvider.h"
#include "AutoSettingsLogs.h"
#include "Logging/StructuredLog.h"

USettingType::USettingType(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ApplicationStrategy = ObjectInitializer.CreateDefaultSubobject<UConsoleVariableSettingApplicationStrategy>(ObjectInitializer.GetObj(),
		TEXT("ApplicationStrategy"));
	PersistenceStrategy = ObjectInitializer.CreateDefaultSubobject<UConfigSettingPersistenceStrategy>(ObjectInitializer.GetObj(),
		TEXT("StorageStrategy"));
}

FString USettingType::GetValue(const FSettingReference& Setting, bool bPreferSavedValue) const
{
	FString SavedValue = PersistenceStrategy->GetSavedValue(Setting).Value;
	FString AppliedValue = ApplicationStrategy->GetAppliedValue(Setting);

	FString Value = bPreferSavedValue ? SavedValue : AppliedValue;

	if (Value.IsEmpty())
	{
		Value = bPreferSavedValue ? AppliedValue : SavedValue;
	}

	return Value;
}

FPrimaryAssetId USettingType::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(SettingPrimaryAssetTypeName, GetFName());
}

void USettingType::PostLoad()
{
	UObject::PostLoad();

	if (!SettingKey_DEPRECATED.IsNone())
	{
		KeyFragment.Name = SettingKey_DEPRECATED;
		SettingKey_DEPRECATED = {};
	}

	if (!AllowedChildKeys_DEPRECATED.IsEmpty())
	{
		for (FName ChildKey : AllowedChildKeys_DEPRECATED)
		{
			AllowedChildKeyFragments.Add(ChildKey);
		}
		AllowedChildKeys_DEPRECATED.Empty();
	}

	if (!Dependencies_DEPRECATED.IsEmpty())
	{
		ControlledSettingKeys = Dependencies_DEPRECATED;
		Dependencies_DEPRECATED.Empty();
	}
}

void USettingType::Apply(const FSetSettingEvent& Event) const
{
	if (!bEnabled)
	{
		return;
	}

	if (!ApplicationStrategy)
	{
		return;
	}

	ApplicationStrategy->Apply(Event);
}

void USettingType::Save(const FSetSettingEvent& Event) const
{
	if (!bEnabled)
	{
		return;
	}

	if (!PersistenceStrategy)
	{
		return;
	}

	PersistenceStrategy->Save(Event);
}

FPartialSettingKey USettingType::GetPartialSettingKey() const
{
	FPartialSettingKey Key;
	
	// Build the key from the setting type hierarchy
	UClass* CurrentClass = GetClass();
	TArray<FSettingKeyFragment> Fragments;
	
	// Walk up the inheritance chain to build the key
	while (CurrentClass && CurrentClass->IsChildOf<USettingType>() && CurrentClass != USettingType::StaticClass())
	{
		const USettingType* CurrentSetting = GetDefault<USettingType>(CurrentClass);
		if (!CurrentSetting->KeyFragment.IsNone())
		{
			Fragments.Insert(CurrentSetting->KeyFragment, 0);
		}
		
		// Move to parent class
		UClass* ParentClass = CurrentClass->GetSuperClass();
		if (ParentClass->IsChildOf<USettingType>() && ParentClass != USettingType::StaticClass())
		{
			CurrentClass = ParentClass;
		}
		else
		{
			break;
		}
	}
	
	// Add the fragments to the key
	for (const FSettingKeyFragment& Fragment : Fragments)
	{
		Key += Fragment;
	}
	
	return Key;
}

TMap<FPartialSettingKey, FString> USettingType::GetDefaultValues() const
{
	TMap<FPartialSettingKey, FString> Defaults;
	
	// Get this SettingType's base key (without registry base key)
	FPartialSettingKey BaseKey = GetPartialSettingKey();
	
	// 1. Start with static default value if configured
	if (bOverrideDefaultValue && !DefaultValue.IsEmpty())
	{
		// Use the SettingType's base key for the static default
		Defaults.Add(BaseKey, DefaultValue);
		UE_LOGFMT(LogAutoSettings, Verbose, "SettingType '{0}': Added static default value for key '{1}' = '{2}'", 
			   GetName(), BaseKey.ToString(), DefaultValue);
	}
	
	// 2. Execute dynamic providers (hardware benchmarking, etc.)
	// Later providers can override earlier ones by using the same complete key
	for (USettingDefaultProvider* Provider : DefaultProviders)
	{
		if (!IsValid(Provider) || !Provider->bEnabled)
		{
			continue;
		}
		
		UE_LOGFMT(LogAutoSettings, Log, "SettingType '{0}': Executing provider '{1}'", 
			   GetName(), Provider->GetClass()->GetName());
		
		TMap<FPartialSettingKey, FString> ProviderDefaults = Provider->GetDefaultValues();
		
		// Merge provider results, combining each fragment with this SettingType's base key
		for (const auto& ProviderPair : ProviderDefaults)
		{
			const FPartialSettingKey& ProviderFragment = ProviderPair.Key;
			const FString& Value = ProviderPair.Value;
			
			// Combine this SettingType's base key with the provider's fragment
			FPartialSettingKey CompleteKey = BaseKey;
			CompleteKey += ProviderFragment;
			
			if (Defaults.Contains(CompleteKey))
			{
				UE_LOGFMT(LogAutoSettings, Verbose, "SettingType '{0}': Provider '{1}' overriding existing default for key '{2}': '{3}' -> '{4}'", 
					   GetName(), Provider->GetClass()->GetName(), CompleteKey.ToString(), Defaults[CompleteKey], Value);
			}
			else
			{
				UE_LOGFMT(LogAutoSettings, Verbose, "SettingType '{0}': Provider '{1}' setting default for key '{2}' = '{3}'", 
					   GetName(), Provider->GetClass()->GetName(), CompleteKey.ToString(), Value);
			}
			
			Defaults.Add(CompleteKey, Value);
		}
		
		UE_LOGFMT(LogAutoSettings, Log, "SettingType '{0}': Provider '{1}' generated {2} defaults", 
			   GetName(), Provider->GetClass()->GetName(), ProviderDefaults.Num());
	}
	
	UE_LOGFMT(LogAutoSettings, Log, "SettingType '{0}': Total default values: {1}", 
		   GetName(), Defaults.Num());
	
	return Defaults;
}