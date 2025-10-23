// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class USettingType;

/**
 * Static utility functions for working with setting types.
 */
class AUTOSETTINGS_API FSettingUtils
{
public:
	static TArray<USettingType*> GetAllSettingTypes();
};