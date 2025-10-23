// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "SettingRegistry.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AutoSettingsSubsystem.generated.h"

class USettingRegistry;

/**
 * Game Instance Subsystem that manages the AutoSettings plugin's core functionality.
 * 
 * This subsystem is responsible for:
 * - Initializing and managing the game's setting registry
 * - Acting as the main entry point for accessing AutoSettings functionality
 * - Coordinating with other AutoSettings subsystems
 * 
 * The subsystem automatically initializes when the game instance is created and
 * provides access to the global setting registry for Blueprint and C++ code.
 */
UCLASS()
class AUTOSETTINGS_API UAutoSettingsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UAutoSettingsSubsystem();


	UFUNCTION(BlueprintPure, Category = "Settings")
	USettingRegistry* GetGameSettingRegistry() const
	{
		return GameSettingRegistry;
	}


protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

private:
	/** The global setting registry that manages all game-wide settings */
	UPROPERTY()
	USettingRegistry* GameSettingRegistry;
};