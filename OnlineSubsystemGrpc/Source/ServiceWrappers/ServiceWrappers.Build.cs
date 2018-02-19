// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ServiceWrappers : ModuleRules
{
	public ServiceWrappers(ReadOnlyTargetRules Target) : base(Target)
	{
        bEnableExceptions = true;

        PublicIncludePaths.AddRange(
			new string[] {
				"ServiceWrappers/Public"
			}
			);
				
		PrivateIncludePaths.AddRange(
			new string[] {
                "ServiceWrappers/Private",
                "ServiceWrappers/Private/GeneratedProto"
            }
			);

        PrivateDependencyModuleNames.AddRange(
            new string[] {
                "Core",
                "Engine",
                "OnlineSubsystem",
                "protobuf",
                "grpc",
                "Json",
                "JsonUtilities",
            }
            );
	}
}
