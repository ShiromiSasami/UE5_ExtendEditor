// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SuperManager : ModuleRules
{
	public SuperManager(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
               System.IO.Path.GetFullPath(Target.RelativeEnginePath) + "/Source/Editor/Blutility/Private",
            }
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "Blutility",
                "EditorScriptingUtilities",
                "UMG",
				"Niagara",
				"UnrealEd",
                "AssetRegistry",
				"AssetTools",
                "ContentBrowser",
                "InputCore",
                "Projects",
                "SceneOutliner",
                "LevelEditor"
            }
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
