// Copyright Sam Bonifacio. All Rights Reserved.


#include "Core/InputSettingApplicationStrategy.h"

#include "Core/AutoSettingsEnhancedInputUserSettings.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "Logging/StructuredLog.h"
#include "AutoSettingsInputLogs.h"
#include "Framework/AutoSettingsPlayerSubsystem.h"
#include "Framework/SettingRegistry.h"
#include "Utility/InputMappingUtils.h"

bool UInputSettingApplicationStrategy::CanRegister_Implementation(const FSettingContext& Context)
{
	// At design time, always allow registration so input settings can be shown in the editor
	if (Context.Registry && Context.Registry->bDesignTime)
	{
		return true;
	}

	// Must have local player to be able to register
	const ULocalPlayer* LocalPlayer = GetLocalPlayer(Context);
	if (!LocalPlayer)
	{
		return false;
	}

	// Must have a player controller to be able to register
	const APlayerController* PlayerController = LocalPlayer->GetPlayerController(nullptr);
	if (!IsValid(PlayerController))
	{
		return false;
	}

	// Must have Enhanced Input User Settings to be able to register
	auto UserSettings = GetEnhancedInputUserSettings(Context);
	if (!IsValid(UserSettings))
	{
		return false;
	}
	
	return true;
}

void UInputSettingApplicationStrategy::OnApply_Implementation(const FSetSettingEvent& Event)
{
	const FInputMappingValue MappingValue = ParseInputValue(Event.Data.Value);
	
	auto UserSettings = GetEnhancedInputUserSettings(Event.Setting.Context);
	if (!ensure(UserSettings))
	{
		return;
	}

	const FName MappingName = GetMappingName(Event.Setting);
	const EPlayerMappableKeySlot KeySlot = GetKeySlot(Event.Setting);

	ApplyAxisMetadata(UserSettings, MappingValue, MappingName);

	if (!EnsureMappingContextRegistered(UserSettings, MappingName, Event.Setting.Context))
	{
		return;
	}

	ApplyKeyMapping(UserSettings, MappingValue.Key, MappingName, KeySlot);
}

FString UInputSettingApplicationStrategy::GetAppliedValue_Implementation(const FSettingReference& Setting)
{
	if (Setting.Context.Registry->bDesignTime)
	{
		return GetDesignTimeDefaultKey(Setting);
	}

	const FKey CurrentKey = SB::AutoSettings::Input::Player::GetCurrentKeyForMapping(
		GetLocalPlayer(Setting.Context), GetMappingName(Setting), GetKeySlot(Setting));
	
	if (CurrentKey.IsAxis1D())
	{
		return FormatAxisKeyWithScale(CurrentKey, Setting);
	}
	
	return CurrentKey.ToString();
}

void UInputSettingApplicationStrategy::RegisterAppliedValueModifiedCallback_Implementation(
	const FSettingReference& Setting, const FSettingAppliedValueModifiedSignature& Callback)
{
	if (Setting.Context.Registry->bDesignTime)
	{
		// Skip at design time as there is no local player
		return;
	}

	const auto UserSettings = GetEnhancedInputUserSettings(Setting.Context);
	if (!ensure(UserSettings))
	{
		return;
	}

	// Fire delegate when applied key is remapped
	UserSettings->OnKeyMappingUpdatedEvent.AddWeakLambda(Callback.GetUObject(),
		[this, Setting, Callback](const FPlayerKeyMapping& KeyMapping)
		{
			if (KeyMapping.GetMappingName() == GetMappingName(Setting) && KeyMapping.GetSlot() == GetKeySlot(Setting))
			{
				const bool bExecuted = Callback.ExecuteIfBound(Setting);
			}
		});
}

void UInputSettingApplicationStrategy::RemoveAppliedValueModifiedCallback_Implementation(const UObject* UserObject,
	const FSettingReference& Setting)
{
	if (Setting.Context.Registry->bDesignTime)
	{
		// Skip at design time as there is no local player
		return;
	}

	const auto UserSettings = GetEnhancedInputUserSettings(Setting.Context);
	if (!ensure(UserSettings))
	{
		return;
	}

	UserSettings->OnKeyMappingUpdatedEvent.RemoveAll(UserObject);
}

ULocalPlayer* UInputSettingApplicationStrategy::GetLocalPlayer(const FSettingContext& Context) const
{
	const USettingRegistry* Registry = Context.Registry;
	if (!ensure(Registry))
	{
		return nullptr;
	}
	const UAutoSettingsPlayerSubsystem* Subsystem = Registry->GetTypedOuter<UAutoSettingsPlayerSubsystem>();
	if (!ensure(Subsystem))
	{
		return nullptr;
	}
	return Subsystem->GetLocalPlayer();
}

UEnhancedInputLocalPlayerSubsystem* UInputSettingApplicationStrategy::GetEnhancedInputSubsystem(
	const FSettingContext& Context) const
{
	const SB::AutoSettings::Input::FEnhancedInputSubsystemData Data = SB::AutoSettings::Input::GetEnhancedInputSubsystemData(GetLocalPlayer(Context));
	return Data.Subsystem;
}

UAutoSettingsEnhancedInputUserSettings* UInputSettingApplicationStrategy::GetEnhancedInputUserSettings(
	const FSettingContext& Context) const
{
	return UAutoSettingsEnhancedInputUserSettings::Get(GetLocalPlayer(Context));
}

FName UInputSettingApplicationStrategy::GetMappingName(const FSettingReference& Setting) const
{
	// The second last node is the mapping name
	return Setting.Key.GetLastFragment(1).Name;
}

UInputSettingApplicationStrategy::FInputMappingValue::FInputMappingValue(const FString& InputValue)
{
	FString KeyString = InputValue;
	
	FString Left, Right;
	if (InputValue.Split(TEXT(":"), &Left, &Right))
	{
		KeyString = Left;
		AxisScale = FCString::Atof(*Right);
	}

	Key = FKey(FName(KeyString));
	if (Key == SB::AutoSettings::Input::Unbound)
	{
		Key = EKeys::Invalid;
	}
}

FString UInputSettingApplicationStrategy::FInputMappingValue::ToString() const
{
	if (AxisScale.IsSet() && Key.IsAxis1D())
	{
		const FString ScaleString = FString::Printf(TEXT("%g"), AxisScale.GetValue());
		return FString::Printf(TEXT("%s:%s"), *Key.ToString(), *ScaleString);
	}
	return Key.ToString();
}

UInputSettingApplicationStrategy::FInputMappingValue UInputSettingApplicationStrategy::ParseInputValue(const FString& InputValue) const
{
	return FInputMappingValue(InputValue);
}

void UInputSettingApplicationStrategy::ApplyAxisMetadata(UAutoSettingsEnhancedInputUserSettings* UserSettings, const FInputMappingValue& MappingValue, const FName& MappingName) const
{
	if (!MappingValue.Key.IsValid())
	{
		return;
	}
	
	if (MappingValue.Key.IsAxis1D() && MappingValue.AxisScale.IsSet())
	{
		const bool bNegate = (MappingValue.AxisScale.GetValue() < 0.0f);
		
		FMappingMetadata Metadata;
		Metadata.bNegate = bNegate;
		UserSettings->SetMappingMetadata(MappingName, Metadata);

		UE_LOGFMT(LogAutoSettingsInput, Log, "Stored directional intent for axis mapping {0}: bNegate={1} (AxisScale={2})", 
			MappingName, bNegate, MappingValue.AxisScale.GetValue());
	}
	else
	{
		UserSettings->ClearMappingMetadata(MappingName);
		
		UE_LOGFMT(LogAutoSettingsInput, Log, "Cleared directional intent for mapping {0} (IsAxis1D={1}, HasAxisScale={2})", 
			MappingName, MappingValue.Key.IsAxis1D(), MappingValue.AxisScale.IsSet());
	}
}

bool UInputSettingApplicationStrategy::EnsureMappingContextRegistered(UAutoSettingsEnhancedInputUserSettings* UserSettings, const FName& MappingName, const FSettingContext& Context) const
{
	const ULocalPlayer* LocalPlayer = GetLocalPlayer(Context);
	const UWorld* World = LocalPlayer ? LocalPlayer->GetWorld() : nullptr;
	const auto MappingContext = SB::AutoSettings::Input::PlayerMapping::GetMappingContext(MappingName, World);

	if (!ensure(MappingContext))
	{
		return false;
	}

	if (!UserSettings->IsMappingContextRegistered(MappingContext))
	{
		UserSettings->RegisterInputMappingContext(MappingContext);
	}
	
	return true;
}

void UInputSettingApplicationStrategy::ApplyKeyMapping(UAutoSettingsEnhancedInputUserSettings* UserSettings, const FKey& NewKey, const FName& MappingName, EPlayerMappableKeySlot KeySlot) const
{
	FMapPlayerKeyArgs Args;
	Args.MappingName = MappingName;
	Args.Slot = KeySlot;
	Args.NewKey = NewKey;

	FGameplayTagContainer FailureReason;

	UE_LOGFMT(LogAutoSettingsInput, Log, "Mapping key {0} to {1} slot {2}", NewKey.ToString(), MappingName, static_cast<uint8>(KeySlot));
	UserSettings->MapPlayerKey(Args, FailureReason);

	ensure(FailureReason.IsEmpty());
}

FString UInputSettingApplicationStrategy::GetDesignTimeDefaultKey(const FSettingReference& Setting) const
{
	const FName MappingName = GetMappingName(Setting);
	const EPlayerMappableKeySlot KeySlot = GetKeySlot(Setting);
	
	const FKey DefaultKey = SB::AutoSettings::Input::PlayerMapping::GetDefaultKey(nullptr, MappingName, KeySlot);
	
	if (DefaultKey.IsValid())
	{
		return DefaultKey.ToString();
	}
	
	return {};
}

FString UInputSettingApplicationStrategy::FormatAxisKeyWithScale(const FKey& AxisKey, const FSettingReference& Setting) const
{
	const FName MappingName = GetMappingName(Setting);
	const float AxisScale = SB::AutoSettings::Input::PlayerMapping::GetEffectiveDirectionalScale(
		GetLocalPlayer(Setting.Context), MappingName);
	
	FInputMappingValue Value;
	Value.Key = AxisKey;
	Value.AxisScale = AxisScale;
	
	return Value.ToString();
}

EPlayerMappableKeySlot UInputSettingApplicationStrategy::GetKeySlot(const FSettingReference& Setting) const
{
	// The last node is the key slot
	const FName KeySlotName = Setting.Key.GetLastFragment().Name;
	int32 KeySlot = FCString::Atoi(*KeySlotName.ToString());
	return static_cast<EPlayerMappableKeySlot>(KeySlot);
}