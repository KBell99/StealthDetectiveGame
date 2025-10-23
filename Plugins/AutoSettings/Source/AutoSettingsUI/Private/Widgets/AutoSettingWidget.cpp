// Copyright Sam Bonifacio. All Rights Reserved.

#include "Widgets/AutoSettingWidget.h"
#include "AutoSettingsError.h"
#include "AutoSettingsLogs.h"
#include "Components/NamedSlot.h"
#include "Engine/GameInstance.h"
#include "Logging/StructuredLog.h"
#include "Framework/AutoSettingsPlayerSubsystem.h"
#include "Framework/AutoSettingsSubsystem.h"
#include "Framework/SettingRegistry.h"
#include "Framework/SettingApplicationStrategy.h"

UAutoSettingWidget::UAutoSettingWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bAutoSave(true)
	, bAutoApply(true)
	, bUpdatingSettingSelection(false)
{
	bIsVariable = false;
	SetIsFocusable(true);
}

UAutoSettingWidget* UAutoSettingWidget::FindInNamedSlot(const UNamedSlot* NamedSlot)
{
	// Check direct children
	for (UWidget* Child : NamedSlot->GetAllChildren())
	{
		if (UAutoSettingWidget* SettingWidget = Cast<UAutoSettingWidget>(Child))
		{
			return SettingWidget;
		}
	}

	// Check further named slots recursively if nested
	for (const UWidget* Child : NamedSlot->GetAllChildren())
	{
		if (const UNamedSlot* ChildNamedSlot = Cast<UNamedSlot>(Child))
		{
			if (UAutoSettingWidget* ChildResult = FindInNamedSlot(ChildNamedSlot))
			{
				return ChildResult;
			}
		}
	}

	// No result
	return nullptr;
}

const USettingType* UAutoSettingWidget::GetSettingType() const
{
	return SettingType.GetDefaultObject();
}

void UAutoSettingWidget::SetSetting(TSubclassOf<USettingType> InSettingType, FPartialSettingKey InSubKey)
{
	if (!InSettingType)
	{
		if (!IsDesignTime())
		{
			LogMissingSetting();
		}
		return;
	}

	if (!IsDesignTime())
	{
		if (auto OldSettingType = GetSettingType())
		{
			// Unbind the applied value modified callback for the previous setting
			if (auto ApplicationStrategy = OldSettingType->ApplicationStrategy)
			{
				ApplicationStrategy->RemoveAppliedValueModifiedCallback(this, GetSettingReference());
			}
		}
	}

	SettingType = InSettingType;
	SubKey = InSubKey;

	// If the setting isn't registered, do so
	if (!GetRegistry()->IsSettingRegistered(GetFullSettingKey()))
	{
		GetRegistry()->RegisterSetting(GetFullSettingKey(), GetSettingType());
	}

	ResetToSettingValue();

	if (!IsDesignTime())
	{
		if (auto ApplicationStrategy = GetSettingType()->ApplicationStrategy)
		{
			// Register delegate fired when applied value is updated
			FSettingAppliedValueModifiedSignature Delegate;
			Delegate.BindDynamic(this, &ThisClass::OnAppliedValueModified);
			ApplicationStrategy->RegisterAppliedValueModifiedCallback(GetSettingReference(), Delegate);
		}
	}
}

void UAutoSettingWidget::Apply()
{
	GetRegistry()->SetSetting(GetFullSettingKey(), CurrentValue, ESettingAction::ApplyOnly);
	bHasUnappliedChange = false;
}

FString UAutoSettingWidget::GetDefaultValue() const
{
	return GetRegistry()->GetCurrentDefaultValue(GetFullSettingKey());
}

bool UAutoSettingWidget::CanResetToDefault() const
{
	return CurrentValue != GetDefaultValue();
}

void UAutoSettingWidget::ResetToDefault()
{
	if (!CanResetToDefault())
	{
		return;
	}

	const FString DefaultValue = GetDefaultValue();
	DispatchUpdateSelection(DefaultValue);
	ApplySettingValue(DefaultValue, true);
}

void UAutoSettingWidget::Save()
{
	// Use the new simplified API - this automatically applies if not already applied
	GetRegistry()->SetSetting(GetFullSettingKey(), CurrentValue, ESettingAction::ApplyAndSave);

	bHasUnappliedChange = false;
	bHasUnsavedChange = false;
}

void UAutoSettingWidget::Cancel()
{
	// We want to reset to the config value rather than the current CVar value in case we are reverting changes that were already applied
	ResetToSettingValue(true);

	const FString SavedValue = GetRegistry()->GetSettingValue(GetFullSettingKey(), ESettingValuePreference::Saved);
	GetRegistry()->SetSetting(GetFullSettingKey(), SavedValue, ESettingAction::ApplyOnly);

	bHasUnappliedChange = false;
	bHasUnsavedChange = false;
}

FString UAutoSettingWidget::GetDebugName() const
{
	FString DebugName = GetName();
	if (GetSettingType())
	{
		DebugName += "[" + GetFullSettingKey().ToString() + "]";
	}
	return DebugName;
}

void UAutoSettingWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	// Wait for the registry to initialize
	if (auto Registry = GetRegistry(false))
	{
		Registry->CallOrRegister_OnInitialized(FSimpleDelegate::CreateWeakLambda(this, [this]()
		{
			// At this point the registry is initialized

			SetSetting(SettingType, SubKey);
		}));
	}
}

void UAutoSettingWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UAutoSettingWidget::NativeDestruct()
{
	if (!IsDesignTime())
	{
		// If the registry is valid and initialized (it won't be if the session is ending)
		// Remove the applied value modified callback, otherwise the widget will still get callback events
		auto Registry = GetRegistry(false);
		if (Registry && Registry->IsInitialized())
		{
			GetSettingType()->ApplicationStrategy->RemoveAppliedValueModifiedCallback(this, GetSettingReference());
		}
	}

	Super::NativeDestruct();
}


void UAutoSettingWidget::UpdateSelection_Implementation(const FString& Value) {}

void UAutoSettingWidget::ApplySettingValue(FString Value, bool bSaveIfPossible)
{
	// Don't try to apply or save in design time
	// Some widgets can potentially get here through PreConstruct
	if (IsDesignTime())
	{
		return;
	}

	if (!SettingType)
	{
		LogMissingSetting();
		return;
	}

	// The widget selection is being updated by this widget - don't treat it as a user-triggered change
	if (IsUpdatingSettingSelection())
	{
		return;
	}

	if (CurrentValue != Value)
	{
		CurrentValue = Value;
	}

	// Apply / save - regardless of previous setting value
	// Currently applied or saved value may be different anyway

	const bool bShouldSave = bAutoSave && bSaveIfPossible;

	// Determine the action based on widget settings
	ESettingAction Action;
	if (bAutoApply || bShouldSave)
	{
		Action = bShouldSave ? ESettingAction::ApplyAndSave : ESettingAction::ApplyOnly;
	}
	else
	{
		// Don't apply automatically, just track that we have an unapplied change
		bHasUnappliedChange = true;
		return;
	}

	// Use the new simplified API
	GetRegistry()->SetSetting(GetFullSettingKey(), CurrentValue, Action);

	// Update widget state
	bHasUnappliedChange = false;
	bHasUnsavedChange = (Action == ESettingAction::ApplyOnly);
}

FFullSettingKey UAutoSettingWidget::GetFullSettingKey() const
{
	const USettingType* Asset = GetSettingType();
	if (!ensure(Asset))
	{
		return {};
	}

	FFullSettingKey FullKey = GetRegistry()->GetSettingKey(Asset);
	FullKey += SubKey;
	return FullKey;
}

USettingRegistry* UAutoSettingWidget::GetRegistry(bool bChecked) const
{
	USettingRegistry* Registry = nullptr;

	// Use domain of the Setting Type, or fall back to Global
	ESettingDomain SettingDomain = ESettingDomain::Global;
	if (const auto Setting = GetSettingType())
	{
		SettingDomain = Setting->SettingDomain;
	}
	
	if (IsDesignTime())
	{
#if WITH_EDITOR
		// If design time, request an appropriate registry from the editor module
		if (GetDesignTimeRegistry.IsBound())
		{
			Registry = GetDesignTimeRegistry.Execute(SettingDomain);
		}

		if (!ensure(Registry))
		{
			UE_LOGFMT(LogAutoSettings, Error, "Failed to find design-time registry");
		}
#endif
	}
	else
	{
		if (SettingDomain == ESettingDomain::Player)
		{
			const ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
			if (bChecked)
			{
				check(LocalPlayer);
			}
			if (LocalPlayer)
			{
				auto Subsystem = LocalPlayer->GetSubsystem<UAutoSettingsPlayerSubsystem>();
				if (bChecked)
				{
					check(Subsystem);
				}
				if (Subsystem)
				{
					Registry = Subsystem->GetPlayerSettingRegistry();
				}
			}
		}
		else
		{
			const UGameInstance* GameInstance = GetGameInstance();
			if (bChecked)
			{
				check(GameInstance);
			}
			if (GameInstance)
			{
				Registry = GameInstance->GetSubsystem<UAutoSettingsSubsystem>()->GetGameSettingRegistry();
			}
		}
	}

	if (bChecked)
	{
		check(Registry);
	}

	return Registry;
}

void UAutoSettingWidget::PostLoad()
{
	Super::PostLoad();

	if (!SettingKey_DEPRECATED.IsNone())
	{
		SettingKeyExtension_DEPRECATED.Fragments.Add(SettingKey_DEPRECATED);
		SettingKey_DEPRECATED = NAME_None;
	}

	SettingKeyExtension_DEPRECATED.MigrateDeprecatedProperties();
	if (SettingKeyExtension_DEPRECATED.IsValid())
	{
		SubKey.Fragments = SettingKeyExtension_DEPRECATED.Fragments;
		SettingKeyExtension_DEPRECATED = {};
	}
}

void UAutoSettingWidget::PostInitProperties()
{
	Super::PostInitProperties();

	if (!SettingKey_DEPRECATED.IsNone())
	{
		SettingKeyExtension_DEPRECATED.Fragments.Add(SettingKey_DEPRECATED);
		SettingKey_DEPRECATED = NAME_None;
	}

	SettingKeyExtension_DEPRECATED.MigrateDeprecatedProperties();
	if (SettingKeyExtension_DEPRECATED.IsValid())
	{
		SubKey.Fragments = SettingKeyExtension_DEPRECATED.Fragments;
		SettingKeyExtension_DEPRECATED = {};
	}
}


void UAutoSettingWidget::ResetToSettingValue(bool bPreferSavedValue)
{
	const ESettingValuePreference Preference = bPreferSavedValue ? ESettingValuePreference::Saved : ESettingValuePreference::Applied;
	CurrentValue = GetRegistry()->GetSettingValue(GetFullSettingKey(), Preference);
	DispatchUpdateSelection(CurrentValue);
}

void UAutoSettingWidget::DispatchUpdateSelection(const FString& Value)
{
	bUpdatingSettingSelection = true;
	UpdateSelection(Value);
	bUpdatingSettingSelection = false;
}

void UAutoSettingWidget::OnAppliedValueModified(const FSettingReference& Setting)
{
	if (!ensure(Setting.Key == GetFullSettingKey()))
	{
		return;
	}

	if (HasUnappliedChange())
	{
		// If we have an unapplied change, the setting widget was purposefully desynchronized from the CVar value and we should ignore CVar changes
		return;
	}

	auto Registry = GetRegistry(false);
	if (!IsValid(Registry) || !Registry->IsInitialized())
	{
		// If we don't have an initialized registry, the game may be shutting down
		// Ignore the setting change in this case
		return;
	}

	UE_LOGFMT(LogAutoSettings, Log, "Widget '{0}' detected setting changed: {1}", GetNameSafe(this), Setting.Key.ToString());

	// The applied value changed externally so update the widget to reflect the new value
	const FString NewValue = GetSettingType()->ApplicationStrategy->GetAppliedValue(GetSettingReference());
	if (NewValue != CurrentValue)
	{
		CurrentValue = NewValue;
		DispatchUpdateSelection(CurrentValue);
	}
}

void UAutoSettingWidget::LogMissingSetting() const
{
	SB::AutoSettings::Errors::LogError(FString::Printf(TEXT("%s: Requires a valid Setting asset"), *GetDebugName()));
}

FSettingReference UAutoSettingWidget::GetSettingReference() const
{
	return { GetFullSettingKey(), GetSettingContext() };
}

FSettingContext UAutoSettingWidget::GetSettingContext() const
{
	USettingRegistry* Registry = GetRegistry();
	// Registry should always be initialized before using it as a context
	ensure(Registry->IsInitialized());
	return { Registry };
}