// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "../Grabable.h"
#include "../Clickable.h"
#include "Rayable.h"
#include "RootSegmentDual.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ROOTMRICNR_API URootSegmentDual : public USceneComponent, public IGrabable, public IClickable, public IRayable
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	URootSegmentDual();

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rendering")
    class URootCone* RootSegmentMesh;
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rendering")
    UStaticMeshComponent* RootJoint;
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Root")
    int RootNumber;

  UFUNCTION()
    int NumberOfAttachedRootSegments();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

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

  UFUNCTION()
    void RegisterRuntimeSpawn();

  //! @return null if attached to nothing OR DrawSpace, pointer otherwise
  UFUNCTION()
    URootSegmentDual* HasRootAttachment();

private:

  void RecenterChildren(URootSegmentDual* Originator);

  TArray<URootSegmentDual*> AttachedChildRoots;
  URootSegmentDual* AttachParentRoot = nullptr;
};
