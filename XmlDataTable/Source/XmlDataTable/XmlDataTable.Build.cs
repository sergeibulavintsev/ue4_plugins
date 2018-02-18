// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class XmlDataTable : ModuleRules
{
	public XmlDataTable(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				"XmlDataTable/Public"
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				"XmlDataTable/Private",
				"XmlDataTable/Private/Factory",
                "XmlDataTable/Private/AssetActions",
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
                "Engine",
                "InputCore",
                "Core",
            }
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
                "ApplicationCore",
                "ContentBrowser",
                "CoreUObject",
                "DesktopWidgets",
                "EditorStyle",
                "Engine",
                "InputCore",
                "Projects",
                "Slate",
                "SlateCore",
                "UnrealEd",
                "XmlParser",
            }
			);

        PrivateIncludePathModuleNames.AddRange(
            new string[]
            {
                "MainFrame",
            }
            );

        DynamicallyLoadedModuleNames.AddRange(
            new string[] {
                "AssetTools",
                "MainFrame",
            }
            );
    }
}
