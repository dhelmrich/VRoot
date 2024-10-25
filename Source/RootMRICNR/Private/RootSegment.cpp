// Fill out your copyright notice in the Description page of Project Settings.


#include "RootSegment.h"
#include "Root.h"
#include "RootCone.h"
#include "SingleComponentGrabbingBehavior.h"
#include <UObject/ConstructorHelpers.h>

URootSegment::URootSegment(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
//   GrabBehavior = CreateDefaultSubobject<USingleComponentGrabbingBehavior>(TEXT("Grabbing Behavior"));
//   GrabBehavior->SetupAttachment(this);
//   GrabBehavior->SetVisibility(false);
  RootSegmentMesh = CreateDefaultSubobject<URootCone>(TEXT("RootMesh"));
  //RootJoint = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RootJoint"));
//   static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereAsset(TEXT("StaticMesh'/Game/SpawnfriendlySphere'"));
//   if (SphereAsset.Succeeded())
//   {
//     RootJoint->SetStaticMesh(SphereAsset.Object);
//   }
  RootSegmentMesh->resolution = 8;
  RootSegmentMesh->NormalSplitting = false;
  RootSegmentMesh->StartRadius = 0.5f;
  RootSegmentMesh->EndRadius = 0.5f;
  RootSegmentMesh->SetupAttachment(this);
  RootSegmentMesh->SetVisibility(true);
  RootSegmentMesh->SetShouldUpdatePhysicsVolume(false);
  RootSegmentMesh->SetActive(true);
  RootSegmentMesh->ConeMesh->SetUseCCD(false);
  RootSegmentMesh->ConeMesh->SetShouldUpdatePhysicsVolume(false);
//RootJoint->SetupAttachment(this);
//   RootJoint->SetVisibility(true);
//   RootJoint->SetActive(true);
//   RootJoint->SetShouldUpdatePhysicsVolume(false);
//   RootJoint->SetUseCCD(false);

  SetActive(true);
  SetVisibility(true);
  SetComponentTickEnabled(false);
}

int URootSegment::NumberOfAttachedRootSegments()
{
  int num = 0;
  for (USceneComponent* child : GetAttachChildren())
  {

    if (GEngine && false)
    {
      GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, (child->GetClass()->GetFName().ToString()));
    }
    if (child->IsA(URootSegment::StaticClass()))
      num++;
  }
  return FGenericPlatformMath::Max(num-1,0);
}

void URootSegment::SignalCollisionUpdate(ECollisionEnabled::Type CollisionUpdate)
{
  //RootJoint->SetCollisionEnabled(CollisionUpdate);
  RootSegmentMesh->ConeMesh->SetCollisionEnabled(CollisionUpdate);
  URootSegment* segmesh;
  for (auto comp : this->GetAttachChildren())
  {
    segmesh = Cast<URootSegment>(comp);
    if (segmesh && segmesh != this)
    {
      segmesh->SignalCollisionUpdate(ECollisionEnabled::NoCollision);
    }
  }
}

void URootSegment::RegisterRuntimeSpawn()
{
  RootSegmentMesh->RegisterAtRuntime();
  //RootJoint->RegisterComponent();
  RootSegmentMesh->RegisterComponent();
  RootSegmentMesh->AttachToComponent(this,FAttachmentTransformRules::KeepWorldTransform);

  //RootJoint->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);
  //GrabBehavior->RegisterComponent();
}

URootSegment* URootSegment::HasRootAttachment()
{
  URootSegment* parent = Cast<URootSegment>(GetAttachParent());
  return parent;
}

void URootSegment::RecenterChildren(URootSegment* Originator)
{
  FVector rootlocation = RootSegmentMesh->GetComponentLocation();
  rootlocation += ((RootSegmentMesh->GetComponentScale().X)
    * (RootSegmentMesh->GetForwardVector()));
  for (USceneComponent* child : GetAttachChildren())
  {
    if (!child) continue;
    URootSegment* rootchild = Cast<URootSegment>(child);
    if (rootchild && rootchild != this && rootchild != Originator)
    {
      // child needs position updated and has to have new end position
      // we record the end position by using the DrawSpace calculation from before
      FVector childend = rootchild->RootSegmentMesh->GetComponentLocation();
      childend += ((rootchild->RootSegmentMesh->GetComponentScale().X)
        * (rootchild->RootSegmentMesh->GetForwardVector()));
      FVector diff = childend - rootlocation;
      rootchild->RootSegmentMesh->SetWorldScale3D(
        FVector(
          // Difference in Position is the X scale
          (diff).Size(),
          rootchild->RootSegmentMesh->GetComponentScale().Y,
          rootchild->RootSegmentMesh->GetComponentScale().Z));
      rootchild->RootSegmentMesh->SetWorldRotation(
        diff.ToOrientationRotator());
      rootchild->RootSegmentMesh->SetWorldLocation(rootlocation);
     // rootchild->RootJoint->SetWorldLocation(rootlocation);
    }
  }
}

void URootSegment::OnClickSignal_Implementation(FVector WorldPositionOfClick)
{
  auto relative_position = WorldPositionOfClick - GetComponentLocation();
  if (relative_position.Size() < (GetComponentScale().X/2.f))
  {
    bAppendAfter = false;
  }
  else
  {
    bAppendAfter = true;
  }
  bSelected = true;
}

void URootSegment::OnGrabbed_Implementation()
{
  if (GEngine && false)
  {
    GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, TEXT("GrabbedOnSegment"));
  }
//   GrabBehavior->OriginPosition = GetComponentLocation();
//   GrabBehavior->OriginOrientation = GetComponentTransform().GetRotation();
  URootSegment* check_component_class = Cast<URootSegment>(GetAttachParent());
  if (check_component_class)
    AttachParentRoot = check_component_class;
  TArray<USceneComponent*> atc = GetAttachChildren();
  for (USceneComponent* child_component : atc)
  {
    check_component_class = Cast<URootSegment>(child_component);
    if (check_component_class && !(child_component->GetFName().IsEqual(GetFName())))
    {
      AttachedChildRoots.Add(check_component_class);
    }
  }
}

void URootSegment::OnReleased_Implementation()
{
  // self refit
  //RootJoint->SetWorldLocation(RootSegmentMesh->GetComponentLocation());
  if (AttachParentRoot)
  {
    // PARENT REFIT
    // Parent needs X-Scale and orientation Update
    FVector diff_to_parent = RootSegmentMesh->GetComponentLocation()
      - AttachParentRoot->RootSegmentMesh->GetComponentLocation();
    AttachParentRoot->RootSegmentMesh->SetWorldScale3D(
      FVector(
        // Difference in Position is the X scale
        (diff_to_parent).Size(),
        AttachParentRoot->RootSegmentMesh->GetComponentScale().Y,
        AttachParentRoot->RootSegmentMesh->GetComponentScale().Z));
    // Rotate the component towards the new end point
    AttachParentRoot->RootSegmentMesh->SetWorldRotation(
      diff_to_parent.ToOrientationRotator());
    AttachParentRoot->RecenterChildren(this);
    // END PARENT REFIT
  }

  // CHILD REFIT
  FVector rootlocation = RootSegmentMesh->GetComponentLocation();
  rootlocation += ((RootSegmentMesh->GetComponentScale().X)
    * (RootSegmentMesh->GetForwardVector()));
  for (URootSegment* child : AttachedChildRoots)
  {
    if (!child)
      continue;
    // child needs position updated and has to have new end position
    // we record the end position by using the DrawSpace calculation from before
    FVector childend = child->RootSegmentMesh->GetComponentLocation();
    childend += ((child->RootSegmentMesh->GetComponentScale().X)
      * (child->RootSegmentMesh->GetForwardVector()));

    FVector diff = childend - rootlocation;
    child->RootSegmentMesh->SetWorldScale3D(
      FVector(
        // Difference in Position is the X scale
        (diff).Size(),
        child->RootSegmentMesh->GetComponentScale().Y,
        child->RootSegmentMesh->GetComponentScale().Z));
    child->RootSegmentMesh->SetWorldRotation(
      diff.ToOrientationRotator());
    child->RootSegmentMesh->SetWorldLocation(rootlocation);
    //child->RootJoint->SetWorldLocation(rootlocation);
  }
  // END CHILD REFIT
  //GrabBehavior->bInit = false;
}

void URootSegment::OnRay_Implementation(FVector WorldPosition)
{

}

