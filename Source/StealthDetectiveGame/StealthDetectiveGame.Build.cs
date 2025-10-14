// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class StealthDetectiveGame : ModuleRules
{
	public StealthDetectiveGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"GameplayTags",
			"GameplayAbilities"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"StealthDetectiveGame",
			"StealthDetectiveGame/Public/Variant_Combat",
			"StealthDetectiveGame/Public/Variant_Combat/AI",
			"StealthDetectiveGame/Public/Variant_Combat/Animation",
			"StealthDetectiveGame/Public/Variant_Combat/Gameplay",
			"StealthDetectiveGame/Public/Variant_Combat/Interfaces",
			"StealthDetectiveGame/Public/Variant_Combat/UI",
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
