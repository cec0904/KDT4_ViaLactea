// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class ViaLactea : ModuleRules
{
	public ViaLactea(ReadOnlyTargetRules Target) : base(Target)
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
            "OnlineSubsystem",
			"OnlineSubsystemUtils",
            "OnlineSubsystemSteam",
            "Sockets",
			//// 260212KHB
            "NavigationSystem",
            "SlateCore",
            "PhysicsCore",
			"Water",
			"Niagara",
			"AIModule",
			//0305
            "GameplayTasks",
			//0306
			"GameplayTags",
			//0413
			"AnimGraphRuntime",
			/////
			//20260220이동진
			"PoseSearch",
			"MotionTrajectory",
			//////////
			///20260226 권도희
			"SlateCore",
            "EnhancedInput"
			////
        });

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"ViaLactea"
		});



        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
