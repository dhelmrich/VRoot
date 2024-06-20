// Fill out your copyright notice in the Description page of Project Settings.


#include "RootCone.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GenericPlatform/GenericPlatformMath.h"

// Sets default values for this component's properties
URootCone::URootCone()
{
  // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
  // off to improve performance if you don't need them.
  PrimaryComponentTick.bCanEverTick = true;
  ConeMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Cone"));
  ConeMesh->SetupAttachment(this);
  ConeMesh->ClearAllMeshSections();
  static ConstructorHelpers::FObjectFinder<UMaterial> CylinderMat((TEXT("Material'/Game/mat_cylinder'")));
  this->SetComponentTickEnabled(true);
  if (CylinderMat.Succeeded())
  {

  }

  MatInstance = UMaterialInstanceDynamic::Create(CylinderMat.Object, ConeMesh);
  ConeMesh->SetMaterial(0, MatInstance);

  // ...
}


void URootCone::Build()
{
  StartPoints.Init(FVector(), resolution * 4);
  // Normal splitting makes this 
  Normals.Init(FVector(), resolution * 4);
  Tangents.Init(FProcMeshTangent(FVector(0, 1, 0), false), resolution * 4);
  MovingNormal.Init(FVector(), resolution);
  UVCoords.Init(FVector2D(), resolution * 4);
  Triangles.Init(0, resolution * 2 * 3);

  auto deltaphi = 2 * PI / resolution;
  for (int i = 0; i < resolution; ++i)
  {
    auto angle = static_cast<float>(i) * deltaphi; // yes


    // I think yes
    // changing sine and cosine makes the circle just move the other way
    StartPoints[2 * i] = (FVector(0.f, FMath::Cos(angle) * StartRadius,
      FMath::Sin(angle) * StartRadius));
    if (i == 0)
      StartPoints[(2 * resolution) - 1] = StartPoints[0];
    else
      StartPoints[(2 * i) - 1] = StartPoints[2 * i];

    MovingNormal[i] = StartPoints[2 * i] * (1.f / StartPoints[2 * i].Size());
    MovingNormal[i].Normalize();

    // we always know at which distance (1.f) the cone ends
    StartPoints[(2 * i) + (2 * resolution)] = (FVector(1.f, FMath::Cos(angle) * EndRadius,
      FMath::Sin(angle) * EndRadius));
    if (i == 0)
      StartPoints[(2 * resolution) - 1 + (2 * resolution)] = StartPoints[(2 * i) + (2 * resolution)];
    else
      StartPoints[(2 * i) - 1 + (2 * resolution)] = StartPoints[(2 * i) + (2 * resolution)];



    // no
    Triangles[(6 * i) + 2] = (2 * i);
    Triangles[(6 * i) + 1] = (2 * i) + 1;
    Triangles[(6 * i) + 0] = (2 * resolution) + (2 * i);
    Triangles[(6 * i) + 3] = (2 * resolution) + (2 * i);
    Triangles[(6 * i) + 4] = (2 * resolution) + (2 * i) + 1;
    Triangles[(6 * i) + 5] = (2 * i) + 1;

    // think about moving this to its own thing because we may need to move indices
    if (i > 0)
    {
      Normals[2 * i - 2] = FVector::CrossProduct(
        StartPoints[2 * i - 1] - StartPoints[(2 * i) - 2],
        StartPoints[(2 * i) - 2 + (2 * resolution)] - StartPoints[(2 * i) - 2]);
      Normals[2 * i - 2].Normalize();
      Normals[(2 * i) - 1] = Normals[2 * i - 2];
      Normals[(2 * i) - 1 + 2 * resolution] = Normals[2 * i - 2];
      Normals[(2 * i) - 2 + 2 * resolution] = Normals[2 * i - 2];

      // we need 

      // some tangents are on the border of the triangle and are thus not very easy to handle
      Tangents[2 * i - 2].TangentX = (StartPoints[(2 * i) - 1] - StartPoints[2 * i - 2]);
      Tangents[2 * i].TangentX.Normalize();
      Tangents[(2 * i) - 1].TangentX = Tangents[2 * i - 2].TangentX;
      Tangents[(2 * i) - 1 + (2 * resolution)].TangentX = Tangents[2 * i - 2].TangentX;
      Tangents[(2 * i) - 2 + (2 * resolution) - 1].TangentX = Tangents[2 * i - 2].TangentX;
    }

    // there are normals and tangent missing now I think because we discard the last two?

    UVCoords[(4 * i) + 0] = FVector2D(0.f, 0.f);
    UVCoords[(4 * i) + 1] = FVector2D(0.f, 1.f);
    UVCoords[(4 * i) + 2] = FVector2D(1.f, 0.f);
    UVCoords[(4 * i) + 3] = FVector2D(1.f, 1.f);
    // cos x + i sin y
    // the cylinder wraps around the x axis s.t. its end points towards view direction
    // in local space
    // 

    // Furthermore, we do not change the height as this is a scale thing
    // we only set the radii here. It is bad enough that
    // the radii are not even correct then (remember afterwards: width*YZScale)
    // uv coords are just 
  }

  Normals[2 * resolution - 2] = FVector::CrossProduct(
    StartPoints[2 * resolution - 1] - StartPoints[(2 * resolution) - 2],
    StartPoints[(2 * resolution) - 2 + (2 * resolution)] - StartPoints[(2 * resolution) - 2]);
  Normals[2 * resolution - 2].Normalize();
  Normals[(2 * resolution) - 1] = Normals[2 * resolution - 2];
  Normals[(2 * resolution) - 1 + 2 * resolution] = Normals[2 * resolution - 2];
  Normals[(2 * resolution) - 2 + 2 * resolution] = Normals[2 * resolution - 2];


  // we need 

  // some tangents are on the border of the triangle and are thus not very easy to handle
  Tangents[2 * resolution - 2].TangentX = (StartPoints[(2 * resolution) - 1] - StartPoints[2 * resolution - 2]);
  Tangents[2 * resolution].TangentX.Normalize();
  Tangents[(2 * resolution) - 1].TangentX = Tangents[2 * resolution - 2].TangentX;
  Tangents[(2 * resolution) - 1 + (2 * resolution)].TangentX = Tangents[2 * resolution - 2].TangentX;
  Tangents[(2 * resolution) - 2 + (2 * resolution) - 1].TangentX = Tangents[2 * resolution - 2].TangentX;

  ConeMesh->CreateMeshSection_LinearColor(0, StartPoints, Triangles, Normals, UVCoords, VertexColors, Tangents, true);
  ConeMesh->SetMeshSectionVisible(0, true);
}

void URootCone::RegisterAtRuntime()
{
  StartRadius = (StartRadius > 0.05f) ? StartRadius : 0.05f;
  EndRadius = (EndRadius > 0.05f) ? EndRadius : 0.05f;
  ConeMesh->RegisterComponent();
  if (NormalSplitting)
    Build();
  else
    BuildNoSplit();
  if(this->RenderFlats)
    RenderCaps();
}

void URootCone::SetStartRadius(float start_radius)
{
  start_radius = (start_radius > 0.05f) ? start_radius : 0.05f;
  if (!NormalSplitting)
  {
    UpdateNoSplitStart(start_radius);
    if (this->RenderFlats)
      UpdateCap(true);
    return;
  }
  //Update Mesh section
  for (int i = 0; i < resolution; ++i)
  {
    StartPoints[2 * i] = MovingNormal[i] * start_radius;
    if (i == 0)
      StartPoints[(2 * resolution) - 1] = StartPoints[0];
    else
      StartPoints[(2 * i) - 1] = StartPoints[2 * i];

    // think about moving this to its own thing because we may need to move indices
    if (i > 0)
    {
      Normals[2 * i - 2] = FVector::CrossProduct(
        StartPoints[2 * i - 1] - StartPoints[(2 * i) - 2],
        StartPoints[(2 * i) - 2 + (2 * resolution)] - StartPoints[(2 * i) - 2]);
      Normals[2 * i - 2].Normalize();
      Normals[(2 * i) - 1] = Normals[2 * i - 2];
      Normals[(2 * i) - 1 + 2 * resolution] = Normals[2 * i - 2];
      Normals[(2 * i) - 2 + 2 * resolution] = Normals[2 * i - 2];
      // we need 

      // some tangents are on the border of the triangle and are thus not very easy to handle
      Tangents[2 * i - 2].TangentX = (StartPoints[(2 * i) - 1] - StartPoints[2 * i - 2]);
      Tangents[2 * i].TangentX.Normalize();
      Tangents[(2 * i) - 1].TangentX = Tangents[2 * i - 2].TangentX;
      Tangents[(2 * i) - 1 + (2 * resolution)].TangentX = Tangents[2 * i - 2].TangentX;
      Tangents[(2 * i) - 2 + (2 * resolution) - 1].TangentX = Tangents[2 * i - 2].TangentX;
    }
  }
  Normals[2 * resolution - 2] = FVector::CrossProduct(
    StartPoints[2 * resolution - 1] - StartPoints[(2 * resolution) - 2],
    StartPoints[(2 * resolution) - 2 + (2 * resolution)] - StartPoints[(2 * resolution) - 2]);
  Normals[2 * resolution - 2].Normalize();
  Normals[(2 * resolution) - 1] = Normals[2 * resolution - 2];
  Normals[(2 * resolution) - 1 + 2 * resolution] = Normals[2 * resolution - 2];
  Normals[(2 * resolution) - 2 + 2 * resolution] = Normals[2 * resolution - 2];
  // we need 

  // some tangents are on the border of the triangle and are thus not very easy to handle
  Tangents[2 * resolution - 2].TangentX = (StartPoints[(2 * resolution) - 1] - StartPoints[2 * resolution - 2]);
  Tangents[2 * resolution].TangentX.Normalize();
  Tangents[(2 * resolution) - 1].TangentX = Tangents[2 * resolution - 2].TangentX;
  Tangents[(2 * resolution) - 1 + (2 * resolution)].TangentX = Tangents[2 * resolution - 2].TangentX;
  Tangents[(2 * resolution) - 2 + (2 * resolution) - 1].TangentX = Tangents[2 * resolution - 2].TangentX;


  ConeMesh->UpdateMeshSection_LinearColor(0, StartPoints, Normals, UVCoords, VertexColors, Tangents);


  StartRadius = start_radius;
  if (this->RenderFlats)
    UpdateCap(true);
}

void URootCone::SetEndRadius(float end_radius)
{
  end_radius = (end_radius > 0.05f) ? end_radius : 0.05f;
  if (!NormalSplitting)
  {
    UpdateNoSplitEnd(end_radius);
    if (this->RenderFlats)
      UpdateCap(false);
    return;
  }
  //Update Mesh section
  for (int i = 0; i < resolution; ++i)
  {
    StartPoints[2 * i + 2 * resolution] = MovingNormal[i] * end_radius + FVector(1.f,0,0);
    if (i == 0)
      StartPoints[(4 * resolution) - 1] = StartPoints[2 * resolution];
    else
      StartPoints[(2 * i) - 1 + 2 * resolution] = StartPoints[2 * i + 2 * resolution];

    // think about moving this to its own thing because we may need to move indices
    if (i > 0)
    {
      Normals[2 * i - 2] = FVector::CrossProduct(
        StartPoints[2 * i - 1] - StartPoints[(2 * i) - 2],
        StartPoints[(2 * i) - 2 + (2 * resolution)] - StartPoints[(2 * i) - 2]);
      Normals[2 * i - 2].Normalize();
      Normals[(2 * i) - 1] = Normals[2 * i - 2];
      Normals[(2 * i) - 1 + 2 * resolution] = Normals[2 * i - 2];
      Normals[(2 * i) - 2 + 2 * resolution] = Normals[2 * i - 2];

      // we need 

      // some tangents are on the border of the triangle and are thus not very easy to handle
      Tangents[2 * i - 2].TangentX = (StartPoints[(2 * i) - 1] - StartPoints[2 * i - 2]);
      Tangents[2 * i].TangentX.Normalize();
      Tangents[(2 * i) - 1].TangentX = Tangents[2 * i - 2].TangentX;
      Tangents[(2 * i) - 1 + (2 * resolution)].TangentX = Tangents[2 * i - 2].TangentX;
      Tangents[(2 * i) - 2 + (2 * resolution) - 1].TangentX = Tangents[2 * i - 2].TangentX;
    }
  }
  Normals[2 * resolution - 2] = FVector::CrossProduct(
    StartPoints[2 * resolution - 1] - StartPoints[(2 * resolution) - 2],
    StartPoints[(2 * resolution) - 2 + (2 * resolution)] - StartPoints[(2 * resolution) - 2]);
  Normals[2 * resolution - 2].Normalize();
  Normals[(2 * resolution) - 1] = Normals[2 * resolution - 2];
  Normals[(2 * resolution) - 1 + 2 * resolution] = Normals[2 * resolution - 2];
  Normals[(2 * resolution) - 2 + 2 * resolution] = Normals[2 * resolution - 2];

  // we need 

  // some tangents are on the border of the triangle and are thus not very easy to handle
  Tangents[2 * resolution - 2].TangentX = (StartPoints[(2 * resolution) - 1] - StartPoints[2 * resolution - 2]);
  Tangents[2 * resolution].TangentX.Normalize();
  Tangents[(2 * resolution) - 1].TangentX = Tangents[2 * resolution - 2].TangentX;
  Tangents[(2 * resolution) - 1 + (2 * resolution)].TangentX = Tangents[2 * resolution - 2].TangentX;
  Tangents[(2 * resolution) - 2 + (2 * resolution) - 1].TangentX = Tangents[2 * resolution - 2].TangentX;
  ConeMesh->UpdateMeshSection_LinearColor(0, StartPoints, Normals, UVCoords, VertexColors, Tangents);
  EndRadius = end_radius;
  if (this->RenderFlats)
    UpdateCap(false);
}

void URootCone::RenderCaps()
{
  StartCapPoints.Init(FVector(),resolution+1);
  EndCapPoints.Init(FVector(), resolution + 1);
  StartCapNormals.Init(FVector(-1.f, 0, 0), resolution+1);
  EndCapNormals.Init(FVector(1.f,0,0),resolution+1);
  CapColor.Init(FLinearColor(),resolution+1);
  CapUV.Init(FVector2D(), resolution + 1);
  StartCapTris.Init(0,resolution*3);
  EndCapTris.Init(0, resolution * 3);
  CapTangents.Init(FProcMeshTangent(), resolution + 1);

  StartCapPoints[0] = FVector(0,0,0);
  EndCapPoints[0] = FVector(1,0,0);
  CapTangents[0].TangentX = FVector(0,1,0);
  for (int i = 1; i < resolution + 1; ++i)
  {
    StartCapPoints[i] = MovingNormal[i-1] * StartRadius;
    EndCapPoints[i] = MovingNormal[i-1] * EndRadius + FVector(1.f,0.f,0.f);
    StartCapTris[3 * i - 3] = 0;
    StartCapTris[3 * i - 2] = i;
    StartCapTris[3 * i - 1] = (i >= resolution) ? 1 : i + 1;
    EndCapTris[3 * i - 1] = 0;
    EndCapTris[3 * i - 2] = i;
    EndCapTris[3 * i - 3] = (i >= resolution) ? 1 : i + 1;
    CapTangents[i].TangentX = MovingNormal[i-1];
  }
  ConeMesh->CreateMeshSection_LinearColor(1,StartCapPoints,StartCapTris,StartCapNormals,CapUV,CapColor,CapTangents, true);

  ConeMesh->CreateMeshSection_LinearColor(2, EndCapPoints, EndCapTris, EndCapNormals, CapUV, CapColor, CapTangents,true);

}

void URootCone::UpdateCap(bool start)
{
  if (start)
  {
    for (int i = 1; i < resolution + 1; ++i)
    {
      StartCapPoints[i] = MovingNormal[i - 1] * StartRadius;
    }
    ConeMesh->UpdateMeshSection_LinearColor(1, StartCapPoints, StartCapNormals, CapUV, CapColor, CapTangents);
  }
  else
  {
    for (int i = 1; i < resolution + 1; ++i)
    {
      EndCapPoints[i] = MovingNormal[i - 1] * EndRadius + FVector(1.f, 0.f, 0.f);
    }
    ConeMesh->UpdateMeshSection_LinearColor(2, EndCapPoints, EndCapNormals, CapUV, CapColor, CapTangents);
  }
}

void URootCone::BuildNoSplit()
{
  if (NormalSplitting)
    return;
  StartPoints.Init(FVector(), resolution * 2);
  // Normal splitting makes this 
  Normals.Init(FVector(), resolution * 2);
  Tangents.Init(FProcMeshTangent(FVector(1, 0, 0), false), resolution * 2);
  MovingNormal.Init(FVector(), resolution);
  UVCoords.Init(FVector2D(), resolution * 2);
  Triangles.Init(0, resolution * 2 * 3);

  auto deltaphi = 2 * PI / resolution;
  for (int i = 0; i < resolution; ++i)
  {
    auto angle = static_cast<float>(i) * deltaphi; // yes
    // I think yes
    // changing sine and cosine makes the circle just move the other way
    StartPoints[i] = (FVector(0.f, FMath::Cos(angle) * StartRadius,
      FMath::Sin(angle) * StartRadius));

    MovingNormal[i] = StartPoints[i] * (1.f / StartPoints[i].Size());
    MovingNormal[i].Normalize();
//     if (GEngine && false)
//     {
//       GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, MovingNormal[i].ToString());
//     }
    float norm = FGenericPlatformMath::Max(StartRadius, EndRadius);
    Normals[i] = MovingNormal[i];
    Normals[i].X = ((StartRadius / norm) - (EndRadius / norm));
    Normals[i + resolution] = Normals[i];

    // we always know at which distance (1.f) the cone ends
    StartPoints[(i)+(resolution)] = (FVector(1.f, FMath::Cos(angle) * EndRadius,
      FMath::Sin(angle) * EndRadius));

    // no
    Triangles[(6 * i) + 2] = (i);
    Triangles[(6 * i) + 1] = (((i)+1)%resolution);
    Triangles[(6 * i) + 0] = (resolution)+(i);
    Triangles[(6 * i) + 3] = (resolution)+(i);
    Triangles[(6 * i) + 4] = (resolution)+(((i)+1) % resolution);
    Triangles[(6 * i) + 5] = (((i)+1) % resolution);

    // think about moving this to its own thing because we may need to move indices

    Tangents[i].TangentX = StartPoints[resolution + i] - StartPoints[i];
    Tangents[resolution + i].TangentX = StartPoints[i] - StartPoints[resolution + i];
    // there are normals and tangent missing now I think because we discard the last two?

    UVCoords[i] = FVector2D(((i % 2 == 0) ? 0.f : 1.f), 0.f);
    UVCoords[i + resolution] = FVector2D(((i % 2 == 0) ? 0.f : 1.f), 1.f);
    // cos x + i sin y
    // the cylinder wraps around the x axis s.t. its end points towards view direction
    // in local space
    // 

    // Furthermore, we do not change the height as this is a scale thing
    // we only set the radii here. It is bad enough that
    // the radii are not even correct then (remember afterwards: width*YZScale)
    // uv coords are just 
  }

  ConeMesh->CreateMeshSection_LinearColor(0, StartPoints, Triangles, Normals, UVCoords, VertexColors, Tangents, true);
  ConeMesh->SetMeshSectionVisible(0, true);
}

void URootCone::UpdateNoSplitStart(float start_radius)
{  //Update Mesh section
  for (int i = 0; i < resolution; ++i)
  {
    StartPoints[i] = MovingNormal[i] * start_radius;
    MovingNormal[i] = StartPoints[i] * (1.f / StartPoints[i].Size());
    float norm = FGenericPlatformMath::Max(start_radius, EndRadius);
    Normals[i] = MovingNormal[i];
    Normals[i].X = ((start_radius / norm) - (EndRadius / norm));
    Normals[i + resolution] = Normals[i];
    Tangents[i].TangentX = StartPoints[resolution + i] - StartPoints[i];
    Tangents[resolution + i].TangentX = StartPoints[i] - StartPoints[resolution + i];
  }
  ConeMesh->UpdateMeshSection_LinearColor(0, StartPoints, Normals, UVCoords, VertexColors, Tangents);
  StartRadius = start_radius;
}

void URootCone::UpdateNoSplitEnd(float end_radius)
{  //Update Mesh section
  for (int i = 0; i < resolution; ++i)
  {
    StartPoints[i + resolution] = MovingNormal[i] * end_radius + FVector(1.f,0,0);

    // think about moving this to its own thing because we may need to move indices

    MovingNormal[i] = StartPoints[i] * (1.f / StartPoints[i].Size());
    float norm = FGenericPlatformMath::Max(end_radius, StartRadius);
    Normals[i] = MovingNormal[i];
    Normals[i].X = ((StartRadius / norm) - (end_radius / norm));
    Normals[i + resolution] = Normals[i];
    Tangents[i].TangentX = StartPoints[resolution + i] - StartPoints[i];
    Tangents[resolution + i].TangentX = StartPoints[i] - StartPoints[resolution + i];
  }
  ConeMesh->UpdateMeshSection_LinearColor(0, StartPoints, Normals, UVCoords, VertexColors, Tangents);
  EndRadius = end_radius;
}

// Called when the game starts
void URootCone::BeginPlay()
{
  Super::BeginPlay();
  // ...

}


// Called every frame
void URootCone::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
  if (countingup)
  {
    SetStartRadius(4*((float)counter) / 100.f);
    counter++;
    if (counter > 100)
      countingup = false;
  }
  else
  {
    SetStartRadius(4*((float)counter) / 100.f);
    counter--;
    if (counter < 10)
      countingup = true;
  }
  // ...
}

