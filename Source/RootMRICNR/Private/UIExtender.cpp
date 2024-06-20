// Fill out your copyright notice in the Description page of Project Settings.

#include "UIExtender.h"
#include "FileTargetBox.h"
#include "../RootModellingPawn.h"
#include "DrawSpace.h"
#include "RootArchitecture.h"

#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AUIExtender::AUIExtender()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	Showcase = CreateDefaultSubobject<UStaticMeshComponent>("ShowcaseTexture");
}

// Called when the game starts or when spawned
void AUIExtender::RegisterRuntime()
{
	Super::BeginPlay();
  RadiusLocking->RegisterRuntime(TEXT("Lock Radius"), 0, 400);
  Undo->RegisterRuntime(TEXT("Add Segment Top"), 0, 400);
  Reset->RegisterRuntime(TEXT("Reset GPU Geometry"), 0, 400);
  Delete->RegisterRuntime(TEXT("Clamp All Nodes"), 0, 400);

  //RadiusLocking->SetClickCallback(
  //  [Current = RadiusLocking,URPawn = Cast<ARootModellingPawn>(UGameplayStatics::GetPlayerPawn(GetWorld(),0))]
  //  (){
  //    URPawn->ToggleFixSegmentSize(!URPawn->bFixSegmentSize);
  //    Current->UpdateColor({0.f,1.f,0.f});
  //  });
  RadiusLocking->bAutoResetColor = false;
  Undo->SetClickCallback([URPawn = Cast<ARootModellingPawn>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0))]() {
    URPawn->RegisteredDrawSpace->RootSystemMesh->AddSectionTop();
    });
  Reset->SetClickCallback(
    [URPawn = Cast<ARootModellingPawn>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0))]
  (){
    if (GEngine && false)
    {
      GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, TEXT("Resetting GPU Mesh"));
    }
    URPawn->RegisteredDrawSpace->ResetGPU();
  });
  Delete->SetClickCallback(
    [URPawn = Cast<ARootModellingPawn>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0))]
  (){
    URPawn->RegisteredDrawSpace->NormRootSystem();
    URPawn->ResetInteraction();
  });
}

void AUIExtender::BeginPlay()
{
}

// Called every frame
void AUIExtender::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

