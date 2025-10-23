// Copyright Sam Bonifacio. All Rights Reserved.


#include "Framework/SettingApplicationStrategy.h"
#include "AutoSettingsLogs.h"
#include "Framework/SettingRegistry.h"
#include "Framework/SettingTypes.h"

bool USettingApplicationStrategy::CanRegister_Implementation(const FSettingContext& Context)
{
	return true;
}

void USettingApplicationStrategy::Apply(const FSetSettingEvent& Event)
{
	if (!ValidateConfiguration(Event.Setting))
	{
		return;
	}

	if (!ensure(Event.GetRegistry()))
	{
		return;
	}

	const FString Previous = GetAppliedValue(Event.Setting);
	UE_LOG(LogAutoSettings, Log, TEXT("Applying setting %s with value: '%s', previous: '%s'"), *Event.Setting.Key.ToString(),
		*Event.Data.Value, *Previous);

	// Skip actual application at design time to prevent side effects, but allow reading
	if (!Event.GetRegistry()->bDesignTime)
	{
		OnApply(Event);
	}

	Event.GetRegistry()->OnSettingApplied.Broadcast(Event);
}

void USettingApplicationStrategy::RemoveAppliedValueModifiedCallback_Implementation(const UObject* UserObject,
	const FSettingReference& Setting) {}

FString USettingApplicationStrategy::GetAppliedValue_Implementation(const FSettingReference& Setting)
{
	return TEXT("");
}

bool USettingApplicationStrategy::ValidateConfiguration_Implementation(const FSettingReference& Setting)
{
	return true;
}

void USettingApplicationStrategy::RegisterAppliedValueModifiedCallback_Implementation(
	const FSettingReference& Setting,
	const FSettingAppliedValueModifiedSignature& Callback) {}

void USettingApplicationStrategy::OnApply_Implementation(const FSetSettingEvent& Event) {}