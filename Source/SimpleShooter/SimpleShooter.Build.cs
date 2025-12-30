// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SimpleShooter : ModuleRules
{
	public SimpleShooter(ReadOnlyTargetRules Target) : base(Target)
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
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"SimpleShooter",
			"SimpleShooter/Variant_Shooter",
			"SimpleShooter/Variant_Shooter/AI",
			"SimpleShooter/Variant_Shooter/UI",
			"SimpleShooter/Variant_Shooter/Weapons",
			"SimpleShooter/Variant_Shooter/Character",
			"SimpleShooter/Variant_Shooter/GameModes",
			"SimpleShooter/Variant_Shooter/GameStates",
			"SimpleShooter/Variant_Shooter/PlayerController"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
