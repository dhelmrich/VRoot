// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using System.Runtime;
using UnrealBuildTool;

public class RootMRICNR : ModuleRules
{
    private string ModulePath
    {
        get { return ModuleDirectory; }
    }
    public RootMRICNR(ReadOnlyTargetRules Target) : base(Target)
	  {
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] {
                "Core",
                "CoreUObject",
                "Engine",
                //"UnrealEd",
                "HeadMountedDisplay",
                "InputCore",
                "StaticZMQ",
                "RHI",
                "RenderCore",
                "ProceduralMeshComponent",
                "Json",
                "DesktopPlatform",
                "UMG",
                "Slate",
                "SlateCore"
        });

		PrivateDependencyModuleNames.AddRange(new string[] {
                //"Slate",
                //"SlateCore",
                "ProceduralMeshComponent",
                "StaticZMQ" });

    }

    public bool Load0MQ(ReadOnlyTargetRules Target)
    {
        bool bSupported = false;
        string fullpath = ModuleDirectory.ToString();
        fullpath = fullpath.Substring(0,fullpath.LastIndexOf("\\"));
        string path = fullpath.Substring(0, fullpath.LastIndexOf("\\"));
        //bool bDebug = (Target.Configuration == UnrealTargetConfiguration.Debug && BuildConfiguration.bDebugBuildsActuallyUseDebugCRT);
        // TODO done add project name as start for correct behavior
//         PublicIncludePaths.AddRange(new string[] {
//             Path.Combine(path,"Plugins\\libzmq\\src")
//         });
//         //  PublicAdditionalLibraries.Add(Path.Combine(ModulePath, "Boost/lib/libboost_chrono-vc141-mt-1_64.lib"));
//         // libzmq-v142-mt-s-4_3_4
//         if (Target.Platform == UnrealTargetPlatform.Win64)
//         {
//             PublicAdditionalLibraries.Add(Path.Combine(path, "Plugins\\libzmq\\build_vs16x64\\lib\\Release\\libzmq-v142-mt-4_3_4.lib"));
//             PublicAdditionalLibraries.Add(Path.Combine(path, "Plugins\\libzmq\\build_vs16x64\\lib\\Release\\libzmq-v142-mt-s-4_3_4.lib"));
//         }
        bSupported = true;
        return bSupported;
    }
}
