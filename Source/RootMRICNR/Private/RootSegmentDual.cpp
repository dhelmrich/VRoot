// Fill out your copyright notice in the Description page of Project Settings.


#include "RootSegmentDual.h"
#include "RootCone.h"
#include "UObject/ConstructorHelpers.h"

// Sets default values for this component's properties
URootSegmentDual::URootSegmentDual()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

  RootSegmentMesh = CreateDefaultSubobject<URootCone>(TEXT("RootMesh"));
  RootJoint = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RootJoint"));
  static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereAsset(TEXT("StaticMesh'/Game/SpawnfriendlySphere'"));
  if (SphereAsset.Succeeded())
  {
    RootJoint->SetStaticMesh(SphereAsset.Object);
  }
  SetActive(true);
  RootSegmentMesh->SetupAttachment(this);
  RootJoint->SetupAttachment(this);
  SetVisibility(true);
  RootSegmentMesh->SetVisibility(true);
  RootJoint->SetVisibility(true);
  RootSegmentMesh->SetActive(true);
  RootJoint->SetActive(true);
  SetComponentTickEnabled(false);
	// ...
}


int URootSegmentDual::NumberOfAttachedRootSegments()
{
  int num = 0;
  for (USceneComponent* child : GetAttachChildren())
  {

    if (GEngine && false)
    {
      GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, (child->GetClass()->GetFName().ToString()));
    }
    if (child->IsA(URootSegmentDual::StaticClass()))
      num++;
  }
  return num - 1;
}

// Called when the game starts
void URootSegmentDual::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


void URootSegmentDual::RegisterRuntimeSpawn()
{
  RootJoint->RegisterComponent();
  RootSegmentMesh->RegisterComponent();
}

URootSegmentDual* URootSegmentDual::HasRootAttachment()
{
  URootSegmentDual* parent = Cast<URootSegmentDual>(GetAttachParent());
  return parent;
}

void URootSegmentDual::RecenterChildren(URootSegmentDual* Originator)
{

}

void URootSegmentDual::OnReleased_Implementation()
{

}

void URootSegmentDual::OnRay_Implementation(FVector WorldPosition)
{

}

void URootSegmentDual::OnGrabbed_Implementation()
{

}

void URootSegmentDual::OnClicked_Implementation(FVector WorldPositionOfClick)
{

}
