// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class AFO : ModuleRules
{
	public AFO(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] 
		{ 
			"Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" , "UMG", "Slate", "SlateCore", "Niagara","HTTP"
        });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

	}
}
