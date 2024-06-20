// Fill out your copyright notice in the Description page of Project Settings.


#include "OverlapTransform.h"
#include <Components/StaticMeshComponent.h>
#include <Engine/StaticMesh.h>
#include <UObject/ConstructorHelpers.h>
#include <Components/BoxComponent.h>
#include "..\Public\OverlapTransform.h"

// Sets default values
AOverlapTransform::AOverlapTransform()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Collider = CreateDefaultSubobject<UBoxComponent>(TEXT("Collider for Selection"));
	Collider->SetBoxExtent({0.001f,0.001f,0.001f},true);
	//this->Collider->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	//this->Collider->SetGenerateOverlapEvents(true);
	//this->Collider->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic,ECollisionResponse::ECR_Overlap);

	//this->Collider->SetActive(true);
  RootComponent = Collider;
  Representation = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Representation Mesh"));
  //static ConstructorHelpers::FObjectFinder<UStaticMesh> DonutAsset((TEXT("StaticMesh'/Game/3DUI/highlight_flat'")));
  static ConstructorHelpers::FObjectFinder<UStaticMesh> DonutAsset((TEXT("StaticMesh'/Game/SpawnfriendlySphere.SpawnfriendlySphere'")));
  //static ConstructorHelpers::FObjectFinder<UMaterial> DonutTex((TEXT("Material'/Game/3DUI/highlight_donut'")));
	if (DonutAsset.Succeeded())
	{
		Representation->SetStaticMesh(DonutAsset.Object);
  }
  //Representation->SetVisibility(false);
  //Representation->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	this->Representation->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	this->Representation->SetGenerateOverlapEvents(true);
	this->Representation->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic,ECollisionResponse::ECR_Overlap);
	Representation->SetupAttachment(RootComponent);
	//Representation->SetCastShadow(true);
}

void AOverlapTransform::Selected(bool SelectedValue)
{
	if (!IsValid(this))
		return;
	bSelected = SelectedValue;
	if(SelectedValue)
	{
		bHighlighted = false;
	  Representation->SetVectorParameterValueOnMaterials(TEXT("Color"), Active);
	}
	else
	{
	  Representation->SetVectorParameterValueOnMaterials(TEXT("Color"), Default);
	}
}

void AOverlapTransform::HighLight(int HighlightValue)
{
	if(HighlightValue == 1)
	{
		bHighlighted = true;
	  Representation->SetVectorParameterValueOnMaterials(TEXT("Color"), MightSelect);
	}
	else if (HighlightValue == 0)
	{
		bHighlighted = false;
	  Representation->SetVectorParameterValueOnMaterials(TEXT("Color"), (bSelected)?Active:Default);
	}
	else
	{
		bHighlighted = true;
	  Representation->SetVectorParameterValueOnMaterials(TEXT("Color"), MightUnselect);
	}
}

void AOverlapTransform::SetTemporary(bool NewTemporarilyVisible)
{
	if (this && !IsValid(this))
	{
		this->bTemporarilyVisible = NewTemporarilyVisible;
	}
}

void AOverlapTransform::SaveState()
{
	OriginScale = FVector(GetActorRelativeScale3D());
	OriginPosition = FVector(GetActorLocation());
	// Todo remove double information
	OriginOrientation = FQuat(GetActorRotation());
}

void AOverlapTransform::RetreiveState()
{
	if(OriginScale.Size() < 1e-5)
		return;
	else
	{
		SetActorLocation(OriginPosition);
		SetActorScale3D(OriginScale);
	}
}

// Called when the game starts or when spawned
void AOverlapTransform::BeginPlay()
{
	Super::BeginPlay();
	Representation->AttachToComponent(RootComponent,FAttachmentTransformRules::SnapToTargetNotIncludingScale);
}

// Called every frame
void AOverlapTransform::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

