// Copyright Sam Bonifacio. All Rights Reserved.

#include "Utility/SettingKeyFunctions.h"
#include "Framework/SettingType.h"

FSettingKey USettingKeyFunctions::ParseSettingKey(const FString& String)
{
	return FSettingKey(String);
}

FPartialSettingKey USettingKeyFunctions::ParsePartialSettingKey(const FString& String)
{
	FPartialSettingKey Key;
	Key.Fragments = FSettingKey(String).Fragments;
	return Key;
}

FFullSettingKey USettingKeyFunctions::ParseFullSettingKey(const FString& String)
{
	FFullSettingKey Key;
	Key.Fragments = FSettingKey(String).Fragments;
	return Key;
}

FFullSettingKey USettingKeyFunctions::ConvertToFullSettingKey(const FSettingKey& SettingKey)
{
	return FFullSettingKey(SettingKey);
}

FPartialSettingKey USettingKeyFunctions::ConvertToPartialSettingKey(const FSettingKey& SettingKey)
{
	return FPartialSettingKey(SettingKey);
}

FPartialSettingKey USettingKeyFunctions::MakeSettingKey(const TSubclassOf<USettingType>& SettingType, const FPartialSettingKey& SubKey)
{
	// Blueprint version delegates to C++ implementation
	return SB::AutoSettings::SettingKeys::MakeSettingKey(SettingType, SubKey);
}