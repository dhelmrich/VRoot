// Copyright Epic Games, Inc. All Rights Reserved.

#include "StaticZMQ.h"
#include "Core.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"
#include "StaticZMQLibrary/zmq.h"

#define LOCTEXT_NAMESPACE "FStaticZMQModule"

void FStaticZMQModule::StartupModule()
{
  // This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

  // Get the base directory of this plugin
  FString BaseDir = IPluginManager::Get().FindPlugin("StaticZMQ")->GetBaseDir();

  // Add on the relative location of the third party dll and load it
  FString LibraryPath;
#if PLATFORM_WINDOWS
  LibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/Win64/libzmq-mt-4_3_3.dll"));
#elif PLATFORM_MAC
  LibraryPath = FPaths::Combine(*BaseDir, TEXT("Source/Win64/libzmq-mt-4_3_3.dylib"));
#endif // PLATFORM_WINDOWS

  ExampleLibraryHandle = !LibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LibraryPath) : nullptr;
  /*
  FMessageDialog::Open(EAppMsgType::Ok,
    FText::FromString(FString("Char: ")
      + FString::FromInt(sizeof(char))
      + FString(", Float: ")
      + FString::FromInt(sizeof(float))));
      */
  if (ExampleLibraryHandle)
  {
    // Call the test function in the third party library that opens a message box
    //FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("ThirdPartyLibraryError", "Loading ZMQ Successfull"));

    FPlatformProcess::GetDllExport(ExampleLibraryHandle, TEXT("Binaries/Win64/libzmq-mt-4_3_3.dll"));




    zmq_has("ipc");
  }
  else
  {
    FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("ThirdPartyLibraryError", "Failed to load ZeroMQ Library"));
  }
}

void FStaticZMQModule::ShutdownModule()
{
  // This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
  // we call this function before unloading the module.

  // Free the dll handle
  FPlatformProcess::FreeDllHandle(ExampleLibraryHandle);
  ExampleLibraryHandle = nullptr;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FStaticZMQModule, StaticZMQ)
