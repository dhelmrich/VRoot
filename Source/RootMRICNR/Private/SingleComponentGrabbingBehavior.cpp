// Fill out your copyright notice in the Description page of Project Settings.


#include "SingleComponentGrabbingBehavior.h"
#include "RootSegment.h"
#include "RootCone.h"

void USingleComponentGrabbingBehavior::HandleNewPositionAndDirection(FVector position, FQuat orientation)
{
  if (!bInit)
  {
    OriginHandOrientation = orientation;
    OriginHandPosition = position;
    bInit = true;
  }
  else
  {
    FVector distance = OriginPosition - OriginHandPosition;
    FVector displacement = position - OriginHandPosition;
    FQuat disrotation = orientation * OriginHandOrientation.Inverse();
    distance = orientation.RotateVector(FVector(distance.Size(), 0, 0));
    URootSegment* parroot = Cast<URootSegment>(this->GetAttachParent());
    parroot->RootSegmentMesh->SetWorldLocation(distance);
    //parroot->RootJoint->SetWorldLocation(OriginPosition + displacement);
    parroot->RootSegmentMesh->SetWorldRotation(OriginOrientation * disrotation * disrotation);
  }
}
