// Fill out your copyright notice in the Description page of Project Settings.


#include "RootArchitecture.h"
#include "Kismet/KismetMathLibrary.h"
#include "Algo/Count.h"
#include "Components/BoxComponent.h"

// Sets default values
URootArchitecture::URootArchitecture()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
  PrimaryComponentTick.bCanEverTick = true;

  RootMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Root Mesh"));
  RootMesh->SetupAttachment(this);
  RootMesh->ClearAllMeshSections();
  
  static ConstructorHelpers::FObjectFinder<UMaterial> CylinderMat((TEXT("Material'/Game/mat_cylinder'")));
  this->SetComponentTickEnabled(true);
  if (CylinderMat.Succeeded())
  {
    RootMesh->SetMaterial(0,CylinderMat.Object);
  }

}

void URootArchitecture::AdjustRadiiForRoot(const TArray<FSegment*>& ChangedSegments)
{
  URoot* Root;
  for(const auto* Segment : ChangedSegments)
  {
    Root = FindRootForSegment(*Segment);
    if(Root)
    {
      for(int i = 0; i < Root->Diameters.Num(); ++i)
      {

        Root->Diameters[i] = (i < Segment->SegmentNumber) ?
                              FGenericPlatformMath::Max(Root->Diameters[i], Segment->Diameter)
                            : FGenericPlatformMath::Min(Root->Diameters[i], Segment->Diameter);
      }
    }
  }
  MakeRoot(RootData,true);

}

URoot* URootArchitecture::FindRootForSegment(const FSegment& Segment)
{
  bool bFoundRoot = false;
  URoot* foundroot = nullptr;
  for (URoot* mod : RootData)
  {
    if (mod->RootNumber == Segment.RootNumber)
    {
      foundroot = mod;
      bFoundRoot = true;
      break;
    }
  }
  return foundroot;
}

URoot* URootArchitecture::FindParentRoot(URoot* Root)
{
  bool bFoundRoot = false;
  URoot* foundroot = nullptr;
  for (URoot* mod : RootData)
  {
    if (mod->RootNumber == Root->Predecessor)
    {
      foundroot = mod;
      bFoundRoot = true;
      break;
    }
  }
  return foundroot;
}

void URootArchitecture::InitRootSystem()
{

}

void URootArchitecture::CreateUV()
{

}

void URootArchitecture::CreateSegment()
{

}

void URootArchitecture::MakeOrientedCircle(const FSegment& Segment, bool InPlace, FSegment* TriangleCheck)
{
  auto deltaphi = 2 * PI / Resolution;
  for (int i = 0; i < Resolution; ++i)
  {
    auto angle = static_cast<float>(i) * deltaphi; // yes
    // I think yes
    // changing sine and cosine makes the circle just move the other way
    FVector iPos = (FVector(0.f, FMath::Cos(angle) * Segment.Diameter,
      FMath::Sin(angle) * Segment.Diameter));
    iPos = Segment.Rotation * iPos;
    iPos += Segment.Position;

    if(InPlace)
    {
      const auto& Previous = *TriangleCheck;
      if (GEngine && false)
      {
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, TEXT("In place replacement"));
      }
      Points[Segment.PointStart + i] = iPos;
      VertexColors[Segment.PointStart + i] = OrderColor(Segment.RootNumber);

      Normals[Segment.NormalStart + i] = (Points[Segment.PointStart + i] - Segment.Position);
      Normals[Segment.NormalStart + i].Normalize();
      if(Segment.SegmentNumber > 0)
      {
        Triangles[Segment.TriangleStart + (6 * i) + 2] = Previous.PointStart + i;
        Triangles[Segment.TriangleStart + (6 * i) + 1] = Previous.PointStart + (((i)+1) % Resolution);
        Triangles[Segment.TriangleStart + (6 * i) + 0] = (Segment.PointStart + i);
        Triangles[Segment.TriangleStart + (6 * i) + 3] = (Segment.PointStart + i);
        Triangles[Segment.TriangleStart + (6 * i) + 4] =  Segment.PointStart + (((i)+1) % Resolution);
        Triangles[Segment.TriangleStart + (6 * i) + 5] = Previous.PointStart + (((i)+1) % Resolution);

        Tangents[Segment.NormalStart + i].TangentX = Points[Segment.PointStart + i]
          - Points[Previous.PointStart + i];
      }
      else
      {
        Tangents[Segment.NormalStart+i].TangentX = Segment.Length / Segment.Length.Size();
      }
      UVCoords[Segment.NormalStart + i] = FVector2D(((i % 2 == 0) ? 0.f : 1.f), 0.f);
    }
    else
    {
      if (GEngine && false)
      {
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("not in place replacement"));
      }
      Points.Add(iPos);
      FVector iNormal = (Points[Segment.PointStart + i] - Segment.Position);
      iNormal.Normalize();
      Normals.Add(iNormal);
      Tangents.Add(FProcMeshTangent(-iNormal,false));
      //VertexColors.Add(FColor::Black);
      VertexColors.Add(OrderColor(Segment.RootNumber));
      //VertexColors.Add((Segment.RootNumber == 1) ? FColor::Red : ((Segment.RootPredecessor == 1) ? FColor::Blue : FColor::Green));
      UVCoords.Add(FVector2D(((i % 2 == 0) ? 0.f : 1.f), 0.f));
    }
  }
  if (Segment.CapNumber > 0)
  {
    // spawn transform or just render a cap
    // in this case we should have the pointies
    RenderCap(Segment);
  }
}

void URootArchitecture::NearestPointToSelection(FVector selectionHit)
{

}

// expect sorted roots s.t. every root is known before the next one approaches
void URootArchitecture::MakeRoot(const TArray<URoot*>& Data, bool bSkipInit)
{
  if(IsValid(RootMesh))
  {
    RootMesh->ClearAllMeshSections();
  }
  

  if(!bSkipInit)
  {
    for (URoot* r : RootData)
    {
      if (r && IsValid(r))
      {
        r->ClearFlags(EObjectFlags::RF_NoFlags);
        r->ConditionalBeginDestroy();
      }
    }
    RootData.Empty();
    RootData.Append(Data);
  }
  auto notnullcount = Algo::CountIf(Data, [](auto root) -> SIZE_T {return root != nullptr; });



  if (GEngine && false)
  {
    GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, FString::Printf(TEXT("Starting at:")).Append(RootData[0]->LCJointPositions[0].ToString()));
  }

  SegmentNumberIndices.Empty();
  Points.Empty();
  Triangles.Empty();
  Normals.Empty();
  Tangents.Empty();
  if (GEngine && false)
  {
    GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, FString::Printf(TEXT("Num: %d"), SegmentNumberIndices.Num()));
  }
  if(Data.Num() == 0 || notnullcount == 0)
  {
    SegmentChangeIndices.RemoveAll([](AOverlapTransform* a) {
      
//       a->Representation->SetVisibility(false);
//       a->SetActorEnableCollision(false);
//       a->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
      if(a)
        a->Destroy();
//       a->RemoveFromRoot();
//       a->MarkPendingKill();
      return true;
      });
    return;
  }
  FixRootNumbers();
	int BufferTriangles = 0;
	int BufferNormals = 0;
  int BufferPoints = 0;
	int BufferIndices = 0;
	SegmentNumberIndices.Empty();
  int BufferCapNumber = 1;
  Orders.Reset();
	for (URoot* root : Data)
  {
    auto& value = Orders.FindOrAdd(root->RootNumber);
    if (root->RootNumber == 1)
    {
      UE_LOG(LogTemp, Warning, TEXT("Assigning a root number %d -> %d"), root->RootNumber, 1);
      value = 1;
    }
    else
    {
      if (Orders.Contains(root->Predecessor))
      {
        value = Orders[root->Predecessor] + 1;
        UE_LOG(LogTemp, Warning, TEXT("Assigning a root number %d -> %d"), root->RootNumber, value);
      }
      else
      {
        value = 2;
      }
    }
    if (GEngine && false)
    {
      UE_LOG(LogTemp, Display, TEXT("Root with: %d"), root->LCJointPositions.Num());
      //GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::White, FString::Printf(TEXT("Root with %d thingies"),root->LCJointPositions.Num()));
    }
    int fix = 0;
    if(root->LCJointPositions.Num() < 2)
      continue;
    for(int i = 0; i < root->LCJointPositions.Num() + fix; ++i)
    {
      FSegment indi;

      /*
        FVector AttachDiff = (Localtransform.GetLocation() - SegmentNumberIndices[AttachSegmentNumber].Position);
        AttachSegment.Position = ((AttachDiff/ AttachDiff.Size()) * (SegmentNumberIndices[AttachSegmentNumber].Diameter / 2.f))
                                     + SegmentNumberIndices[AttachSegmentNumber].Position;
      */
      
      if (i == root->LCJointPositions.Num() - 1 + fix)
      {
        indi.Length = (root->LCJointPositions[i] - root->LCJointPositions[i-1]);
        indi.Position = root->LCJointPositions[i];
//         GEngine->AddOnScreenDebugMessage(-1, 1500.0f, FColor::White, FString::Printf(TEXT("Last Point is (%f,%f,%f)"),root->LCJointPositions[i].X,
//           root->LCJointPositions[i].Y, 
//           root->LCJointPositions[i].Z ));
      }
      else
      {
        indi.Length = (root->LCJointPositions[i + 1] - root->LCJointPositions[i]);
        indi.Position = root->LCJointPositions[i];
      }
      indi.SegmentNumber = i;
      indi.Diameter = root->Diameters[i];
      indi.RootPredecessor = root->Predecessor;
      indi.TriangleStart = BufferTriangles;
      // we cap back!
      if(i > 0)
        BufferTriangles += Resolution * 2 * 3;

      indi.NormalStart = BufferNormals;
      BufferNormals += Resolution;

      indi.PointStart = BufferPoints;
      indi.TangentStart = BufferPoints;
      BufferPoints += Resolution;

      indi.RootNumber = root->RootNumber;

      // either the very first root or the ends of the rest
      if(i == root->LCJointPositions.Num()-1 + fix
        ||  (i == 0 && root->Predecessor < 0))
      {
        indi.CapNumber = BufferCapNumber;
        BufferCapNumber++;
      }

      SegmentNumberIndices.Add(indi);
    }
	}

  int numChange = 0;
  if (SegmentChangeIndices.Num() > SegmentNumberIndices.Num())
  {
    for (int i = SegmentNumberIndices.Num(); i < SegmentChangeIndices.Num(); ++i)
    {
      SegmentChangeIndices[i]->Representation->SetVisibility(false);
      SegmentChangeIndices[i]->SetActorEnableCollision(false);
      //SegmentChangeIndices[i]->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
      SegmentChangeIndices[i]->Destroy();
    }
  }
  SegmentChangeIndices.SetNum(FGenericPlatformMath::Min(SegmentChangeIndices.Num(),SegmentNumberIndices.Num()),true);
  //SegmentChangeIndices.Empty();
  //SegmentChangeIndices.Init(nullptr,SegmentNumberIndices.Num());

  Triangles.Init(0,BufferTriangles);
  Points.Init({},BufferPoints);
  Normals.Init({},BufferNormals);
  VertexColors.Init({},BufferNormals);
  UVCoords.Init({},BufferPoints);
  Tangents.Init({},BufferPoints);
  AOverlapTransform* SpawnUtil;
  FRotator LastRotation;
  FQuat LastOrient = FQuat::FindBetweenVectors({ 1,0,0 }, {1,0,0});
  int sn = 0;
  for(FSegment& Indices : SegmentNumberIndices)
  {
    
    FQuat orient = LastOrient * FQuat::FindBetweenVectors(LastOrient.GetForwardVector(),Indices.Length);
    Indices.Rotation = orient;
    if (numChange >= SegmentChangeIndices.Num())
    {
      SpawnUtil = GetWorld()->SpawnActor<AOverlapTransform>(Indices.Position, orient.Rotator(), FActorSpawnParameters());
      SegmentChangeIndices.Add(SpawnUtil);
      numChange++;
    }
    else
    {
      SpawnUtil = SegmentChangeIndices[numChange];
      numChange++;
    }
    SpawnUtil->SegmentNum = sn;
    SpawnUtil->PlaceSegment = Indices.SegmentNumber;
    SpawnUtil->RootNum = Indices.RootNumber;
    if(Indices.SegmentNumber == 0 && Indices.RootPredecessor >= 0)
      SpawnUtil->SetActorEnableCollision(false);
    else
      SpawnUtil->SetActorEnableCollision(true);
    //SpawnUtil->AttachToActor(this->GetAttachmentRootActor(), FAttachmentTransformRules::SnapToTargetIncludingScale);
    SpawnUtil->AttachToComponent(this,FAttachmentTransformRules::SnapToTargetIncludingScale);
    SpawnUtil->SetActorRelativeLocation(Indices.Position);
    //SpawnUtil->SetActorRelativeScale3D({ 1.f,1.5f * Indices.Diameter,1.5f * Indices.Diameter });
    SpawnUtil->SetActorRelativeScale3D({ 2.f * Indices.Diameter,2.f * Indices.Diameter,2.f * Indices.Diameter });
    SpawnUtil->SetActorRelativeRotation(orient);

    // TODO maybe remove this if it causes overt issues
//     if(Indices.SegmentNumber == 0)
//       SpawnUtil->Collider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    //SpawnUtil->AddActorWorldTransform(this->GetComponentTransform());
    auto deltaphi = 2 * PI / Resolution;
    for (int i = 0; i < Resolution; ++i)
    {
      auto angle = static_cast<float>(i) * deltaphi; // yes
      // I think yes
      // changing sine and cosine makes the circle just move the other way
      Points[Indices.PointStart + i] = (FVector(0.f, FMath::Cos(angle) * Indices.Diameter,
        FMath::Sin(angle) * Indices.Diameter));
      Points[Indices.PointStart + i] = orient* Points[Indices.PointStart + i];
      Points[Indices.PointStart + i] += Indices.Position;

      VertexColors[Indices.PointStart + i] = OrderColor(Indices.RootNumber);
        
      //VertexColors[Indices.PointStart + i] = (Indices.RootNumber == 1) ? FColor::Red : ((Indices.RootPredecessor == 1) ? FColor::Blue : FColor::Green);

      
      // make normal from center -> outer point
      Normals[Indices.NormalStart + i] = (Points[Indices.PointStart + i] - Indices.Position);
      Normals[Indices.NormalStart + i].Normalize();

      // we start at negative resolution with this now which means that I have to redo this completely
      // this case is the other way around: Everything that was "down", namely without Resolution
      // is not the past circle and the current one is the normal one. As such, we have to switch this around:
      // this should be switched now check again later
      if(Indices.SegmentNumber > 0)
      {
        Triangles[Indices.TriangleStart + (6 * i) + 2] = (Indices.PointStart + i)-Resolution;
        Triangles[Indices.TriangleStart + (6 * i) + 1] = Indices.PointStart + (((i)+1) % Resolution) - Resolution;
        Triangles[Indices.TriangleStart + (6 * i) + 0] = (Indices.PointStart + i);
        Triangles[Indices.TriangleStart + (6 * i) + 3] = (Indices.PointStart + i);
        Triangles[Indices.TriangleStart + (6 * i) + 4] = Indices.PointStart + (((i)+1) % Resolution);
        Triangles[Indices.TriangleStart + (6 * i) + 5] = Indices.PointStart + (((i)+1) % Resolution) - Resolution;

        Tangents[Indices.NormalStart + i].TangentX = Points[Indices.PointStart + i]
          - Points[Indices.PointStart + i - Resolution];
      }
      else
      {
        Tangents[Indices.NormalStart+i].TangentX = Indices.Length / Indices.Length.Size();
      }

      // think about moving this to its own thing because we may need to move indices
      // debug: because we go back there's no "+resolution"
      // there are normals and tangent missing now I think because we discard the last two?

      UVCoords[Indices.NormalStart + i] = FVector2D(((i % 2 == 0) ? 0.f : 1.f), 0.f);
    }
    if (Indices.CapNumber > 0)
    {
      // spawn transform or just render a cap
      // in this case we should have the pointies
      RenderCap(Indices);
    }
    ++sn;
  }

  RootMesh->CreateMeshSection_LinearColor(0,Points,Triangles,Normals,UVCoords,VertexColors,Tangents,false);
}

void URootArchitecture::MakeEmpty()
{
  MakeRoot({});
  SegmentNumberIndices.Empty();
  RemakeRoot();
}

void URootArchitecture::RemoveSections(const TArray<AOverlapTransform*>& Selection)
{
  TMap<int,int> HighestSegmentsToDelete;
  for (AOverlapTransform* Selected : Selection)
  {
    if (Selected->PlaceSegment == 1)
    {
      if (HighestSegmentsToDelete.Contains(Selected->RootNum))
      {
        HighestSegmentsToDelete[Selected->RootNum] = 0;

      }
      else
      {
        HighestSegmentsToDelete.Add(TTuple<int, int>(Selected->RootNum, 0));
      }

    }
    else if (HighestSegmentsToDelete.Contains(Selected->RootNum))
    {
      HighestSegmentsToDelete[Selected->RootNum] = 
              (Selected->PlaceSegment < HighestSegmentsToDelete[Selected->RootNum]) 
            ? Selected->PlaceSegment : HighestSegmentsToDelete[Selected->RootNum];
    }
    else
    {
      HighestSegmentsToDelete.Add(TTuple<int,int> (Selected->RootNum , Selected->PlaceSegment));
    }
  }
  // edit we were certain that this was correct


  for (TPair<int, int> Deletion : HighestSegmentsToDelete)
  {
    RemoveSections(Deletion.Key, Deletion.Value);
  }
  // also redo the pointstart entries on the sections to make this correct
}

void URootArchitecture::RemoveSections(int Root, int HighestSegment /*= 0*/)
{
  bool bFoundRoot = false;
  URoot* foundroot = nullptr;
  for (URoot* mod : RootData)
  {
    if (mod->RootNumber == Root)
    {
      mod->LCJointPositions.SetNum(HighestSegment);
      mod->Diameters.SetNum(HighestSegment);
      foundroot = mod;
      bFoundRoot = true;
    }
  }
  if(!bFoundRoot)
    return;
  if (HighestSegment == 0)
  {
    foundroot->RootNumber = -2;
    foundroot->Diameters.Empty();
    foundroot->StartJoint = -2;
    foundroot->Predecessor = -2;
    foundroot->ClearFlags(EObjectFlags::RF_NoFlags);
    foundroot->LCJointPositions.Empty();
    foundroot->Diameters.Empty();
    foundroot->ConditionalBeginDestroy();
    RootData.Remove(foundroot);
    RootData.RemoveSingle(foundroot);
  }
  SegmentNumberIndices.RemoveAll([Root, HighestSegment](FSegment a) {return a.RootNumber == Root && a.SegmentNumber >= HighestSegment; });
  SegmentChangeIndices.RemoveAll([Root, HighestSegment](AOverlapTransform* a) {
    if (a->RootNum == Root && a->PlaceSegment >= HighestSegment)
    {
      a->Representation->SetVisibility(false);
      a->SetActorEnableCollision(false);
      a->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
      a->RemoveFromRoot();
      //a->MarkPendingKill();
      a->Destroy();
      return true;
    }
    else return false;
    });
  for (URoot* mod : RootData)
  {
    if (mod->RootNumber == Root)
    {
      mod->LCJointPositions.SetNum(HighestSegment);
      mod->Diameters.SetNum(HighestSegment);
    }
  }
  TArray<int> AttachedRootNumbers;
  RootData.Sort([](URoot& a, URoot& b) {return a.Predecessor < b.Predecessor;});
  bool bRemovedSth = true;
  while(bRemovedSth)
  {
    bRemovedSth = false;
    for(int i = 0; i < RootData.Num(); ++i)
    {
      URoot* r;
      if (i < RootData.Num() && IsValid(RootData[i]))
      {
        r = RootData[i];
      }
      else continue;
      if ((r->Predecessor == Root && r->StartJoint >= HighestSegment) || AttachedRootNumbers.Find(r->Predecessor) != INDEX_NONE)
      {
        bRemovedSth = true;
        AttachedRootNumbers.Add(r->RootNumber);
        SegmentNumberIndices.RemoveAll([iRoot = r->RootNumber](FSegment a) {return a.RootNumber == iRoot; });
        SegmentChangeIndices.RemoveAll([iRoot = r->RootNumber](AOverlapTransform* a) {
          if (a->RootNum == iRoot)
          {
            a->Representation->SetVisibility(false);
            a->SetActorEnableCollision(false);
            //a->RemoveFromRoot();
            //a->MarkPendingKill();
            a->Destroy();
            //a->Delete(true);
            return true;
          }
          else return false;
          });
        r->RootNumber = -2;
        r->Diameters.Empty();
        r->StartJoint = -2;
        r->Predecessor = -2;
        r->ClearFlags(EObjectFlags::RF_NoFlags);
        r->LCJointPositions.Empty();
        r->Diameters.Empty();
        r->ConditionalBeginDestroy();
        RootData.Remove(r);
        RootData.RemoveSingle(r);
      }
    }
  }

  // what's this segmentnumber again btw

  SegmentChangeIndices.RemoveAll([Root, HighestSegment](AOverlapTransform* a) {
    if (a->RootNum == Root && a->PlaceSegment >= HighestSegment)
    {
      a->Representation->SetVisibility(false);
      a->SetActorEnableCollision(false);
      //a->RemoveFromRoot();
      //a->MarkPendingKill();
      return true;
    }
    else return false;
  });
  RemakeRoot();
}

const TArray<URoot*>& URootArchitecture::FetchSubmissionData(bool Correction)
{
  if(Correction)
  {
    FixRootNumbers();
    MakeRoot(RootData,true);
  }
  return RootData;
}

AOverlapTransform* URootArchitecture::CreateNewSegment(FTransform NewCircle, int AttachSegmentNumber)
{
  // I will allow a copy of the Segment here.
  NewCircle.SetScale3D(NewCircle.GetScale3D() / 2.f);
  FTransform SpaceInverse = GetComponentTransform().Inverse();
//   FTransform Localtransform = NewCircle.GetRelativeTransform(GetComponentTransform());
//   FTransform LocalOrigin = LocalOrigin.GetRelativeTransform(GetComponentTransform());
  FTransform Localtransform = NewCircle*SpaceInverse;
  FTransform LocalOrigin = OriginalDrawPoint *SpaceInverse;

  bool bCreatedCap = false;
  bool bCopyBack = false;
  int CapNumPrev = -1, CapNumNew = -1;

  //if (GEngine && true)
  //{
  //  GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Black, TEXT("MAKING A SEGMENT"));
  //  for(auto* root : RootData)
  //  {
  //    GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("Root: %d, #: %d, P: %d"), root->RootNumber,root->LCJointPositions.Num(),root->Predecessor));
  //  }
  //}

  // find a way to have an up vector with which i could compare
  // a new segment


  // volume variance

  // simplification zille idea:
  // 

  FSegment AttachSegment;
  URoot* RootToEdit;
  if (AttachSegmentNumber < 0 || AttachSegmentNumber >= SegmentChangeIndices.Num())
  {
    // new root, additional segment and making the points
    // if new root altogether, standard set of points
    AttachSegment.CapNumber = (RootMesh->GetNumSections() > 1) ? RootMesh->GetNumSections() : 1;
    CapNumNew = AttachSegment.CapNumber + 1;
    AttachSegment.PointStart = Points.Num();
    AttachSegment.TriangleStart = Triangles.Num();
    AttachSegment.TangentStart = Tangents.Num();
    AttachSegment.NormalStart = Normals.Num();
    AttachSegment.RootPredecessor = -1;
    AttachSegment.Rotation = LocalOrigin.GetRotation();
    AttachSegment.Position = LocalOrigin.GetLocation();
    AttachSegment.SegmentNumber = 0;
    AttachSegment.Length = -LocalOrigin.GetRotation().RotateVector({1,0,0});
    AttachSegment.Diameter = FGenericPlatformMath::Max(Localtransform.GetScale3D().Y, 0.001f);
    bCreatedCap = true;

    // NEW SPAWN UTIL

    do
    {
      RootToEdit = NewObject<URoot>(GetTransientPackage(), MakeUniqueObjectName(GetTransientPackage(), URoot::StaticClass()), EObjectFlags::RF_Standalone);
    } while (!RootToEdit);

    

    RootToEdit->RootNumber = FindRootNumber();
    RootToEdit->Predecessor = -1;
    RootToEdit->StartJoint = -1;
    RootToEdit->DiameterOnJoint = true;
    RootData.Add(RootToEdit);
    auto& ordervalue = Orders.FindOrAdd(RootToEdit->RootNumber);
    ordervalue = 1;

    RootToEdit->LCJointPositions.Add(AttachSegment.Position);
    RootToEdit->Diameters.Add(AttachSegment.Diameter);

    AttachSegment.RootNumber = RootToEdit->RootNumber;
    
    AOverlapTransform* SpawnUtil = GetWorld()->SpawnActor<AOverlapTransform>();
    // TODO IM CONFUSED
    SpawnUtil->SegmentNum = SegmentChangeIndices.Num();
    SpawnUtil->RootNum = RootToEdit->RootNumber;
    //SpawnUtil->AttachToActor(this->GetAttachmentRootActor(), FAttachmentTransformRules::SnapToTargetIncludingScale);
    SpawnUtil->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetIncludingScale);
    SpawnUtil->SetActorRelativeLocation(AttachSegment.Position);
    //SpawnUtil->SetActorRelativeScale3D({ 1.f,1.5f * AttachSegment.Diameter,1.5f * AttachSegment.Diameter });
    SpawnUtil->SetActorRelativeScale3D({ 2.f * AttachSegment.Diameter,2.f * AttachSegment.Diameter,2.f * AttachSegment.Diameter });
    SpawnUtil->SetActorRelativeRotation(AttachSegment.Length.ToOrientationQuat());
    SpawnUtil->PlaceSegment = AttachSegment.SegmentNumber;
    SegmentChangeIndices.Add(SpawnUtil);

    SegmentNumberIndices.Add(AttachSegment);
    MakeOrientedCircle(AttachSegment);
  }
  else if(SegmentNumberIndices[AttachSegmentNumber].CapNumber < 0 || (SegmentNumberIndices[AttachSegmentNumber].RootPredecessor < 0 && SegmentNumberIndices[AttachSegmentNumber].SegmentNumber == 0))
  {
    // this is the situation where we attach at the SIDE
    // meaning that we create a new root and also need
    // an implicit attachment
    AttachSegment.CapNumber = RootMesh->GetNumSections()+1;
    CapNumNew = AttachSegment.CapNumber + 1;
    AttachSegment.PointStart = Points.Num();
    AttachSegment.TriangleStart = Triangles.Num();
    AttachSegment.TangentStart = Tangents.Num();
    AttachSegment.NormalStart = Normals.Num();
    AttachSegment.RootPredecessor = SegmentNumberIndices[AttachSegmentNumber].RootNumber;
    AttachSegment.SegmentNumber = 0;
    bCreatedCap = true;

    // the implicit attachment is in the root itself
    // but that also means we have to move stuff
    // I should find a way to visualize this implicit attachment
    // what I COULD do is unreal-attach the overlapactors and then
    // try to do this nonmanually at the potential cost of stability
//     RootToEdit = NewObject<URoot>(this,GetTransientPackage(),
//       MakeUniqueObjectName(GetTransientPackage(), URoot::StaticClass()));
    //do
    //{
    //  RootToEdit = NewObject<URoot>(GetTransientPackage(), MakeUniqueObjectName(GetTransientPackage(), URoot::StaticClass()), EObjectFlags::RF_Standalone);
    //} while (!RootToEdit);

    RootToEdit = NewObject<URoot>(GetTransientPackage(), MakeUniqueObjectName(GetTransientPackage(), URoot::StaticClass()), EObjectFlags::RF_Standalone | EObjectFlags::RF_Transient);

    RootToEdit->RootNumber = FindRootNumber();
    AttachSegment.Rotation = Localtransform.GetRotation();
    RootToEdit->Predecessor = SegmentNumberIndices[AttachSegmentNumber].RootNumber;
    RootToEdit->StartJoint = SegmentNumberIndices[AttachSegmentNumber].SegmentNumber;
    RootToEdit->DiameterOnJoint = true;
    RootData.Add(RootToEdit);
    auto& ordervalue = Orders.FindOrAdd(RootToEdit->RootNumber);
    if (Orders.Contains(SegmentNumberIndices[AttachSegmentNumber].RootNumber))
    {
      ordervalue = 1 + Orders[SegmentNumberIndices[AttachSegmentNumber].RootNumber];
    }
    else
      ordervalue = 1;

    // we do not reuse the segment but have an implicit attachment!
    AttachSegment.RootNumber = RootToEdit->RootNumber;
    FVector AttachDiff = (Localtransform.GetLocation() - SegmentNumberIndices[AttachSegmentNumber].Position);
    AttachSegment.Position = ((AttachDiff/ AttachDiff.Size()) * (SegmentNumberIndices[AttachSegmentNumber].Diameter / 2.f))
                                 + SegmentNumberIndices[AttachSegmentNumber].Position;
    AttachSegment.Diameter = SegmentNumberIndices[AttachSegmentNumber].Diameter;
    AttachSegment.Length = (Localtransform.GetLocation() - SegmentNumberIndices[AttachSegmentNumber].Position);

    RootToEdit->LCJointPositions.Add(AttachSegment.Position);
    RootToEdit->Diameters.Add(AttachSegment.Diameter);
    
    AOverlapTransform* SpawnUtil;
    do
    {
      SpawnUtil = GetWorld()->SpawnActor<AOverlapTransform>();
    } while (!SpawnUtil);
    // TODO IM CONFUSED
    SpawnUtil->SegmentNum = SegmentNumberIndices.Num();
    SegmentNumberIndices.Add(AttachSegment);
    SegmentChangeIndices.Add(SpawnUtil);
    SpawnUtil->RootNum = AttachSegment.RootNumber;
    SpawnUtil->Predecessor = SegmentNumberIndices[AttachSegmentNumber].RootNumber;
    SpawnUtil->SetActorEnableCollision(false);
    //SpawnUtil->AttachToActor(this->GetAttachmentRootActor(), FAttachmentTransformRules::SnapToTargetIncludingScale);
    SpawnUtil->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetIncludingScale);
    SpawnUtil->SetActorRelativeLocation(AttachSegment.Position);
    //SpawnUtil->SetActorRelativeScale3D({ 1.f,1.5f * AttachSegment.Diameter,1.5f * AttachSegment.Diameter });

    SpawnUtil->SetActorRelativeScale3D({ 2.f * AttachSegment.Diameter,2.f * AttachSegment.Diameter,2.f * AttachSegment.Diameter });
    SpawnUtil->SetActorRelativeRotation(AttachSegment.Length.ToOrientationQuat());
    SpawnUtil->PlaceSegment = 0;


    MakeOrientedCircle(AttachSegment);
  }
  else
  {
    // remember the edge case of selecting the overlap transform that is at the start of the root
    // for now, i shall disable this option
    AttachSegment = SegmentNumberIndices[AttachSegmentNumber];
    URoot** FetchRoot = RootData.FindByPredicate([RNum = AttachSegment.RootNumber](URoot* root)
      {
        return IsValid(root) && root->RootNumber == RNum;
      });
    RootToEdit = (FetchRoot != nullptr) ? *FetchRoot : nullptr;
    if (!IsValid(RootToEdit))
    {
      UE_LOG(LogTemp, Warning,
                TEXT("Did not find root even though checks say it should exist. Wanted S%d<R%d<A%d"),
                AttachSegment.RootNumber, AttachSegment.SegmentNumber, RootData.Num());
      return nullptr;
    }
    CapNumNew = AttachSegment.CapNumber;
    AttachSegment.CapNumber = -1;
    SegmentNumberIndices[AttachSegmentNumber] = AttachSegment;
    bCopyBack = true;
  }
  FSegment * NormalizationSegment = SegmentNumberIndices.FindByPredicate([a=AttachSegment](const FSegment& s){return s.SegmentNumber == a.SegmentNumber - 1;});
  FTransform Righting = GetComponentTransform().Inverse();
  FSegment NewSeg;

  FTransform RootSpaceTransform = NewCircle*Righting;

  // check this if fails
  NewSeg.Length = NewCircle.GetRotation().RotateVector(FVector(1,0,0));
  NewSeg.PointStart = Points.Num();
  NewSeg.TriangleStart = Triangles.Num();
  NewSeg.TangentStart = Tangents.Num();
  NewSeg.NormalStart = Normals.Num();
  NewSeg.RootNumber = AttachSegment.RootNumber;
  NewSeg.RootPredecessor = AttachSegment.RootPredecessor;
  NewSeg.Rotation = RootSpaceTransform.GetRotation();
  NewSeg.CapNumber = CapNumNew;
  // todo if we paint at the start
  NewSeg.SegmentNumber = AttachSegment.SegmentNumber + 1; // this should always be correct because of attachsegment definition
  NewSeg.Diameter = FGenericPlatformMath::Max(RootSpaceTransform.GetScale3D().Y,0.001f);
  NewSeg.Position = RootSpaceTransform.GetLocation();
  // this should work usually, but I will probably have to fix things
  // once I actually try to do this
  // let's hope it doesn't fail (ever) but otherwise TODO CHECK
  RootToEdit->LCJointPositions.Add(NewSeg.Position);
  RootToEdit->Diameters.Add(NewSeg.Diameter);
  if(bCopyBack && NormalizationSegment && NormalizePreviousSegment)
  {
    auto FoundSegment = SegmentNumberIndices.FindByPredicate([comp = SegmentNumberIndices[AttachSegmentNumber]](auto seg) {return seg.RootNumber == comp.RootNumber && seg.SegmentNumber == comp.SegmentNumber - 1; });
    if(FoundSegment)
    {
      FQuat orient = FQuat::Slerp(SegmentNumberIndices[AttachSegmentNumber].Rotation, NewCircle.GetRotation(), 0.5);
      SegmentNumberIndices[AttachSegmentNumber].Rotation = orient;
      MakeOrientedCircle(SegmentNumberIndices[AttachSegmentNumber], true, FoundSegment);
      UE_LOG(LogTemp, Display, TEXT("Adapted the attach segment orientation"));
    }
  }
  AOverlapTransform* SpawnUtil;
  do
  {
    SpawnUtil = GetWorld()->SpawnActor<AOverlapTransform>();
  }while(!SpawnUtil);

  //if (GEngine && true)
  //{
  //  GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Black, FString::Printf(TEXT("RootNumber: %d"), NewSeg.RootNumber));
  //  GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Black, FString::Printf(TEXT("Predecessor: %d"), NewSeg.RootPredecessor));
  //  GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Black, FString::Printf(TEXT("SegmentNumber: %d"), NewSeg.SegmentNumber));
  //  GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Black, FString::Printf(TEXT("RootNumber: %d"), NewSeg.RootNumber));
  //}
  
  // TODO IM CONFUSED
  SpawnUtil->PlaceSegment = NewSeg.SegmentNumber;
  SpawnUtil->RootNum = NewSeg.RootNumber;
  //SpawnUtil->AttachToActor(this->GetAttachmentRootActor(), FAttachmentTransformRules::SnapToTargetIncludingScale);
  SpawnUtil->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetIncludingScale);
  SpawnUtil->SetActorRelativeLocation(NewSeg.Position);
  SpawnUtil->SetActorRelativeScale3D({ 1.f,1.5f * NewSeg.Diameter,1.5f * NewSeg.Diameter });
  SpawnUtil->SetActorRelativeScale3D({ 2.f * NewSeg.Diameter,2.f * NewSeg.Diameter,2.f * NewSeg.Diameter });
  //SpawnUtil->SetActorRelativeRotation(NewSeg.Length.ToOrientationQuat());
  SpawnUtil->SetActorRelativeRotation(RootSpaceTransform.GetRotation());
  SpawnUtil->PlaceSegment = NewSeg.SegmentNumber;

  
  // now we wuold add the segment
  SpawnUtil->SegmentNum = SegmentNumberIndices.Num();
  SegmentNumberIndices.Add(NewSeg);
  SegmentChangeIndices.Add(SpawnUtil);

  if (GEngine && false)
  {
    GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Black, FString::Printf(TEXT("Collision Actor Num: %d"), SpawnUtil->SegmentNum));

  }
  MakeOrientedCircle(NewSeg);
  
  for (int i = 0; i < Resolution; ++i)
  {
    Triangles.Add(NewSeg.PointStart + i);
    Triangles.Add(AttachSegment.PointStart + (((i)+1) % Resolution));
    Triangles.Add((AttachSegment.PointStart + i));
    Triangles.Add((NewSeg.PointStart + i));
    Triangles.Add(NewSeg.PointStart + (((i)+1) % Resolution));
    Triangles.Add(AttachSegment.PointStart + (((i)+1) % Resolution));
  
    Tangents[NewSeg.NormalStart + i].TangentX = Points[NewSeg.PointStart + i]
      - Points[AttachSegment.PointStart + i];
  }
  if (NewSeg.CapNumber > 0)
  {
    // spawn transform or just render a cap
    // in this case we should have the pointies
    RenderCap(NewSeg);
  }
  RootMesh->CreateMeshSection_LinearColor(0, Points, Triangles, Normals, UVCoords, VertexColors, Tangents, false);

  return SpawnUtil;
}

AOverlapTransform* URootArchitecture::SplitSegment(AOverlapTransform* Upper, AOverlapTransform* Lower)
{
  FSegment Supper = SegmentNumberIndices[Upper->SegmentNum];
  FSegment& Slower = SegmentNumberIndices[Lower->SegmentNum];


  // 1. Create new Segment with the following properties:
  FSegment SplitSegment;
  AOverlapTransform* SpawnUtil = GetWorld()->SpawnActor<AOverlapTransform>();
  SpawnUtil->SegmentNum = SegmentNumberIndices.Num();
  SpawnUtil->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetIncludingScale);
  // TODO IM CONFUSED
  //SpawnUtil->AttachToActor(this->GetAttachmentRootActor(), FAttachmentTransformRules::SnapToTargetIncludingScale);
  //    1.1 Position = Mean
  SplitSegment.Position = (Supper.Position + Slower.Position)/2.f;
  SplitSegment.Diameter = (Supper.Diameter + Slower.Diameter) / 2.f;
  SplitSegment.Color = Supper.Color;
  //SpawnUtil->SetActorRelativeScale3D({ 1.f,1.5f * SplitSegment.Diameter,1.5f * SplitSegment.Diameter });
  SpawnUtil->SetActorRelativeScale3D({ 2.f * SplitSegment.Diameter,2.f * SplitSegment.Diameter,2.f * SplitSegment.Diameter });
  SpawnUtil->SetActorRelativeLocation(SplitSegment.Position);
  //    1.2 Segment Number = Lower Segment Number
  SplitSegment.SegmentNumber = Slower.SegmentNumber;
  SpawnUtil->PlaceSegment = SplitSegment.SegmentNumber;
  //    1.3. Root Number = Upper.RootNumber
  SplitSegment.RootNumber = Supper.RootNumber;
  SpawnUtil->RootNum = SplitSegment.RootNumber;
  //    1.4. Orientation = FQuat::Slerp(orient1,orient2,0.5f);
  SplitSegment.Rotation = FQuat::Slerp(Supper.Rotation, Slower.Rotation, 0.5f);
  SpawnUtil->SetActorRelativeRotation(SplitSegment.Rotation);
  //    1.5. Pointstarts etc is at the end again
  SplitSegment.PointStart = Points.Num();
  SplitSegment.NormalStart = Points.Num();
  SplitSegment.TangentStart = Points.Num();
  SplitSegment.TriangleStart = Triangles.Num();
  SegmentNumberIndices.Add(SplitSegment);
  SegmentChangeIndices.Add(SpawnUtil);
  //    1.5. Fill in triangles and points and stuff
  auto deltaphi = 2 * PI / Resolution;
  for (int i = 0; i < Resolution; ++i)
  {
    auto angle = static_cast<float>(i) * deltaphi; // yes
    // I think yes
    // changing sine and cosine makes the circle just move the other way
    FVector iPos = (FVector(0.f, FMath::Cos(angle) * SplitSegment.Diameter,
      FMath::Sin(angle) * SplitSegment.Diameter));
    // this works here because of the Fquat slerpiness
    iPos = SplitSegment.Rotation * iPos;
    iPos += SplitSegment.Position;
    Points.Add(iPos);

    // make normal from center -> outer point
    FVector iNormal = (Points[SplitSegment.PointStart + i] - SplitSegment.Position);
    iNormal.Normalize();
    Normals.Add(iNormal);
    
    VertexColors.Add(OrderColor(SplitSegment.RootNumber));


    // we start at negative resolution with this now which means that I have to redo this completely
    // this case is the other way around: Everything that was "down", namely without Resolution
    // is not the past circle and the current one is the normal one. As such, we have to switch this around:
    // this should be switched now check again later
    Triangles.Add(SplitSegment.PointStart + i);
    Triangles.Add(Supper.PointStart + (((i)+1) % Resolution));
    Triangles.Add((Supper.PointStart + i));
    Triangles.Add((SplitSegment.PointStart + i));
    Triangles.Add(SplitSegment.PointStart + (((i)+1) % Resolution));
    Triangles.Add(Supper.PointStart + (((i)+1) % Resolution));

    Triangles[Slower.TriangleStart + (6 * i) + 2] = (SplitSegment.PointStart + i);
    Triangles[Slower.TriangleStart + (6 * i) + 1] = SplitSegment.PointStart + (((i)+1) % Resolution);
    Triangles[Slower.TriangleStart + (6 * i) + 0] = (Slower.PointStart + i);
    Triangles[Slower.TriangleStart + (6 * i) + 3] = (Slower.PointStart + i);
    Triangles[Slower.TriangleStart + (6 * i) + 4] = Slower.PointStart + (((i)+1) % Resolution);
    Triangles[Slower.TriangleStart + (6 * i) + 5] = SplitSegment.PointStart + (((i)+1) % Resolution);

    Tangents.Add({});
    Tangents[SplitSegment.NormalStart + i].TangentX = Points[SplitSegment.PointStart + i]
      - Points[Supper.PointStart + i];

    // think about moving this to its own thing because we may need to move indices
    // debug: because we go back there's no "+resolution"
    // there are normals and tangent missing now I think because we discard the last two?

    UVCoords.Add(FVector2D(((i % 2 == 0) ? 0.f : 1.f), 0.f));
  }
  // 2. Change Upper Segment:
  //    2.1. NO NEED to change triangles because they go out back
  // 3. Change Lower Segment:
  //    3.1 Segment Number = Lower Segment Number + 1
  //    3.2 FOR ALL LOWER SEGEMENTS AND ADD SEGMENT NUMBER
  Slower.SegmentNumber++;
  Lower->PlaceSegment++;
  for (FSegment& Segment : SegmentNumberIndices)
  {
    if (Segment.RootNumber == SplitSegment.RootNumber && Segment.SegmentNumber >= SplitSegment.SegmentNumber + 2)
    {
      Segment.SegmentNumber++;
    }
  }
  for (AOverlapTransform* Ovlp : SegmentChangeIndices)
  {
    if (Ovlp->RootNum == SplitSegment.RootNumber && Ovlp->PlaceSegment >= SplitSegment.SegmentNumber + 2)
    {
      Ovlp->PlaceSegment++;
    }
  }
  //    3.2. Triangles Change with new IDs (that a are in the back), copy this from above with new segment
  // 4. CHANGE ROOT SYSTEM:
  URoot** FetchRoot = RootData.FindByPredicate([RNum = SplitSegment.RootNumber](URoot* root)
  {
    return root->RootNumber == RNum;
  });
  URoot* RootToEdit = (FetchRoot != nullptr) ? *FetchRoot : nullptr;
  //    4.1. Add diameter in between the things
  //    4.2. add joint position ( maybe iterate through and add and move after )
  FVector MovePos = RootToEdit->LCJointPositions[SplitSegment.SegmentNumber];
  float MoveDiameter = RootToEdit->Diameters[SplitSegment.SegmentNumber];
  RootToEdit->LCJointPositions[SplitSegment.SegmentNumber] = SplitSegment.Position;
  RootToEdit->Diameters[SplitSegment.SegmentNumber] = SplitSegment.Diameter;
  RootToEdit->LCJointPositions.Add({});
  RootToEdit->Diameters.Add(0.0001f);
  for (int i = SplitSegment.SegmentNumber + 1; i < RootToEdit->LCJointPositions.Num(); ++i)
  {
    FVector tempv = RootToEdit->LCJointPositions[i];
    float tempf = RootToEdit->Diameters[i];
    RootToEdit->LCJointPositions[i] = MovePos;
    RootToEdit->Diameters[i] = MoveDiameter;
    MovePos = tempv;
    MoveDiameter = tempf;
  }

  // 2. Change Other Roots that may have now faulty startjoint entries
  for (URoot* root : RootData)
  {
    if (root->Predecessor == SplitSegment.RootNumber && root->StartJoint >= SplitSegment.SegmentNumber)
    {
      root->StartJoint++;
    }
  }

  RootMesh->ClearMeshSection(0);
  RootMesh->CreateMeshSection_LinearColor(0, Points, Triangles, Normals, UVCoords, VertexColors, Tangents, false);
  return SpawnUtil;
}

void URootArchitecture::Reattach(AOverlapTransform* Upper, AOverlapTransform* Lower)
{
  // the upper root will be the attachment point!

  FSegment Supper = SegmentNumberIndices[Upper->SegmentNum];
  FSegment& Slower = SegmentNumberIndices[Lower->SegmentNum];


  URoot** FetchRoot = RootData.FindByPredicate([RNum = Lower->RootNum](URoot* root)
  {
    return root->RootNumber == RNum;
  });
  URoot* RootToEdit = (FetchRoot != nullptr) ? *FetchRoot : nullptr;
  

}

int URootArchitecture::FindSegment(int RootNumber, int SegmentNumber)
{
  int f = -1;
  for (int i = 0; i < SegmentNumberIndices.Num(); ++i)
  {
    if (SegmentNumberIndices[i].RootNumber == RootNumber && SegmentNumberIndices[i].SegmentNumber == SegmentNumber)
    {
      f = i;
      break;
    }
  }
  if (GEngine && false)
  {
    GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::White, FString::Printf(TEXT("Found Segment at %d"),f));
  }
  return f;
}

const TArray<AOverlapTransform*>& URootArchitecture::FetchOverlapActors()
{
  return SegmentChangeIndices;
}

void URootArchitecture::Subdivide(AOverlapTransform* Upper, AOverlapTransform* Lower, int Subdivisions/*=2*/)
{
  FSegment Supper = SegmentNumberIndices[Upper->SegmentNum];
  FSegment& Slower = SegmentNumberIndices[Lower->SegmentNum];


  // 1. Create new Segment with the following properties:
  FSegment SplitSegment;
  AOverlapTransform* SpawnUtil = GetWorld()->SpawnActor<AOverlapTransform>();
  SpawnUtil->SegmentNum = SegmentNumberIndices.Num();
  SpawnUtil->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetIncludingScale);
  // TODO IM CONFUSED
  //SpawnUtil->AttachToActor(this->GetAttachmentRootActor(), FAttachmentTransformRules::SnapToTargetIncludingScale);
  //    1.1 Position = Mean
  SplitSegment.Position = (Supper.Position + Slower.Position)/2.f;
  SplitSegment.Diameter = (Supper.Diameter + Slower.Diameter) / 2.f;
  //SpawnUtil->SetActorRelativeScale3D({ 1.f,1.5f * SplitSegment.Diameter,1.5f * SplitSegment.Diameter });
  SpawnUtil->SetActorRelativeScale3D({ 2.f * SplitSegment.Diameter,2.f * SplitSegment.Diameter,2.f * SplitSegment.Diameter });
  SpawnUtil->SetActorRelativeLocation(SplitSegment.Position);
  //    1.2 Segment Number = Lower Segment Number
  SplitSegment.SegmentNumber = Slower.SegmentNumber;
  SpawnUtil->PlaceSegment = SplitSegment.SegmentNumber;
  //    1.3. Root Number = Upper.RootNumber
  SplitSegment.RootNumber = Supper.RootNumber;
  SpawnUtil->RootNum = SplitSegment.RootNumber;
  //    1.4. Orientation = FQuat::Slerp(orient1,orient2,0.5f);
  SplitSegment.Rotation = FQuat::Slerp(Supper.Rotation, Slower.Rotation, 0.5f);
  SpawnUtil->SetActorRelativeRotation(SplitSegment.Rotation);
  //    1.5. Pointstarts etc is at the end again
  SplitSegment.PointStart = Points.Num();
  SplitSegment.NormalStart = Points.Num();
  SplitSegment.TangentStart = Points.Num();
  SplitSegment.TriangleStart = Triangles.Num();
  SegmentNumberIndices.Add(SplitSegment);
  SegmentChangeIndices.Add(SpawnUtil);
  //    1.5. Fill in triangles and points and stuff
  auto deltaphi = 2 * PI / Resolution;
  for (int i = 0; i < Resolution; ++i)
  {
    auto angle = static_cast<float>(i) * deltaphi; // yes
    // I think yes
    // changing sine and cosine makes the circle just move the other way
    FVector iPos = (FVector(0.f, FMath::Cos(angle) * SplitSegment.Diameter,
      FMath::Sin(angle) * SplitSegment.Diameter));
    // this works here because of the Fquat slerpiness
    iPos = SplitSegment.Rotation * iPos;
    iPos += SplitSegment.Position;
    Points.Add(iPos);

    // make normal from center -> outer point
    FVector iNormal = (Points[SplitSegment.PointStart + i] - SplitSegment.Position);
    iNormal.Normalize();
    Normals.Add(iNormal);
    VertexColors.Add(OrderColor(SplitSegment.RootNumber));

    // we start at negative resolution with this now which means that I have to redo this completely
    // this case is the other way around: Everything that was "down", namely without Resolution
    // is not the past circle and the current one is the normal one. As such, we have to switch this around:
    // this should be switched now check again later
    Triangles.Add(SplitSegment.PointStart + i);
    Triangles.Add(Supper.PointStart + (((i)+1) % Resolution));
    Triangles.Add((Supper.PointStart + i));
    Triangles.Add((SplitSegment.PointStart + i));
    Triangles.Add(SplitSegment.PointStart + (((i)+1) % Resolution));
    Triangles.Add(Supper.PointStart + (((i)+1) % Resolution));

    Triangles[Slower.TriangleStart + (6 * i) + 2] = (SplitSegment.PointStart + i);
    Triangles[Slower.TriangleStart + (6 * i) + 1] = SplitSegment.PointStart + (((i)+1) % Resolution);
    Triangles[Slower.TriangleStart + (6 * i) + 0] = (Slower.PointStart + i);
    Triangles[Slower.TriangleStart + (6 * i) + 3] = (Slower.PointStart + i);
    Triangles[Slower.TriangleStart + (6 * i) + 4] = Slower.PointStart + (((i)+1) % Resolution);
    Triangles[Slower.TriangleStart + (6 * i) + 5] = SplitSegment.PointStart + (((i)+1) % Resolution);

    Tangents.Add({});
    Tangents[SplitSegment.NormalStart + i].TangentX = Points[SplitSegment.PointStart + i]
      - Points[Supper.PointStart + i];

    // think about moving this to its own thing because we may need to move indices
    // debug: because we go back there's no "+resolution"
    // there are normals and tangent missing now I think because we discard the last two?

    UVCoords.Add(FVector2D(((i % 2 == 0) ? 0.f : 1.f), 0.f));
  }
  // 2. Change Upper Segment:
  //    2.1. NO NEED to change triangles because they go out back
  // 3. Change Lower Segment:
  //    3.1 Segment Number = Lower Segment Number + 1
  //    3.2 FOR ALL LOWER SEGEMENTS AND ADD SEGMENT NUMBER
  Slower.SegmentNumber++;
  Lower->PlaceSegment++;
  for (FSegment& Segment : SegmentNumberIndices)
  {
    if (Segment.RootNumber == SplitSegment.RootNumber && Segment.SegmentNumber >= SplitSegment.SegmentNumber + 2)
    {
      Segment.SegmentNumber++;
    }
  }
  for (AOverlapTransform* Ovlp : SegmentChangeIndices)
  {
    if (Ovlp->RootNum == SplitSegment.RootNumber && Ovlp->PlaceSegment >= SplitSegment.SegmentNumber + 2)
    {
      Ovlp->PlaceSegment++;
    }
  }
  //    3.2. Triangles Change with new IDs (that a are in the back), copy this from above with new segment
  // 4. CHANGE ROOT SYSTEM:
  URoot** FetchRoot = RootData.FindByPredicate([RNum = SplitSegment.RootNumber](URoot* root)
  {
    return root->RootNumber == RNum;
  });
  URoot* RootToEdit = (FetchRoot != nullptr) ? *FetchRoot : nullptr;
  //    4.1. Add diameter in between the things
  //    4.2. add joint position ( maybe iterate through and add and move after )
  FVector MovePos = RootToEdit->LCJointPositions[SplitSegment.SegmentNumber];
  float MoveDiameter = RootToEdit->Diameters[SplitSegment.SegmentNumber];
  RootToEdit->LCJointPositions[SplitSegment.SegmentNumber] = SplitSegment.Position;
  RootToEdit->Diameters[SplitSegment.SegmentNumber] = SplitSegment.Diameter;
  RootToEdit->LCJointPositions.Add({});
  RootToEdit->Diameters.Add(0.0001f);
  for (int i = SplitSegment.SegmentNumber + 1; i < RootToEdit->LCJointPositions.Num(); ++i)
  {
    FVector tempv = RootToEdit->LCJointPositions[i];
    float tempf = RootToEdit->Diameters[i];
    RootToEdit->LCJointPositions[i] = MovePos;
    RootToEdit->Diameters[i] = MoveDiameter;
    MovePos = tempv;
    MoveDiameter = tempf;
  }

  // 2. Change Other Roots that may have now faulty startjoint entries
  for (URoot* root : RootData)
  {
    if (root->Predecessor == SplitSegment.RootNumber && root->StartJoint >= SplitSegment.SegmentNumber)
    {
      root->StartJoint++;
    }
  }

  RootMesh->ClearMeshSection(0);
  RootMesh->CreateMeshSection_LinearColor(0, Points, Triangles, Normals, UVCoords, VertexColors, Tangents, false);
  
}

AOverlapTransform* URootArchitecture::MoveSelection(AOverlapTransform* Selection, bool DirectionUp, bool AllowTraversal)
{
  AOverlapTransform* Predecessor = nullptr;
  URoot** FetchRoot = RootData.FindByPredicate([RNum = Selection->RootNum](URoot* root)
  {
    return root->RootNumber == RNum;
  });
  URoot* RootToEdit = (FetchRoot != nullptr) ? *FetchRoot : nullptr;
  if(!RootToEdit)
  {
    return nullptr;
  }
  if(DirectionUp)
  {
    if(Selection->PlaceSegment == 1 && AllowTraversal)
    {
      AOverlapTransform** FetchPred = SegmentChangeIndices.FindByPredicate([this,R=RootToEdit](AOverlapTransform* ovl)
      {
        return ovl->RootNum == R->Predecessor && ovl->PlaceSegment == R->StartJoint;
      });
      Predecessor = (FetchPred != nullptr) ? *FetchPred : nullptr;
    }
    else if(Selection->PlaceSegment > 1)
    {
      AOverlapTransform** FetchPred = SegmentChangeIndices.FindByPredicate([this,S=Selection](AOverlapTransform* ovl)
      {
        return ovl->PlaceSegment == S->PlaceSegment -1 && ovl->RootNum == S->RootNum;
      });
      Predecessor = (FetchPred != nullptr) ? *FetchPred : nullptr;
    }
  }
  else
  {
    if(! Selection->bEndCap)
    {
      AOverlapTransform** FetchPred = SegmentChangeIndices.FindByPredicate([this,S=Selection](AOverlapTransform* ovl)
      {
        return ovl->PlaceSegment == S->PlaceSegment +1 && ovl->RootNum == S->RootNum;
      });
      Predecessor = (FetchPred != nullptr) ? *FetchPred : nullptr;
    }
  }
  return Predecessor;
}

TArray<AOverlapTransform*> URootArchitecture::FindDependencies(AOverlapTransform* Activator)
{
  TArray<AOverlapTransform*> Result;
  TArray<int> DependendRoots;
  URoot** FetchRoot = RootData.FindByPredicate([RNum = Activator->RootNum](URoot* root)
  {
    return root->RootNumber == RNum;
  });
  URoot* RootToEdit = (FetchRoot != nullptr) ? *FetchRoot : nullptr;
  if (!RootToEdit)
  {
    UE_LOG(LogTemp, Warning,
      TEXT("Did not find root even though checks say it should exist. Wanted S%d<R%d<A%d"),
      Activator->RootNum, Activator->PlaceSegment, RootData.Num());
    return Result;
  }
  for (URoot* roots : RootData)
  {
    if(roots->Predecessor == Activator->RootNum && roots->StartJoint == Activator->PlaceSegment)
      DependendRoots.Add(roots->RootNumber);
  }
  if(DependendRoots.Num() > 0)
    for (AOverlapTransform* Ovlp : SegmentChangeIndices)
    {
      if (Ovlp->PlaceSegment == 0 && DependendRoots.Find(Ovlp->RootNum) != INDEX_NONE)
        Result.Add(Ovlp);
    }
  return Result;
}

void URootArchitecture::Unify(TArray<int> Segments, bool bForce /*= false*/)
{

}

void URootArchitecture::Delete(TArray<int> Segments)
{

}

void URootArchitecture::AddSectionTop()
{
  for (int i = 0; i < RootData.Num(); ++i)
  {
    if (RootData[i]->RootNumber == 1)
    {
      float copydiam = RootData[i]->Diameters[0];
      FVector copypos = RootData[i]->LCJointPositions[0] - FVector(0.f, 0.f, -1.f);
      if (RootData[i]->LCJointPositions.Num() > 1)
        copypos = RootData[i]->LCJointPositions[0] - (RootData[i]->LCJointPositions[1] - RootData[i]->LCJointPositions[0]);
      
      RootData[i]->LCJointPositions.Insert(copypos, 0);
      RootData[i]->Diameters.Insert(copydiam, 0);
    }
    else if (RootData[i]->Predecessor == 1)
    {
      RootData[i]->StartJoint++;
    }
  }
  MakeRoot(RootData, true);
}

void URootArchitecture::UpdateSections(const TArray<AOverlapTransform*>& Data, bool AdjustRadius)
{
  // first check whether there are attachments that need to be fixed
  TArray<FSegment*> Segments;
  auto deltaphi = 2 * PI / Resolution;
  for (AOverlapTransform* ovlp : Data)
  {
    FSegment& Indices = SegmentNumberIndices[ovlp->SegmentNum];
    Segments.Add(&Indices);
    FTransform normed = (ovlp->GetActorTransform() * this->GetComponentTransform().Inverse());
    Indices.Rotation = normed.GetRotation();
    Indices.Position = normed.GetLocation();
    //Indices.Diameter = normed.GetScale3D().Y / 1.5f;
    Indices.Diameter = normed.GetScale3D().Y / 2.f;
    //ovlp->SetActorScale3D({ovlp->GetActorScale3D().X,ovlp->GetActorScale3D().Y*1.5f,ovlp->GetActorScale3D().Z*1.5f});
    FQuat orient = FQuat::FindBetweenVectors({ 1,0,0 }, Indices.Length);


    URoot* RootToEdit = nullptr;

    URoot** FetchRoot = RootData.FindByPredicate([RNum = Indices.RootNumber](URoot* root)
    {
      return root->RootNumber == RNum;
    });
    RootToEdit = (FetchRoot != nullptr) ? *FetchRoot : nullptr;
    if (!RootToEdit)
    {
      UE_LOG(LogTemp, Warning,
        TEXT("Did not find root even though checks say it should exist. Wanted S%d<R%d<A%d"),
        Indices.RootNumber, Indices.SegmentNumber, RootData.Num());
      return;
    }
    RootToEdit->LCJointPositions[Indices.SegmentNumber] = Indices.Position;
    RootToEdit->Diameters[Indices.SegmentNumber] = Indices.Diameter;
    
    for (int i = 0; i < Resolution; ++i)
    {
      auto angle = static_cast<float>(i) * deltaphi; // yes
      // I think yes
      // changing sine and cosine makes the circle just move the other way
      Points[Indices.PointStart + i] = (FVector(0.f, FMath::Cos(angle) * Indices.Diameter,
        FMath::Sin(angle) * Indices.Diameter));
      Points[Indices.PointStart + i] = Indices.Rotation * Points[Indices.PointStart + i];
      Points[Indices.PointStart + i] += Indices.Position;

      // make normal from center -> outer point
      Normals[Indices.NormalStart + i] = (Points[Indices.PointStart + i] - Indices.Position);
      Normals[Indices.NormalStart + i].Normalize();

      // we start at negative resolution with this now which means that I have to redo this completely
      // this case is the other way around: Everything that was "down", namely without Resolution
      // is not the past circle and the current one is the normal one. As such, we have to switch this around:
      // this should be switched now check again later
      if (Indices.SegmentNumber > 0)
      {
//         Triangles[Indices.TriangleStart + (6 * i) + 2] = (Indices.PointStart + i) - Resolution;
//         Triangles[Indices.TriangleStart + (6 * i) + 1] = Indices.PointStart + (((i)+1) % Resolution) - Resolution;
//         Triangles[Indices.TriangleStart + (6 * i) + 0] = (Indices.PointStart + i);
//         Triangles[Indices.TriangleStart + (6 * i) + 3] = (Indices.PointStart + i);
//         Triangles[Indices.TriangleStart + (6 * i) + 4] = Indices.PointStart + (((i)+1) % Resolution);
//         Triangles[Indices.TriangleStart + (6 * i) + 5] = Indices.PointStart + (((i)+1) % Resolution) - Resolution;

        Tangents[Indices.NormalStart + i].TangentX = Points[Indices.PointStart + i]
          - Points[Indices.PointStart + i - Resolution];
      }
      else
      {
        Tangents[Indices.NormalStart + i].TangentX = Indices.Length / Indices.Length.Size();
      }

      // think about moving this to its own thing because we may need to move indices
      // debug: because we go back there's no "+resolution"
      // there are normals and tangent missing now I think because we discard the last two?

      UVCoords[Indices.NormalStart + i] = FVector2D(((i % 2 == 0) ? 0.f : 1.f), 0.f);
    }
    if (Indices.CapNumber > 0)
    {
      // spawn transform or just render a cap
      // in this case we should have the pointies
      RenderCap(Indices);
    }
  }
  if(!AdjustRadius)
  {
    RootMesh->ClearMeshSection(0);
    RootMesh->CreateMeshSection_LinearColor(0, Points, Triangles, Normals, UVCoords, VertexColors, Tangents, false);
  }
  else
  {
    AdjustRadiiForRoot(Segments);
  }
}

AOverlapTransform* URootArchitecture::FindTransformByRootNumber(int Root, int Segment)
{
  AOverlapTransform** res;
  res = SegmentChangeIndices.FindByPredicate([Root,Segment](AOverlapTransform* p) {
    if (p->RootNum == Root && p->PlaceSegment == Segment)
      return true;
    else return false;
    });
  if (res)
    return *res;
  else return nullptr;
}


const FSegment URootArchitecture::Peek(int Place)
{
  if (Place >= 0 && Place < SegmentNumberIndices.Num())
  {
    return SegmentNumberIndices[Place];
  }
  else
    return FSegment();
}

void URootArchitecture::RenderCap(const FSegment& Segment)
{
  if(!RenderCapEnabled)
   return;
  TArray<FVector> CapPoints;
  CapPoints.Init({},Resolution+1);
  TArray<FProcMeshTangent> CapTangents;
  CapTangents.Init({},Resolution+1);
  TArray<FVector> CapNormals;
  CapNormals.Init({},Resolution+1);
  TArray<int> CapTriangles;
  TArray<FVector2D> capuv;
  capuv.Init({0,0},Resolution+1);
  TArray<FColor> colors;
  colors.Init({},Resolution+1);

  CapPoints[0] = Segment.Position;
  for(unsigned int i = 0; i < (unsigned int)Resolution; ++i)
  {
    CapPoints[i+1] = Points[i+Segment.PointStart];
    CapTangents[i+1].TangentX = Points[i+Segment.PointStart]+CapPoints[0];
    CapNormals[i+1] = Segment.Length*-1;
    CapTriangles.Add(0);
    CapTriangles.Add(i+1);
    CapTriangles.Add(((i + 1) % Resolution) + 1);
    CapTriangles.Add(((i + 1) % Resolution) + 1);
    CapTriangles.Add(i+1);
    CapTriangles.Add(0);
  }
  RootMesh->CreateMeshSection(Segment.CapNumber,CapPoints,CapTriangles,CapNormals,capuv,colors,CapTangents,false);

}

const TArray<FSegment>& URootArchitecture::FetchPointList()
{
	return SegmentNumberIndices;
}

bool URootArchitecture::NeedsInitialRadius(AOverlapTransform* AttachSegmentMarker)
{
  return SegmentNumberIndices[AttachSegmentMarker->SegmentNum].CapNumber > 0;
}

// Called when the game starts or when spawned
void URootArchitecture::BeginPlay()
{
	Super::BeginPlay();
	
}

int URootArchitecture::FindRootNumber()
{
  int rootnum = -1;
  TArray<int> nums;
  for (URoot* root : RootData)
  {
    if (IsValid(root))
    {
      nums.Add(root->RootNumber);
    }
  }
  nums.Sort();
  int oldnum = -1;
  int maxnum = -1;
  for (int i = 0; i < nums.Num(); ++i)
  {
    if (i > 0)
    {
      if (nums[i] > oldnum + 1)
      {
        return nums[i] + 1;
      }
    }
    oldnum = nums[i];
    maxnum = FGenericPlatformMath::Max(nums[i],maxnum);
  }
  if (oldnum > 0)
  {
    return maxnum+1;
  }
  return 1;
}

void URootArchitecture::FixRootNumbers()
{
  for (int i = 0; i < RootData.Num();)
  {
    URoot* tr = RootData[i];
    if (!IsValid(tr))
    {
      RootData.RemoveAt(i);
    }
    else
    {
      ++i;
    }
  }
  RootData.StableSort([](const auto& a, const auto& b) {
    return a.RootNumber < b.RootNumber;
    });
  TMap<int,int> Exchanger;
  TArray<int> nums;
  for (URoot* root : RootData)
  {
    Exchanger.Add(TPair<int,int>(root->RootNumber,root->RootNumber));
    nums.AddUnique(root->RootNumber);
  }
  
  nums.Sort();

  int start = 1;
  for (auto exchange : nums)
  {
    Exchanger[exchange] = start;
    ++start;
  }

  Exchanger.Add(-1,-1);
  for (URoot* root : RootData)
  {
    root->RootNumber = Exchanger[root->RootNumber];
    root->Predecessor = Exchanger[root->Predecessor];
  }
}

FColor URootArchitecture::OrderColor(const int& OrderIndex)
{
  FColor Result{ FColor::Black };
  
  if (Orders.Contains(OrderIndex))
  {
    switch(Orders[OrderIndex])
    {
    case 1:
      Result = FColor::Red;
      break;
    case 2:
      Result = FColor::Green;
      break;
    case 3:
      Result = FColor::Yellow;
      break;
    case 4:
      Result = FColor::Cyan;
      break;
    case 5:
      Result = FColor::Orange;
      break;
    case 6:
      Result = FColor::Blue;
      break;
    case 7:
      Result = FColor::Purple;
      break;
    case 8:
      Result = FColor::White;
      break;
    case 9:
      Result = FColor::Magenta;
      break;
    default:
      Result = FColor::Black;
      break;
    }
  }
  return Result;
}

void URootArchitecture::BuildSegment(int SegmentNumber)
{

}

void URootArchitecture::Norm(FVector Max, FVector Min)
{
  for (URoot* root : RootData)
  {
    for (int i = 0; i < root->LCJointPositions.Num(); ++i)
    {
      root->LCJointPositions[i] = root->LCJointPositions[i].BoundToBox(Min, Max);
    }
  }
  MakeRoot(RootData, true);
}

void URootArchitecture::RemakeRoot()
{
  UE_LOG(LogTemp, Display,TEXT("Remaking the root system"));
  FixRootNumbers();
  int pointstart = 0, trianglestart = 0;
  int rootnumber = 0, segmentnumber = 0;
  int cappy = 1;
  auto deltaphi = 2 * PI / Resolution;

  SegmentNumberIndices.StableSort([](FSegment a, FSegment b){
    return (a.RootNumber == b.RootNumber) ? a.SegmentNumber < b.SegmentNumber : a.RootNumber < b.RootNumber;
  });
  SegmentChangeIndices.StableSort([](AOverlapTransform& a, AOverlapTransform& b) {
      return (a.RootNum < b.RootNum) ? a.PlaceSegment < b.PlaceSegment : a.RootNum < b.RootNum;
  });



  int o = 0;
  RootMesh->ClearAllMeshSections();
  decltype(Points) nPoints;
  decltype(Tangents) nTangents;
  decltype(Triangles) nTriangles;
  decltype (Normals) nNormals;
  decltype (UVCoords) nUV;
  decltype(VertexColors) nVC;


  Triangles.Init(0, SegmentNumberIndices.Num()*2*3*Resolution);
  Points.Init({0,0,0}, SegmentNumberIndices.Num()*Resolution);
  Normals.Init({0,0,0}, SegmentNumberIndices.Num() * Resolution);
  VertexColors.Init({0,0,0}, SegmentNumberIndices.Num() * Resolution);
  UVCoords.Init({0,0}, SegmentNumberIndices.Num() * Resolution);
  Tangents.Init({0,0,0}, SegmentNumberIndices.Num() * Resolution);

  for (int s = 0; s < SegmentNumberIndices.Num(); ++s)
  {
    FSegment& Segment = SegmentNumberIndices[s];
    FQuat orient = FQuat::FindBetweenVectors({ 1,0,0 }, Segment.Length);
    AOverlapTransform* SpawnUtil = SegmentChangeIndices[s];
    SpawnUtil->SegmentNum = s;
    SpawnUtil->PlaceSegment = Segment.SegmentNumber;
    SpawnUtil->RootNum = Segment.RootNumber;
    //SpawnUtil->AttachToActor(this->GetAttachmentRootActor(), FAttachmentTransformRules::SnapToTargetIncludingScale);
    SpawnUtil->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetIncludingScale);
    SpawnUtil->SetActorRelativeLocation(Segment.Position);
    //SpawnUtil->SetActorRelativeScale3D({ 1.f,1.5f * Segment.Diameter,1.5f * Segment.Diameter });
    SpawnUtil->SetActorRelativeScale3D({ 2.f * Segment.Diameter,2.f * Segment.Diameter,2.f * Segment.Diameter });
    SpawnUtil->SetActorRelativeRotation(Segment.Rotation);
    if(s+1 == SegmentNumberIndices.Num() || SegmentNumberIndices[s+1].RootNumber != Segment.RootNumber || SegmentNumberIndices[s].SegmentNumber == 0)
      Segment.CapNumber = cappy++;

    Segment.PointStart = pointstart;
    Segment.TangentStart = pointstart;
    Segment.TriangleStart = trianglestart;
    Segment.NormalStart = pointstart;
    bool bHasTriangles = Segment.SegmentNumber > 0;
    // copy points down
    for (int i = 0; i < Resolution; ++i)
    {
      auto angle = static_cast<float>(i) * deltaphi; 
      Points[Segment.PointStart + i] = (FVector(0.f, FMath::Cos(angle) * Segment.Diameter,
        FMath::Sin(angle) * Segment.Diameter));
      Points[Segment.PointStart + i] = Segment.Rotation * Points[Segment.PointStart + i];
      Points[Segment.PointStart + i] += Segment.Position;
      Normals[Segment.NormalStart + i] = (Points[Segment.PointStart + i] - Segment.Position);
      Normals[Segment.NormalStart + i].Normalize();

      VertexColors.Add(OrderColor(Segment.RootNumber));


      if (Segment.SegmentNumber > 0)
      {
        Triangles[Segment.TriangleStart + (6 * i) + 2] = (Segment.PointStart + i) - Resolution;
        Triangles[Segment.TriangleStart + (6 * i) + 1] = Segment.PointStart + (((i)+1) % Resolution) - Resolution;
        Triangles[Segment.TriangleStart + (6 * i) + 0] = (Segment.PointStart + i);
        Triangles[Segment.TriangleStart + (6 * i) + 3] = (Segment.PointStart + i);
        Triangles[Segment.TriangleStart + (6 * i) + 4] = Segment.PointStart + (((i)+1) % Resolution);
        Triangles[Segment.TriangleStart + (6 * i) + 5] = Segment.PointStart + (((i)+1) % Resolution) - Resolution;

        Tangents[Segment.NormalStart + i].TangentX = Points[Segment.PointStart + i]
          - Points[Segment.PointStart + i - Resolution];
      }
      else
      {
        Tangents[Segment.NormalStart + i].TangentX = Segment.Length / Segment.Length.Size();
      }
      UVCoords[Segment.NormalStart + i] = FVector2D(((i % 2 == 0) ? 0.f : 1.f), 0.f);
//       nPoints.Add(Points[Segment.PointStart + i]);
//       nTangents.Add(Tangents[Segment.PointStart + i]);
//       nNormals.Add(Normals[Segment.PointStart + i]);
//       nUV.Add(UVCoords[Segment.PointStart + i]);
//       nVC.Add(VertexColors[Segment.PointStart + i]);
// 
// 
//       if (bHasTriangles) // we need to copy triangles
//       {
//         nTriangles.Add((pointstart + i));
//         nTriangles.Add(pointstart + (((i)+1) % Resolution) - Resolution);
//         nTriangles.Add((pointstart + i) - Resolution);
//         nTriangles.Add((pointstart + i));
//         nTriangles.Add(pointstart + (((i)+1) % Resolution));
//         nTriangles.Add(pointstart + (((i)+1) % Resolution) - Resolution);
//       }
    }
    if (Segment.CapNumber > 0)
    {
      RenderCap(Segment);
    }
    pointstart += Resolution;
    if(bHasTriangles)
      trianglestart += Resolution*6;
  }

  RootMesh->CreateMeshSection_LinearColor(0,Points,Triangles,Normals,UVCoords,VertexColors,Tangents,false);
}

// Called every frame
void URootArchitecture::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime,TickType,ThisTickFunction);

}

void URootArchitecture::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  Super::EndPlay(EndPlayReason);
  for (URoot* root : RootData)
  {
    if(root == nullptr || !IsValid(root))
    {
      continue;
    }
    root->ClearFlags(EObjectFlags::RF_NoFlags);
    root->ConditionalBeginDestroy();
  }
  //RootData.Empty();
}

