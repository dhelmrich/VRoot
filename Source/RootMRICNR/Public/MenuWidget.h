// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "../Grabable.h"
#include "MenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class ROOTMRICNR_API AMenuWidget : public AStaticMeshActor, public IGrabable
{
	GENERATED_BODY()
	protected:
public:
	void OnGrabbed_Implementation() override;
	FTransform OriginPosition;

	UPROPERTY() bool bNeedReleaseOfComponents = false;
	UPROPERTY(EditAnywhere) bool bFixBetween = false;

	void OnReleased_Implementation() override;

};
