// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class OnlineSubsystemMeta : ModuleRules
{
	public OnlineSubsystemMeta(ReadOnlyTargetRules Target) : base(Target)
	{
        Definitions.Add("ONLINESUBSYSTEMMETA_PACKAGE=1");

        PublicIncludePaths.AddRange(
			new string[] {
				"OnlineSubsystemMeta/Public"
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
                "OnlineSubsystemMeta/Private",
            }
			);

        PrivateDependencyModuleNames.AddRange(
            new string[] {
                "Core",
                "CoreUObject",
                "Engine",
                "Sockets",
                "OnlineSubsystem",
                "OnlineSubsystemUtils",
                "ServiceWrappers",
                "Json",
                "JsonUtilities",
            }
            );
	}
}
