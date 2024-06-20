// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;
using System;

public class StaticZMQ : ModuleRules
{
	public StaticZMQ(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"StaticZMQLibrary",
				"Projects"
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

        string fullpath = ModuleDirectory.ToString();
        fullpath = fullpath.Substring(0, fullpath.LastIndexOf("\\"));
        string path = fullpath.Substring(0, fullpath.LastIndexOf("\\"));
		PublicAdditionalLibraries.Add(Path.Combine(path, "Binaries", "Win64", "libzmq-mt-4_3_3.lib"));
		PublicDelayLoadDLLs.Add("libzmq-mt-4_3_3.dll");
	}
}
