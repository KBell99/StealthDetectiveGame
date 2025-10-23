// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UAutoSettingsConsoleVariable;

namespace SB::AutoSettings::Console
{
	/**
	 * Get all console variable classes from asset registry
	 * @return Array of console variable classes (excludes abstract classes)
	 */
	TArray<UClass*> GetAllConsoleVariableClasses();

	/**
	 * Get all console variable default objects
	 * @return Array of console variable CDOs
	 */
	TArray<UAutoSettingsConsoleVariable*> GetAllConsoleVariableDefaultObjects();
}