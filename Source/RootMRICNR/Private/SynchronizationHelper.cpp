// Fill out your copyright notice in the Description page of Project Settings.

#include "SynchronizationHelper.h"
#include "FileTargetBox.h"
#include "DrawSpace.h"
#include "SliderWidget.h"
#include "HAL/Thread.h"
#include "Root.h"
#include <Kismet/GameplayStatics.h>
#include "../RootModellingPawn.h"
#include "OverlapTransform.h"

// Sets default values
// ASynchronizationHelper::ASynchronizationHelper(const FObjectInitializer& ObjectInitializer) 
//       : Super(ObjectInitializer)
ASynchronizationHelper::ASynchronizationHelper()
  : Super()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
  //LoadConfig();
	//ServerConnect = CreateDefaultSubobject<UZMQHandle>(TEXT("Server Connect"));
  
// 	ServerConnect->AssignStateActor(this);
//   ServerConnect->SetupAttachment(this->GetRootComponent());
}

// Called when the game starts or when spawned
void ASynchronizationHelper::BeginPlay()
{
  Super::BeginPlay();
  ServerConnect = NewObject<UZMQHandle>(this,TEXT("Server Connect"));
  ServerConnect->Port = this->Port;
  ServerConnect->ServerName = this->ServerName;
  ServerConnect->PreInitPreparation();
  ServerConnect->AssignStateActor(this);
  ServerConnect->AttachToComponent(this->GetRootComponent(),FAttachmentTransformRules::SnapToTargetIncludingScale);
  ServerConnect->RegisterComponent();

  if (bPerformanceMeasurement)
  {
    APlayerController* PController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PController)
    {
      if (GEngine && false)
      {
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, TEXT("Triggered Player controlle"));
      }
      PController->ConsoleCommand(TEXT("csvprofile start"), true);
    }
    bCsvActive = true;
    RootRandomDelegate.BindLambda([this]()
      {
        if(CurrentAmount <= iHowMany)
        {
          this->ServerConnect->RequestRandomRoot(CurrentAmount);
          this->CurrentAmount = this->CurrentAmount + 50;
          if (GEngine && false)
          {
            GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, FString::Printf(TEXT("Amount %d"),CurrentAmount));
          }
        }
        else
        {
          if (bCsvActive)
          {
//             FString cmd = "csvprofile stop";
//             GetWorld()->Exec(GetWorld(), *cmd);

            APlayerController* PController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
            if (PController)
            {
              PController->ConsoleCommand(TEXT("csvprofile stop"), true);
            }
            bCsvActive = false;
          }
        }
      });
    GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, RootRandomDelegate, 1, false, fWaitBetween);
  }
}

void ASynchronizationHelper::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  APlayerController* PController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
  if (PController)
  {
    PController->ConsoleCommand(TEXT("csvprofile stop"), true);
  }
  Super::EndPlay(EndPlayReason);
  GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
}

void ASynchronizationHelper::MakeFileNameTile(FString FileName, unsigned int ID)
{
	FTransform SpawnTransform; 
	AFileTargetBox* NewBox = GetWorld()->SpawnActor<AFileTargetBox>(AFileTargetBox::StaticClass(), SpawnTransform);
	BoxSet[ID] = NewBox;
	NewBox->FileID = ID;
	NewBox->RegisterRuntime(FileName,ID, 450);
}

void ASynchronizationHelper::SendAfterActionTaken()
{
  TArray<URoot*> data = RootSink->ParseSegmentsIntoRootsystem(false);
  ServerConnect->SendRootSystem(data);
}

void ASynchronizationHelper::SetSliderWidget(ASliderWidget* InIsoSlider)
{
  IsoSlider = InIsoSlider;
  IsoSlider->SetReleaseCallback([this](const FTransform& OriginPosition){
    float distance = (this->IsoSlider->OriginPosition.GetLocation() - this->IsoSlider->GetActorLocation()).Size();
    // I KNOW THIS IS A HACK I WILL CHANGE
    // SO SELF REF TODO MAKE THE COMPONENT INITIALIZED VIA CODE INSTEAD OF HARDCODED
    distance = (((this->IsoSlider->GetActorLocation().X + 110.f) / 440.f) * (this->IsoMax - this->IsoMin) + this->IsoMin)*-1.f;
    this->IsoSurfaceParam = distance;
    IsoCurSign->Write(FString::Printf(TEXT("Now: %f"), IsoSurfaceParam), -1);
    ServerConnect->FetchNewIsosurface(IsoSurfaceParam);
  });
}

void ASynchronizationHelper::StopZMQThread()
{
  if(ServerConnect)
    ServerConnect->StopZMQThread();
}

void ASynchronizationHelper::SetupCallbacks()
{
	UpButton->SetClickCallback([this]()
    {
      this->UpdateTiles(-this->BoxSet.Num());
    });
  UpButton->RegisterRuntime(FString(TEXT("UP")), 0, 500);
  DownButton->SetClickCallback([this]()
    {
      this->UpdateTiles(+this->BoxSet.Num());
    });
  DownButton->RegisterRuntime(FString(TEXT("DOWN")), 0, 500);
  FetchButton->SetClickCallback([this]()
    {
      if (GEngine && false)
      {
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, ServerConnect->PrintStates());
      }
      if(ActiveFile == -1)
        this->ServerConnect->FileRequest();
      else
      {
        this->ServerConnect->SelectedRequest(ActiveFile);
      }
      bInit = 3;
    });
  FetchButton->RegisterRuntime(FString(TEXT("FETCH")),0, 500);
  SendButton->SetClickCallback([this]()
    {
      if (GEngine && false)
      {
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, ServerConnect->PrintStates());
      }
      ARootModellingPawn* pawn = Cast<ARootModellingPawn>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
      if (pawn)
      {
        //pawn->ResetInteraction();
      }
      TArray<URoot*> data = RootSink->ParseSegmentsIntoRootsystem();
      ServerConnect->SendRootSystem(data);
    });
  SendButton->RegisterRuntime(FString(TEXT("SEND")), 0, 500);
  IsoCurSign->RegisterRuntime(FString(TEXT("2000")), 0);
  IsoMaxSign->RegisterRuntime(FString(TEXT("Max: ???")), 0);
  IsoMinSign->RegisterRuntime(FString(TEXT("Min: ???")), 0);
  IsoCurSign->SetClickCallback([this]()
    {
      if (GEngine && false)
      {
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("NEW ISOVALUE"));
      }  
      ServerConnect->FetchNewIsosurface(IsoSurfaceParam);
    });
  RootLock->RegisterRuntime(FString(TEXT("Lock Root")),0, 500);
  RootLock->SetClickCallback([this]()
    {
      if (GEngine && false)
      {
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("ROOT CALLBACK"));
      }
      if (!ServerConnect->GetRootSystemLock())
      {
        RootLock->GetStaticMeshComponent()->SetVectorParameterValueOnMaterials(TEXT("Background"), FVector(FColor::Blue.R,FColor::Blue.B,FColor::Blue.G));
        ServerConnect->ToggleRootSystemLock();
      }
      else
      {
        RootLock->GetStaticMeshComponent()->SetVectorParameterValueOnMaterials(TEXT("Background"), FVector(0,0,0));
        ServerConnect->ToggleRootSystemLock();
      }
    });

  ReDrawRoot->RegisterRuntime(FString(TEXT("Make Taproot")), 0, 500);
  ReDrawRoot->SetClickCallback([this]()
    {
      if (GEngine && false)
      {
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("TAPROOT"));
      }
      ARootModellingPawn* pawn = Cast<ARootModellingPawn>(UGameplayStatics::GetPlayerPawn(GetWorld(),0));
      if (pawn && pawn->Selection.Num() == 1)
      {
        TArray<URoot*> data = RootSink->ParseSegmentsIntoRootsystem();
        ServerConnect->RequestTopologyChange(data,pawn->Selection[0]->RootNum);
      }
      pawn->ResetInteraction();
    });

  ToggleMaxLength->RegisterRuntime(FString(TEXT("Cap Segment Length")), 0, 500);
  ToggleMaxLength->SetClickCallback([this]()
    {
      ARootModellingPawn* pawn = Cast<ARootModellingPawn>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
      if (pawn)
      {
        pawn->bFixMaxLength = !(pawn->bFixMaxLength);
        if (pawn->bFixMaxLength)
        {
          ToggleMaxLength->RegisterRuntime(FString(TEXT("Capped at ")).Append(FString::SanitizeFloat(pawn->fMaximumSegmentLength)), 0, 500);
        }
        else
        {
          ToggleMaxLength->RegisterRuntime(FString(TEXT("Cap Segment Length")), 0, 500);
        }
      }
    });
  Undo->RegisterRuntime(FString(TEXT("Undo")), 0, 500);
  Undo->SetClickCallback([this]()
    {
      ARootModellingPawn* pawn = Cast<ARootModellingPawn>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
      if (pawn)
      {
        pawn->ResetInteraction();
      }
      ServerConnect->UndoAction();
    });
}

void ASynchronizationHelper::UpdateTiles(int offset)
{
  if (FileList.Num() > BoxSet.Num())
  {
    ScrollPos = (unsigned int)FGenericPlatformMath::Max(0,
      FGenericPlatformMath::Min(FileList.Num()-1, (int)ScrollPos +offset));
    
  }
  else
  {

    ScrollPos = 0;
  }
  for(unsigned int i = 0; i < (unsigned int)BoxSet.Num(); ++i)
  {
    BoxSet[i]->Write(TEXT(""), 0);
  }
  for (unsigned int i = 0;
        i < FGenericPlatformMath::Min((unsigned int)BoxSet.Num(),
            FileList.Num()-ScrollPos);
        ++i)
  {
    if (GEngine && false)
    {
      GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, FileList[i + ScrollPos]);
    }
    BoxSet[i]->Write(FileList[i+ScrollPos],i+ScrollPos,450);
    if (CurrentlyHighlighted)
    {
      CurrentlyHighlighted->UpdateColor(FVector(0,0,0));
      CurrentlyHighlighted = nullptr;
    }
    if (ActiveFile == i + ScrollPos)
    {
      CurrentlyHighlighted = BoxSet[i];
    }
  }
}

void ASynchronizationHelper::ShowIsosurface(const TArray<FVector>& Points,
  const TArray<FVector>& Normals,
	const TArray<int>& Triangles)
{
  RootSink->MakeIsosurface(Points,Normals,Triangles);

  // What i am doing here is making sure that the iso range gets updated (tick wait)
  // when i receive a new file but it does not when I specifically ask for another
  // iso value for my meshimesh
  if(bInit == 3)
    bInit = 2;
  else bInit = 1;
}

void ASynchronizationHelper::RelayBounds(FVector Bounds)
{
  if (GEngine && false)
  {
    GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, TEXT("RELAYING BOUNDS"));
  }
  RootSink->UpdateBounds(Bounds);
}

void ASynchronizationHelper::ShowState()
{
  ServerConnect->FileRequest();

  if (GEngine && false)
  {
    GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, TEXT("recv alive"));
  }
  bInit = 1;
}

void ASynchronizationHelper::ShowFiles(TArray<FString> FileNames)
{

  for (auto file : FileNames)
    if (GEngine && false)
    {
      GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, file);
    }
  FileList.Empty();
  for (auto f : FileNames)
  {
    if (FileList.Find(f) < 0)
    {
      FileList.Add(f);
    }
  }
  FileList.Sort();
  ScrollPos = 0;

  UpdateTiles(0);

  if (GEngine && false)
  {
    GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, TEXT("Recv files"));
  }
}

void ASynchronizationHelper::ShowRoots(const TArray<URoot*>& Data)
{

  if (GEngine && false)
  {
    GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, TEXT("Sync helper roots"));
  }
  RootSink->OverrideRootSystem(Data);
  if (bPerformanceMeasurement)
  {
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, RootRandomDelegate, 1, false, fWaitBetween);
  }
}

void ASynchronizationHelper::UpdateRange(float l, float u)
{
  if (GEngine && false)
  {
    GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, FString::Printf(TEXT("Now: %f->%f"), l,u));
  }
  IsoMinSign->Write(FString::Printf(TEXT("Min: %f"), u), -1);
  IsoMaxSign->Write(FString::Printf(TEXT("Max: %f"), l), -1);
  IsoSlider->SetActorLocation(IsoSlider->BeginPlayLocation);
  float d = l - u;
  IsoMin = u;
  IsoMax = l;
  bInit = 1;
}

void ASynchronizationHelper::AttachFileTiles(const TArray<AFileTargetBox*>& Tiles)
{
	BoxSet.Empty();

	for (auto* tile : Tiles)
	{
    if (GEngine && false)
    {
      GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::White, tile->GetFullName());
    }
		if (tile)
		{
      BoxSet.Add(tile);
      tile->RegisterRuntime(FString(TEXT("Hello")), 0);
      tile->SetClickCallback([this, tile]()
        {
          this->ActiveFile = tile->FileID;
          if(this->CurrentlyHighlighted)
            this->CurrentlyHighlighted->UpdateColor({0,0,0});
          this->CurrentlyHighlighted = tile;
          tile->UpdateColor(FVector(0.66f, 0.66f, 0));
        });
		}
	}
}

// Called every frame
void ASynchronizationHelper::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);
  if (ServerConnect && bInit == 0)
  {
    ServerConnect->TryContactingServer();
  }
  else if (bInit == 2)
  {
    if (GEngine && false)
    {
      GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::White, TEXT("ISORANGE"));
    }
    ServerConnect->RangeOfIsosurfaces();
  }
}

void ASynchronizationHelper::BeginDestroy()
{
  Super::BeginDestroy();
}
