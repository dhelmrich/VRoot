// Fill out your copyright notice in the Description page of Project Settings.


#include "LeaderBoard.h"

#include <string>

#include "Core.h"
#include "Kismet/GameplayStatics.h"
#include "DesktopPlatformModule.h"
#include "Algo/Count.h"

void ALeaderBoard::OnConstruction(const FTransform& Transform)
{
  Super::OnConstruction(Transform);
  
}

void ALeaderBoard::AddToLeaderBoard(FString name)
{
  if(bFallbackMode)
  {
    if(IsValid(GEngine) && IsValid(GEngine->GameViewport))
    {
      auto OSWindow = GEngine->GameViewport->GetWindow()->GetNativeWindow()->GetOSWindowHandle();
      auto Platform = FDesktopPlatformModule::Get();
      
    }
  }
  Leaderboard.Add({name, LastTime});
  PrintLeaderboard();
  DumpLeaderbord();
}

TArray<FString> ALeaderBoard::GetLeaderboardInOrder()
{
  TArray<FString> BoardOrder;
  
  Leaderboard.Sort([](TTuple<FString,float> a, TTuple<FString,float> b)
  {
    return a.Value < b.Value;
  });

  for(auto Entry : Leaderboard)
  {
    BoardOrder.Add(Entry.Key);
  }
  return BoardOrder;
}

void ALeaderBoard::StartTimer()
{
  LastTime = 0.f;
  TimeAtStart = UGameplayStatics::GetTimeSeconds(GetWorld());
}

void ALeaderBoard::StopTimer()
{
  LastTime = UGameplayStatics::GetTimeSeconds(GetWorld()) - TimeAtStart;
}

void ALeaderBoard::PrintLeaderboard()
{
  Leaderboard.Sort([](TTuple<FString,float> a, TTuple<FString,float> b)
  {
    return a.Value < b.Value;
  });
  FString BoardText = " Leaderboard:\n\n";
  int c = 0;
  for(auto Entry : Leaderboard)
  {
    ++c;
    float SanEntry = static_cast<float>(static_cast<int>(Entry.Value * 100.f)) / 100.f;
    BoardText = BoardText.Append(" ").Append(Entry.Key).Append(": ").Append(FString::SanitizeFloat(SanEntry)).Append("\n\n");
    if(c >= 5)
    {
      break;
    }
  }
  this->Write(BoardText,0,300);
}

void ALeaderBoard::DumpLeaderbord()
{
  if (Leaderboard.Num() == 0 || Algo::CountIf(Leaderboard,[](TPair<FString,float> pair){return !pair.Key.IsEmpty();}) == 0)
    return;
  FString JasonString;
  FString LevelName = GetLevel()->GetOuter()->GetName();
  FString File = FPaths::GeneratedConfigDir() + UGameplayStatics::GetPlatformName() + TEXT("/") + LevelName +  TEXT(".json");
  TSharedPtr<FJsonObject> Jason = MakeShareable(new FJsonObject());
  for(auto Entry : Leaderboard)
  {
    if(Entry.Key.IsEmpty())
      continue;
    Jason->SetNumberField(Entry.Key, Entry.Value);
  }
  if (Jason->Values.Num() == 0)
    return;
  auto Writer = TJsonWriterFactory<TCHAR>::Create(&JasonString,0);
  FJsonSerializer::Serialize(Jason.ToSharedRef(),Writer);
  FFileHelper::SaveStringToFile(JasonString,*File);
}

void ALeaderBoard::Tick(float DeltaTime)
{
}

void ALeaderBoard::BeginPlay()
{
  RegisterRuntime(TEXT(""), 0, 300);
  Leaderboard.Empty();
  FString JasonString;
  FString LevelName = GetLevel()->GetOuter()->GetName();
  FString File = FPaths::GeneratedConfigDir() + UGameplayStatics::GetPlatformName() + TEXT("/") + LevelName +  TEXT(".json");
  UE_LOG(LogTemp,Warning,TEXT("Testing for file: %s"),*File);
  if(FFileHelper::LoadFileToString(JasonString, *File))
  {
    TSharedPtr<FJsonObject> Jason = MakeShareable(new FJsonObject());
	  TSharedRef<TJsonReader<TCHAR>> Reader = FJsonStringReader::Create(JasonString);
	  FJsonSerializer::Deserialize(Reader, Jason);
    for(auto Entry : Jason->Values)
    {
      float Number;
      if(Entry.Value->TryGetNumber(Number))
      {
        Leaderboard.Add({Entry.Key,Number});
      }
    }
    PrintLeaderboard();
  }
  else
  {
    Write("Leaderboard:", 0, 300);
  }
}

void ALeaderBoard::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  Super::EndPlay(EndPlayReason);
  DumpLeaderbord();
}
