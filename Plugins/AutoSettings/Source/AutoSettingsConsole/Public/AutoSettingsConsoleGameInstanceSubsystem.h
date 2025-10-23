// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AutoSettingsConsoleGameInstanceSubsystem.generated.h"

class UAutoSettingsConsoleVariable;

/**
 * Subsystem responsible for handling Blueprint-exposed Console Variable classes at a GameInstance level
 */
UCLASS()
class AUTOSETTINGSCONSOLE_API UAutoSettingsConsoleGameInstanceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	void RegisterConsoleVariableClasses();

private:

	UPROPERTY()
	TArray<UAutoSettingsConsoleVariable*> ConsoleVariableObjects;

	UPROPERTY()
	bool bHasRegisteredConsoleVariableClasses = false;
};