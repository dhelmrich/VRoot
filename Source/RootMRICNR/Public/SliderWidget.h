// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "../Grabable.h"
#include "SliderWidget.generated.h"

/**
 * 
 */
UCLASS()
class ROOTMRICNR_API ASliderWidget : public AStaticMeshActor, public IGrabable
{
	GENERATED_BODY()
	
public:
  void OnGrabbed_Implementation() override;
  void SetReleaseCallback(TFunction<void(const FTransform&)> InCallbackRef);

  void OnReleased_Implementation() override;
  FTransform OriginPosition;
  FVector BeginPlayLocation;

protected:
    virtual void BeginPlay() override;

private:

  TFunction<void(const FTransform&)> CallbackRef;
};
