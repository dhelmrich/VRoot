// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "Root.h"
#include "OverlapTransform.h"
#include "Components/SceneComponent.h"
#include "RootArchitecture.generated.h"

USTRUCT()
struct FSegment
{
	GENERATED_BODY()
	FVector Position;
	FVector Length;
	FColor Color;
	FQuat Rotation;
	float Diameter;
	int TriangleStart;
	int PointStart;
	int NormalStart;
	int TangentStart;
	int RootNumber;
	int RootPredecessor;
	int SegmentNumber;
	int CapNumber{-1};
};

UCLASS(ClassGroup = Rendering, meta = (BlueprintSpawnableComponent), Blueprintable)
class ROOTMRICNR_API URootArchitecture : public USceneComponent
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	URootArchitecture();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	int Resolution = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Config, Category = "Mesh")
	bool RenderCapEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Config, Category = "Mesh")
		bool NormalizePreviousSegment = false;
		
  void AdjustRadiiForRoot(const TArray<FSegment*>& ChangedSegments);

	URoot* FindRootForSegment(const FSegment& Segment);
	URoot* FindParentRoot(URoot* Root);

	UFUNCTION(BlueprintCallable)
	void InitRootSystem();

	UFUNCTION(BlueprintCallable)
	void CreateUV();

	void CreateSegment();
	void MakeOrientedCircle(const FSegment& Segment, bool InPlace = false, FSegment* TriangleCheck = nullptr);

	UFUNCTION()
	void NearestPointToSelection(FVector selectionHit);

	UFUNCTION(BlueprintCallable)
	void MakeRoot(const TArray<URoot*>& Data, bool bSkipInit = false);
	UFUNCTION(BlueprintCallable)
		void MakeEmpty();

	UFUNCTION(BlueprintCallable)
		void Norm(FVector Max, FVector Min);
	
  void RemoveSections(const TArray<AOverlapTransform*>& Selection);

  
  void RemoveSections(int Root, int HighestSegment = 0);

	UFUNCTION(BlueprintCallable)
	const TArray<URoot*>& FetchSubmissionData(bool Correction = false);

	UFUNCTION(BlueprintCallable)
		AOverlapTransform* CreateNewSegment(FTransform NewCircle, int AttachSegmentNumber = -1);

		UFUNCTION(BlueprintCallable)
      AOverlapTransform* SplitSegment(AOverlapTransform* Upper, AOverlapTransform* Lower);

    UFUNCTION(BlueprintCallable)
      void Reattach(AOverlapTransform* Upper, AOverlapTransform* Lower);

	int FindSegment(int RootNumber, int SegmentNumber);

  const TArray<AOverlapTransform*>& FetchOverlapActors();

	UFUNCTION(BlueprintCallable, Category = "Mesh")
		void Subdivide(AOverlapTransform* Upper, AOverlapTransform* Lower, int Subdivisions=2);

	UFUNCTION(BlueprintCallable, Category = "Root")
	  AOverlapTransform* MoveSelection(AOverlapTransform* Selection, bool DirectionUp, bool AllowTraversal = true);

	UFUNCTION()
		TArray<AOverlapTransform*> FindDependencies(AOverlapTransform* Activator);

	UFUNCTION()
		void Unify(TArray<int> Segments, bool bForce = false);

	UFUNCTION(BlueprintCallable)
		void Delete(TArray<int> Segments);

	UFUNCTION()
		void AddSectionTop();

  UFUNCTION(BlueprintCallable, Category = "Root")
    void UpdateSections(const TArray<AOverlapTransform*>& Data, bool AdjustRadius = false);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UProceduralMeshComponent* RootMesh;

	UFUNCTION()
		AOverlapTransform* FindTransformByRootNumber(int Root, int Segment);

	UFUNCTION()
		const FSegment Peek(int Place);

	void RenderCap(const FSegment& Segment);

	const TArray<FSegment>& FetchPointList();
	FTransform OriginalDrawPoint;

	bool NeedsInitialRadius(AOverlapTransform* AttachSegmentMarker);


  // this method is called internally, with a correct set of segtions
  // an incorrect set of index entries and an incorrect set of mesh data
	UFUNCTION(BlueprintCallable, Category = "Root")
  void RemakeRoot();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	int FindRootNumber();
	void FixRootNumbers();

	// raw data
	// this plan includes updating the root as we process
	UPROPERTY()
	TArray<URoot*> RootData;
	bool bModified = false;

	// procedural mesh data
	TArray<int> Triangles;
  TArray<FVector> Points;
	TArray<FVector> Normals; // no splitting available here
  TArray<FTransform> Origins; // for interaction with pawn either this or Actors
  TArray<FLinearColor> VertexColors;
  TArray<FVector2D> UVCoords;
  TArray<FProcMeshTangent> Tangents;
  TArray<FSegment> SegmentNumberIndices;
	TArray<AOverlapTransform*> SegmentChangeIndices;

	FColor OrderColor(const int& RootNumber);

	/************************************************************************/
	/* PROCEDURAL MESH METHODS                                              */
	/************************************************************************/
	void BuildSegment(int SegmentNumber);



public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


private:

	TMap<int, int> Orders;

public:
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

};
