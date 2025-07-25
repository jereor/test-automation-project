// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class MobilePlatformer : ModuleRules
{
	public MobilePlatformer(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] 
		{ 
			"Core", 
			"CoreUObject", 
			"Engine",
			"InputCore",
			"Slate",
			"SlateCore",
			"ApplicationCore"
		});

		if (Target.Configuration != UnrealTargetConfiguration.Shipping)
		{
			PublicDependencyModuleNames.AddRange(new string[]
			{
				"AutomationController",
				"AutomationTest",
				"FunctionalTesting",
				"AutomationWindow",
				"UnrealEd",
				"ToolMenus",
				"AutomationMessages"
			});
		}
	}
}