// Copyright Epic Games, Inc. All Rights Reserved.
using System.IO;
using UnrealBuildTool;

public class UavSitlTrainer : ModuleRules
{
	public UavSitlTrainer(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		string MavlinkRootPath = Path.Combine(ModuleDirectory, "ThirdParty", "mavlink");
		PublicIncludePaths.AddRange(new string[] {
			ModuleDirectory, Path.Combine(ModuleDirectory, MavlinkRootPath)
		});

		PublicDefinitions.Add("MAVLINK_USE_MESSAGE_INFO=1");
		PublicSystemIncludePaths.Add(MavlinkRootPath);

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "Sockets", "Networking", "MathCore"
			});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
