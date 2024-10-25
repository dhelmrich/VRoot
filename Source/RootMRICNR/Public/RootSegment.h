// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "../Grabable.h"
#include "../Clickable.h"
#include "Rayable.h"
#include <Chaos/Transform.h>
#include "RootSegment.generated.h"



/**
 * 
 */
UCLASS(ClassGroup = Rendering, meta = (BlueprintSpawnableComponent), Blueprintable)
class ROOTMRICNR_API URootSegment : public USceneComponent, public IGrabable, public IClickable, public IRayable
{
	GENERATED_BODY()


public:
  URootSegment(const FObjectInitializer& ObjectInitializer);
	  
		// THIS WILL ALSO BE A ROOT CONE BECAUSE WHY TF WOULD I HAVE AN ADDITOINAL
		// COMPONENT THAT I CANT EVEN CAST INTO ONE ANOTHER
		UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rendering")
      class URootCone* RootSegmentMesh;

// 		UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
// 		  class USingleComponentGrabbingBehavior* GrabBehavior;

//     UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rendering")
//       UStaticMeshComponent* RootJoint;
		UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Root")
			int RootNumber;

		UFUNCTION()
		  int NumberOfAttachedRootSegments();

public:

	UPROPERTY()
	bool bSelected = false;

	UPROPERTY()
	FVector EndPoint;

	UPROPERTY()
	bool bAppendAfter = true;

	void OnClickSignal_Implementation(FVector WorldPositionOfClick) override;


	void OnGrabbed_Implementation() override;


	void OnRay_Implementation(FVector WorldPosition) override;

	void OnReleased_Implementation() override;

	void SignalCollisionUpdate(ECollisionEnabled::Type CollisionUpdate);

	UFUNCTION()
	void RegisterRuntimeSpawn();

	//! @return null if attached to nothing OR DrawSpace, pointer otherwise
	UFUNCTION()
	URootSegment* HasRootAttachment();

private:

	void RecenterChildren(URootSegment* Originator);

	TArray<URootSegment*> AttachedChildRoots;
	URootSegment* AttachParentRoot = nullptr;
};
