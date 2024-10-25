// Fill out your copyright notice in the Description page of Project Settings.


#include "RootDataSwitcher.h"
#include "DrawSpace.h"
#include "RootArchitecture.h"

#include "Components/StaticMeshComponent.h"
#include "RootMRICNR/RootModellingPawn.h"

// Sets default values
ARootDataSwitcher::ARootDataSwitcher()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
  RootMesh = nullptr;
  RootMeshAsset = nullptr;
  DrawSpace = nullptr;
  Button = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Button"));
  RootComponent = Button;

}

// Called when the game starts or when spawned
void ARootDataSwitcher::BeginPlay()
{
	Super::BeginPlay();

	
}

// Called every frame
void ARootDataSwitcher::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ARootDataSwitcher::OnClickSignal_Implementation(FVector WorldPositionOfClick)
{
  if(!DrawSpace || !RootMeshAsset) return;
	// check if mesh exists
  if (!RootMesh)
  {
    // make a new mesh and attach to draw space
    RootMesh = NewObject<UStaticMeshComponent>(DrawSpace);
    RootMesh->RegisterComponent();
    // setup transform
    RootMesh->AttachToComponent(DrawSpace->GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
  }
  RootMesh->SetStaticMesh(RootMeshAsset);
  RootMesh->SetRelativeScale3D(CustomScale);
  DrawSpace->RootSystemMesh->MakeEmpty();

  // get player pawn
  auto player = GetWorld()->GetFirstPlayerController()->GetPawn();
  auto asRootModellingPawn = Cast<ARootModellingPawn>(player);
  asRootModellingPawn->ResetInteraction_Implementation();
}

