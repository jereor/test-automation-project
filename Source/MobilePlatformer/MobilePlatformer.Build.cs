// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class MobilePlatformer : ModuleRules
{
	public MobilePlatformer(ReadOnlyTargetRules target) : base(target)
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
	}
}