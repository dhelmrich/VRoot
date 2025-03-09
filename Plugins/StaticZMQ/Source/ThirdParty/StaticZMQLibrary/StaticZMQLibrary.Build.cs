// Fill out your copyright notice in the Description page of Project Settings.

using System.IO;
using UnrealBuildTool;

public class StaticZMQLibrary : ModuleRules
{
	public StaticZMQLibrary(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			// Add the import library
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "lib", "libzmq-mt-4_3_3.lib"));

			// Delay-load the DLL, so we can load it from the right place first
			PublicDelayLoadDLLs.Add("libzmq-mt-4_3_3.dll");

			// Ensure that the DLL is staged along with the executable
			RuntimeDependencies.Add("$(PluginDir)/Binaries/ThirdParty/StaticZMQLibrary/Win64/libzmq-mt-4_3_3.dll");

			// Add the library directory to include paths
			PublicIncludePaths.Add("$(PluginDir)/Source/ThirdParty/StaticZMQLibrary/include");
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            PublicDelayLoadDLLs.Add(Path.Combine(ModuleDirectory, "Mac", "Release", "libzmq-mt-4_3_3.dylib"));
            RuntimeDependencies.Add("$(PluginDir)/Source/ThirdParty/StaticZMQLibrary/Mac/Release/libzmq-mt-4_3_3.dylib");
        }
		else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "lib", "libzmq.so"));
        }
		else if (Target.Platform == UnrealTargetPlatform.Android)
		{
            // add definition to C++ preprocessor
            PublicDefinitions.Add("NO_ZMQ_STATIC_DEFINE");
        }
	}
}
