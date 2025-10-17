﻿// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class NBTSystem : ModuleRules {
    public NBTSystem(ReadOnlyTargetRules Target) : base(Target) {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[] {
                "Core",
            }
        );
        
        PrivateDependencyModuleNames.AddRange(
            new string[] {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore", 
                "NetCore",
                "AngelscriptCode",
            }
        );
    }
}