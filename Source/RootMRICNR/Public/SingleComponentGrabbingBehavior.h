// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../GrabbingBehaviorComponent.h"
#include "SingleComponentGrabbingBehavior.generated.h"

/**
 * 
 */
UCLASS()
class ROOTMRICNR_API USingleComponentGrabbingBehavior : public UGrabbingBehaviorComponent
{
	GENERATED_BODY()

public:

		virtual void HandleNewPositionAndDirection(FVector position, FQuat orientation) override;

    bool bInit = false;
    FVector OriginHandPosition;
    FQuat OriginHandOrientation;

  FVector OriginPosition;
  FQuat OriginOrientation;
};
