// Fill out your copyright notice in the Description page of Project Settings.


#include "DrawSpace.h"


#include "Containers/Queue.h"
#include "RootCone.h"
#include "RootArchitecture.h"
#include "OverlapTransform.h"
#include <Components/BoxComponent.h>
#include "..\Public\DrawSpace.h"

#define MAX(a,b) ((a > b)?a : b)

// make the scene darker!

// Sets default values
ADrawSpace::ADrawSpace()
{
  // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
  PrimaryActorTick.bCanEverTick = true;
  static ConstructorHelpers::FObjectFinder<UMaterial> Isomat((TEXT("Material'/Game/mat_root'")));
  if (Isomat.Succeeded())
  {
    SurfaceMaterial = Isomat.Object;
  }
}

void ADrawSpace::SetHightlightActor(UStaticMeshComponent* highlight)
{
  HightlightActor = highlight;
  //HightlightActor->SetVisibility(false);
}

UStaticMeshComponent* ADrawSpace::GetHighlightActor()
{
  return HightlightActor;
}

USceneComponent* ADrawSpace::GetAttachRoot()
{
  return RootSystemMesh;
}

void ADrawSpace::UpdateSections(const TArray<AOverlapTransform*>& Data, bool bFix, bool bAdjustRadius)
{
  CurrentStoredRadius = Data[0]->GetActorScale3D().GetAbsMax();
  if (bFix)
  {
    for (auto* ot : RootSystemMesh->FetchOverlapActors())
    {
      ot->SetActorRelativeScale3D(Data[0]->GetActorRelativeScale3D());
    }
    RootSystemMesh->UpdateSections(RootSystemMesh->FetchOverlapActors(), bAdjustRadius);
  }
  else
    RootSystemMesh->UpdateSections(Data, bAdjustRadius);
}

void ADrawSpace::InitializeRootSystem()
{
  
}

void ADrawSpace::StopAddingSegment()
{
  if (LastUpdated)
  {
    URootSegment* attachroot = Cast<URootSegment>(LastUpdated->GetAttachParent());
    LastUpdated->RemoveFromRoot();
    LastUpdated->UnregisterComponent();
    LastUpdated->DestroyComponent(true);
    LastUpdated = nullptr;
    if (bAutoAppend && attachroot)
      SegmentTreeMarker = attachroot;
    else
      SegmentTreeMarker = nullptr;
  }
}

// this potentially always takes into account the location that was set
// but usually it is probably easier to not do that
URootCone* ADrawSpace::AddRootSegmentAtLocation(FVector loc, bool forcecreatenew)
{
  URootSegment* rs = NewObject<URootSegment>(this,
    MakeUniqueObjectName(this, URootSegment::StaticClass()));
  rs->RootSegmentMesh->resolution = ConeResolution;
  LastUpdated = rs;
  rs->RegisterRuntimeSpawn();
  if (SegmentTreeMarker == nullptr)
  {
    if (GEngine && false)
    {
      GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Creating first one"));
    }
    rs->AttachToComponent(GetRootComponent(),
      FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    //   rs->SetWorldLocation(loc);
    rs->RootSegmentMesh->SetWorldLocation(loc);
    //rs->RootJoint->SetWorldLocation(loc);
    rs->RootNumber = GetNewRootNumber();
    RootNumberTree.FindOrAdd(rs->RootNumber);
  }
  else
  {
    // if it has no attach childern then it will default to appending
    // otherwise we can force it to create a new one once with a controller
    // click
    rs->AttachToComponent(SegmentTreeMarker, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

    //     if (ActiveNode->NumberOfAttachedRootSegments() > 0)
    //     {
    //       FVector rootlocation = ActiveNode->GetComponentLocation();
    //       rs->RootNumber = GetNewRootNumber();
    //       rs->SetWorldLocation(rootlocation);
    //       rs->RootSegmentMesh->SetWorldLocation(rootlocation);
    //       rs->RootJoint->SetWorldLocation(rootlocation);
    //     }
    //     else
    //     {
    FVector rootlocation = SegmentTreeMarker->RootSegmentMesh->GetComponentLocation();
    rootlocation += ((SegmentTreeMarker->RootSegmentMesh->GetComponentScale().X)
      * (SegmentTreeMarker->RootSegmentMesh->GetForwardVector()));
    if (SegmentTreeMarker->NumberOfAttachedRootSegments() > 0)
    {
      rs->RootNumber = GetNewRootNumber();
      RootNumberTree.FindOrAdd(SegmentTreeMarker->RootNumber).Add(rs->RootNumber);
    }
    else
      rs->RootNumber = SegmentTreeMarker->RootNumber;
    //      rs->SetWorldLocation(rootlocation);
    rs->RootSegmentMesh->SetWorldLocation(rootlocation);
    //rs->RootJoint->SetWorldLocation(rootlocation);

    //    }
  }
  rs->SetActive(true);
  rs->SetVisibility(true);
  rs->RegisterComponent();

  return rs->RootSegmentMesh;
}

FTransform ADrawSpace::ConvertTransformToDrawspace(FTransform UnitTransform)
{
  return GetActorTransform() * UnitTransform;
}

TArray<AOverlapTransform*> ADrawSpace::FindDependencies(AOverlapTransform* Activator)
{
  return RootSystemMesh->FindDependencies(Activator);
}

FTransform ADrawSpace::ConvertTransformToUnit(FTransform DrawSpaceTransform)
{
  return DrawSpaceTransform * GetActorTransform().Inverse();
}

void ADrawSpace::OverrideRootSystem(const TArray<URoot*>& Data)
{
  if (GEngine && false)
  {
    GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, TEXT("drawspace roots"));
  }
  RootSystemMesh->MakeRoot(Data);
}

AOverlapTransform* ADrawSpace::FindTransformByRootNumber(int Root, int Segment)
{
  return this->RootSystemMesh->FindTransformByRootNumber(Root, Segment);
}

void ADrawSpace::SyncRadius(AOverlapTransform* Selection)
{
  const FSegment Seg = RootSystemMesh->Peek(Selection->SegmentNum);
  if (Seg.Diameter > 0.f)
  {
    CurrentStoredRadius = Seg.Diameter * 2.f;
  }
  CurrentStoredRadius = Selection->GetActorScale3D().X;
}

bool ADrawSpace::HasRoot()
{
  return RootSystemMesh->FetchPointList().Num() > 0;
}

void ADrawSpace::GenerateRandomRoot(int nsegments /*= 50*/)
{
  auto savdia = this->DiameterMode;
  this->DiameterMode = EDSDiameterMode::dia_mode_segment;

  TArray<FTransform> roots;
  FTransform current = GetActorTransform();
  current.AddToTranslation(FVector(0, 0, 200));

  for (int i = 0; i < nsegments; ++i)
  {
    auto sign = [](bool yes = true) -> float {
      return (yes)?((FMath::FRand() >= 0.5f) ? 1.f : -1.f):1.f;
    };
    
    FVector end(
      FMath::RandRange(RandomRangeLow.X, RandomRangeHigh.X)* (sign(XSign)),
      FMath::RandRange(RandomRangeLow.Y, RandomRangeHigh.Y)* (sign(YSign)),
      FMath::RandRange(RandomRangeLow.Z, RandomRangeHigh.Z)* (sign(ZSign))
    );
    if (GEngine && false)
    {
      GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Black, end.ToString());
    }
    float width = FMath::RandRange(2.f, 5.f);
    current.SetScale3D(FVector(end.Size(), width, width));
    current.SetRotation(end.ToOrientationQuat());
    FVector endpoint = end + current.GetLocation();
    roots.Add(current);
    URootCone* mesh = AddRootSegmentAtLocation(current.GetLocation());
    FTransform toadd = mesh->GetComponentTransform();
    toadd.MultiplyScale3D(current.GetScale3D());
    mesh->SetWorldTransform(toadd);
    mesh->SetWorldRotation(end.ToOrientationQuat());
    SignalRelease(endpoint);
    current = FTransform();
    current.SetLocation(endpoint);
  }
  this->DiameterMode = savdia;
}

void ADrawSpace::PrepareRootSystemForTransmission()
{
  // we assume here that the box has the same scale as the isosurface has bounds
  // we take this from either unreal or paraview
  // but paraview generally has easier accessable bounds
  // if need be this can be computed with O(n) additional cost in the proc mesh section
  // that means if the inverse transform of the drawspace is used, the coordinates should be correct

  // TODO CHECK IF ROOT NUMBER MAKES SENSE IN THE SCENE FOR ANY ROOT
  TArray<URoot*> RootContainer;
  URootSegment* TargetSegment = nullptr;
  TQueue<URootSegment*> NewRootBeginnings;
  int maximum_id = -1;
  // to access attached components in an actor, access the root component first
  for (USceneComponent* child : GetRootComponent()->GetAttachChildren())
  {
    TargetSegment = Cast<URootSegment>(child);
    if (TargetSegment)
    {
      NewRootBeginnings.Enqueue(TargetSegment);
    }
  }
  while (!(NewRootBeginnings.IsEmpty()))
  {
    URoot* currentroot = NewObject<URoot>();
    NewRootBeginnings.Dequeue(TargetSegment);
    currentroot->RootNumber = TargetSegment->RootNumber;
    maximum_id = MAX((currentroot->RootNumber),maximum_id);

    if (TargetSegment->GetAttachParent() == this->GetRootComponent())
    {
      currentroot->Predecessor = -1;
      currentroot->StartJoint = -1;
    }
    while (TargetSegment->HasRootAttachment())
    {
      URootSegment* childsegment;
      for (USceneComponent* child : TargetSegment->GetAttachChildren())
      {
        childsegment = Cast<URootSegment>(child);
        if (childsegment && childsegment->RootNumber != currentroot->RootNumber)
        {
          NewRootBeginnings.Enqueue(childsegment);
        }
        else if (childsegment)
        {
          FTransform componentInfo = ConvertTransformToUnit(childsegment->GetComponentTransform());
          currentroot->LCJointPositions.Add(componentInfo.GetLocation());
          float scale = childsegment->RootSegmentMesh->StartRadius;
          // self ref both of these should be the same
          scale /= (this->GetActorScale3D().Y + this->GetActorScale3D().Z)/2.f;
          // next root position is recorded in pos of next segment
          // we only have to add the local x scale when we parse the last one
          TargetSegment = childsegment;
        }
      }
    }
    auto lastroottransform = ConvertTransformToUnit(TargetSegment->GetComponentTransform());
    FVector endpos = lastroottransform.GetRotation().RotateVector(FVector(lastroottransform.GetScale3D().X,0.f,0.f));
    currentroot->LCJointPositions.Add(TargetSegment->GetComponentLocation() + endpos);
    float scale = TargetSegment->RootSegmentMesh->EndRadius;
    // self ref both of these should be the same
    scale /= (this->GetActorScale3D().Y + this->GetActorScale3D().Z) / 2.f;
    currentroot->Diameters.Add(scale);
    RootContainer.Add(currentroot);
    // extract number
    // iterate through dependency tree
    // unravel all transforms
    // push to uroot list
    // mark each new root with a marker
    // remember to set adjacency and maybe even at which joint it starts
  }
  TArray<unsigned int> remapping;
  remapping.Init(0,(uint32)maximum_id);
  RootContainer.Sort([](const auto& a, const auto& b) -> bool{
    return a.RootNumber < b.RootNumber;
  });

}

bool ADrawSpace::NeedsInitialRadius(AOverlapTransform* AttachSegmentMarker)
{
  return RootSystemMesh->NeedsInitialRadius(AttachSegmentMarker);
}

AOverlapTransform* ADrawSpace::MoveSelection(AOverlapTransform* Selection, bool DirectionUp)
{
  return RootSystemMesh->MoveSelection(Selection,DirectionUp);
}

const TArray<URoot*>& ADrawSpace::ParseSegmentsIntoRootsystem(bool Correction)
{
  return RootSystemMesh->FetchSubmissionData(Correction);

  // sort the segment list

  // parse each into a root, remembering attachment values

  // find out what root comes first
}

FVector ADrawSpace::GetNextPosition()
{
  FVector rootlocation = SegmentTreeMarker->RootSegmentMesh->GetComponentLocation();
  rootlocation += ((SegmentTreeMarker->RootSegmentMesh->GetComponentScale().X)
    * (SegmentTreeMarker->RootSegmentMesh->GetForwardVector()));
  return rootlocation;
}

void ADrawSpace::NormRootSystem()
{
  auto* cube = Cast<UStaticMeshComponent>(this->GetDefaultSubobjectByName(TEXT("Cube")));
  RootSystemMesh->Norm(cube->GetRelativeScale3D() * 2, -2 * cube->GetRelativeScale3D());
}

// Called when the game starts or when spawned
void ADrawSpace::BeginPlay()
{
  Super::BeginPlay();
  RootSystemMesh = FindComponentByClass<URootArchitecture>();
//   TArray<URoot*> Data;
//   URoot * cr = NewObject<URoot>(GetTransientPackage(),
//       MakeUniqueObjectName(GetTransientPackage(), URoot::StaticClass()));
//   cr->LCJointPositions = {FVector(0,0,0),
//                           FVector(100,100,0),
//                           FVector(200,200,100),
//                           FVector(400,400,0)};
//   cr->Diameters = {100.f,70.f,100.f,50 };
//   cr->Predecessor = -1;
//   cr->RootNumber = 0;
//   Data.Add(cr);
//   RootSystemMesh->MakeRoot(Data);


  //   URootCone* rs = NewObject<URootCone>(this,
  //     MakeUniqueObjectName(this, URootCone::StaticClass()));
  //   rs->NormalSplitting = false;
  //   rs->RegisterComponent();
  //   rs->resolution = ConeResolution;
  //   rs->AttachToComponent(GetRootComponent(),
  //     FAttachmentTransformRules::SnapToTargetIncludingScale);
  //   //   rs->SetWorldLocation(loc);
  //   rs->SetWorldLocation(FVector(150.f,0,150.f));
  //   rs->SetWorldScale3D(FVector(100,100,100));
  //   rs->RegisterAtRuntime();
}

int ADrawSpace::GetNewRootNumber()
{
  auto num = RootNumbers.Num();
  RootNumbers.Add(num);
  return num;
}

void ADrawSpace::GenerateOneRoot()
{
  FTransform current;
  FVector rootlocation = SegmentTreeMarker->RootSegmentMesh->GetComponentLocation();
  rootlocation += ((SegmentTreeMarker->RootSegmentMesh->GetComponentScale().X)
    * (SegmentTreeMarker->RootSegmentMesh->GetForwardVector()));
  auto sign = [](bool yes = true) -> float {
    return (yes) ? ((FMath::FRand() >= 0.5f) ? 1.f : -1.f) : 1.f;
  };

  FVector end(
    FMath::RandRange(RandomRangeLow.X, RandomRangeHigh.X) * (sign(XSign)),
    FMath::RandRange(RandomRangeLow.Y, RandomRangeHigh.Y) * (sign(YSign)),
    FMath::RandRange(RandomRangeLow.Z, RandomRangeHigh.Z) * (sign(ZSign))
  );
  if (GEngine && false)
  {
    GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Black, end.ToString());
  }
  float width = FMath::RandRange(2.f, 5.f);
  current.SetScale3D(FVector(end.Size(), width, width));
  current.SetRotation(end.ToOrientationQuat());
  FVector endpoint = end + rootlocation;
  URootCone* mesh = AddRootSegmentAtLocation(current.GetLocation());
  FTransform toadd = mesh->GetComponentTransform();
  toadd.MultiplyScale3D(current.GetScale3D());
  mesh->SetWorldTransform(toadd);
  mesh->SetWorldRotation(end.ToOrientationQuat());
  SignalRelease(endpoint);
}

// Called every frame
void ADrawSpace::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);
//   counter++;
//   if (counter > 5)
//   {
//     GenerateOneRoot();
//     counter = 0;
//   }
}

void ADrawSpace::SignalRelease(FVector EndPos)
{
  LastUpdated->EndPoint = EndPos;
  if (DiameterMode == EDSDiameterMode::dia_mode_segment)
  {
//     LastUpdated->RootJoint->SetWorldScale3D(FVector(
//       LastUpdated->RootSegmentMesh->GetComponentScale().GetAbsMin() * 1.5f,
// 
//       LastUpdated->RootSegmentMesh->GetComponentScale().GetAbsMin() * 1.5f,
// 
//       LastUpdated->RootSegmentMesh->GetComponentScale().GetAbsMin() * 1.5f
//     ));
  }
  else
  {
//     LastUpdated->RootJoint->SetWorldScale3D(FVector(
//       LastUpdated->RootSegmentMesh->StartRadius,
//       LastUpdated->RootSegmentMesh->StartRadius,
//       LastUpdated->RootSegmentMesh->StartRadius
//     ));
  }
  if (bAutoAppend)
    SegmentTreeMarker = LastUpdated;
}

AOverlapTransform* ADrawSpace::SplitSegment(AOverlapTransform* Upper, AOverlapTransform* Lower)
{
  return RootSystemMesh->SplitSegment(Upper,Lower);
}

void ADrawSpace::PrepareSegmentFromIndicator(AOverlapTransform* AttachBox, FTransform IndicatorPosition)
{
  // we save this because we call it anyway no matter wha

  RootSystemMesh->OriginalDrawPoint = IndicatorPosition;

  if (GEngine && false)
  {
    GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, IndicatorPosition.ToString());
  }
  
  if (AttachBox == nullptr)
  {
    // if we think that this is genuinely something we want to do then we could just say we create a new one here
    // but since I do that in the beginning I don't know how productive that would be
    // or we could ourselves spawn a new AOverlapIndicator

  }
  else
  {
    ActivatedSegmentNumber = AttachBox->SegmentNum;
  }
}

AOverlapTransform* ADrawSpace::MakeSegmentFromIndicator(FTransform EndPos)
{
  auto* res = RootSystemMesh->CreateNewSegment(EndPos,ActivatedSegmentNumber);
  CurrentStoredRadius = EndPos.GetScale3D().X;
  ActivatedSegmentNumber = -1;
  return res;
}

void ADrawSpace::OnReleased_Implementation()
{
  HightlightActor->SetVectorParameterValueOnMaterials(FName("BorderColor"), FVector(0, 0, 0));
  HightlightActor->SetVisibility(false);
}

void ADrawSpace::OnGrabbed_Implementation()
{
  HightlightActor->SetVectorParameterValueOnMaterials(FName("BorderColor"), FVector(1, 0, 0));
  HightlightActor->SetVisibility(true);
}

void ADrawSpace::StopCollisionUpdate(bool ExludeCaps)
{
//   for (AOverlapTransform* ovlp : this->RootSystemMesh->FetchOverlapActors())
//   {
//     ovlp->AttachToComponent(RootSystemMesh, FAttachmentTransformRules::KeepWorldTransform);
//   }


  for (AOverlapTransform* ovlp : this->RootSystemMesh->FetchOverlapActors())
  {
    if(!ExludeCaps || !ovlp->bEndCap)
      ovlp->Collider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  }



//   URootSegment* segmesh;
//   for (auto* comp : this->GetRootComponent()->GetAttachChildren())
//   {
//     segmesh = Cast<URootSegment>(comp);
//     if (segmesh)
//     {
//       segmesh->SignalCollisionUpdate(ECollisionEnabled::NoCollision);
//     }
//   }
}

void ADrawSpace::RestartCollision()
{
//   for (AOverlapTransform* ovlp : this->RootSystemMesh->FetchOverlapActors())
//   {
//     ovlp->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
//   }



  for (AOverlapTransform* ovlp : this->RootSystemMesh->FetchOverlapActors())
  {
    ovlp->Collider->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
  }




//   URootSegment* segmesh;
//   for (auto* comp : this->GetRootComponent()->GetAttachChildren())
//   {
//     segmesh = Cast<URootSegment>(comp);
//     if (segmesh)
//     {
//       segmesh->SignalCollisionUpdate(ECollisionEnabled::QueryOnly);
//     }
//   }
}

void ADrawSpace::UpdateBounds(FVector Bounds)
{
  if (!bCenterRootSystem)
    return;
  auto* cube = Cast<UStaticMeshComponent>(this->GetDefaultSubobjectByName(TEXT("Cube")));
  cube->SetRelativeScale3D(Bounds);
  if (NetworkSurface)
  {
    NetworkSurface->SetRelativeLocation(FVector(-Bounds.X / 2.f, -Bounds.Y / 2.f, -Bounds.Z));
    cube->SetRelativeLocation(FVector(0.f,0.f,-Bounds.Z));
  }
  //RootSystemMesh->SetRelativeLocation(FVector(Bounds.X/2.f,Bounds.Y/2.f, -Bounds.Z));
  //FVector c = RootSystemMesh->GetRelativeLocation();
  //c.Y = Bounds.Z;
  //RootSystemMesh->SetRelativeLocation(c);
}

void ADrawSpace::ResetGPU()
{
  if (GEngine && false)
  {
    GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, TEXT("Resetting GPU Mesh"));
  }
  RootSystemMesh->MakeRoot(RootSystemMesh->FetchSubmissionData(), true);
}

void ADrawSpace::MakeIsosurface(const TArray<FVector>& Points,
                                const TArray<FVector>& Normals,
                                const TArray<int>& Triangles)
{
  if (!NetworkSurface)
  {
    NetworkSurface = NewObject<UProceduralMeshComponent>(this,TEXT("Surface from VTK Server"));
    NetworkSurface->RegisterComponent();
    NetworkSurface->ClearAllMeshSections();
    NetworkSurface->AttachToComponent(this->GetRootComponent(),FAttachmentTransformRules::SnapToTargetIncludingScale);
  }
  else
  {
    NetworkSurface->ClearAllMeshSections();
  }
  TArray<FVector2D> UV;
  TArray<FColor> Colors;
  TArray<FProcMeshTangent> Tangents;
  Tangents.Init(FProcMeshTangent(FVector(),false), Points.Num());
  for (int i = 0; i < Triangles.Num(); i += 3)
  {
    Tangents[Triangles[i]].TangentX = Points[Triangles[i]] - Points[Triangles[i + 1]];
    // skip i+2
  }
  NetworkSurface->CreateMeshSection(0,Points,Triangles, Normals,UV,Colors,Tangents,false);
  NetworkSurface->SetMaterial(0, SurfaceMaterial);
  NetworkSurface->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ADrawSpace::DeleteSegments(const TArray<AOverlapTransform*>& Selection)
{
  for (AOverlapTransform* ovlp : Selection)
  {
    ovlp->Collider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ovlp->Representation->SetVisibility(false,true);
  }
  RootSystemMesh->RemoveSections(Selection);
  if(!HasRoot())
  {
    CurrentStoredRadius = RadiusDefault;
  }
  else
  {
    RootSystemMesh->MakeRoot(RootSystemMesh->FetchSubmissionData(), true);
  }
}

void ADrawSpace::ZeroRootSystem()
{
  FVector c = RootSystemMesh->GetRelativeLocation();
  c.Y = this->GetActorRelativeScale3D().Z;
}

