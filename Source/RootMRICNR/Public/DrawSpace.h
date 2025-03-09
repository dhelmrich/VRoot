// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RootSegment.h"
#include "../Grabable.h"
#include "RootSegmentDual.h"
#include "Root.h"
#include "DrawSpace.generated.h"

UENUM()
enum class EDSDiameterMode
{
  dia_mode_joint UMETA(DisplayName = "Diameter Mode Joint"),
  dia_mode_segment UMETA(DisplayName = "Diameter Mode Segment")
};

class AOverlapTransform;
class URootCone;
class UProceduralMeshComponent;

UCLASS()
class ROOTMRICNR_API ADrawSpace : public AActor, public IGrabable
{
  GENERATED_BODY()

public:
  // Sets default values for this actor's properties
  ADrawSpace();

  UPROPERTY(BlueprintReadOnly, Category = "UI")
    UStaticMeshComponent* HightlightActor;
  UFUNCTION(BlueprintCallable, Category = "UI")
    void SetHightlightActor(UStaticMeshComponent* hightlight);
  UFUNCTION(BlueprintCallable, Category = "UI")
    UStaticMeshComponent* GetHighlightActor();

  UFUNCTION(BlueprintCallable, Category = "Root")
    USceneComponent* GetAttachRoot();

  UFUNCTION(BlueprintCallable, Category = "Root")
    void UpdateSections(const TArray<AOverlapTransform*>& Data, bool bFix = false, bool bAdjustRadius = false);

  UFUNCTION(BlueprintCallable, Category = "")
    void InitializeRootSystem();

  UFUNCTION(BlueprintCallable, Category = "UI")
    void StopAddingSegment();

  // Then this would always spawn a new root while the other
  // method would first try to append a root segment
  // but if no reference to a root segment is found
  // then a new one will be spawned with this method
  // to that end lol the location will always be transmitted
  UFUNCTION(BlueprintCallable, Category = "Draw")
    URootCone* AddRootSegmentAtLocation(FVector loc, bool forcecreatenew = false);

  UFUNCTION(BlueprintCallable, Category = "Root")
    FTransform ConvertTransformToDrawspace(FTransform UnitTransform);
  UFUNCTION()
    TArray<AOverlapTransform*> FindDependencies(AOverlapTransform* Activator);

  UFUNCTION(BlueprintCallable, Category = "Root")
    FTransform ConvertTransformToUnit(FTransform DrawSpaceTransform);

  UFUNCTION()
    void OverrideRootSystem(const TArray<URoot*>& Data);

    /************************************************************************/
    /* new edit mode variables and functions                                */
    /************************************************************************/

    // A root system is initialized if it has at least two connected segments (joints)
    UPROPERTY()
      bool bInitializedSystem;


    UFUNCTION()
      AOverlapTransform* FindTransformByRootNumber(int Root, int Segment);


    UFUNCTION()
      void SyncRadius(AOverlapTransform* Selection);


    UFUNCTION()
      bool HasRoot();


    /************************************************************************/
    /* eeeeeeeeeeeend                                                       */
    /************************************************************************/

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Root")
    URootSegment* SegmentTreeMarker = nullptr;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Root")
    URootSegment* LastUpdated = nullptr;

  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Root")
    class URootArchitecture* RootSystemMesh;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Root")
    TArray<int> RootNumbers;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Draw")
    bool bStartSelect = false;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Config, Category = "Draw")
    bool bCenterRootSystem = true;

    // DEBUG VALUES

  UPROPERTY(EditAnywhere, Category = "Debug")
    bool bMakeRandoMRoot = false;

  UPROPERTY(EditAnywhere, Category = "Debug", AdvancedDisplay, meta=(ClampMin = 2))
    int iHowMany = 300;

  UPROPERTY(EditAnywhere, Category = "Debug", AdvancedDisplay)
    FVector RandomRangeLow {
    40.f, 40.f, 40.f
  };

  UPROPERTY(EditAnywhere, Category = "Debug", AdvancedDisplay)
    FVector RandomRangeHigh {
    40.f, 40.f, 40.f
  };

  UPROPERTY(EditAnywhere, Category = "Debug", AdvancedDisplay)
    bool XSign = true;

  UPROPERTY(EditAnywhere, Category = "Debug", AdvancedDisplay)
    bool YSign = true;

  UPROPERTY(EditAnywhere, Category = "Debug", AdvancedDisplay)
    bool ZSign = false;


  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Draw")
    bool bAutoAppend = true;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resolution")
    int ConeResolution = 8;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Draw")
    float CurrentStoredRadius = 4.f;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Draw")
    float RadiusDefault = 4.f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Draw")
    EDSDiameterMode DiameterMode = EDSDiameterMode::dia_mode_segment;

  UFUNCTION(BlueprintCallable, Category = "Root")
    void GenerateRandomRoot(int nsegments = 50);

  UFUNCTION(BlueprintCallable, Category = "Root")
    void PrepareRootSystemForTransmission();

    bool NeedsInitialRadius(AOverlapTransform* AttachSegmentMarker);

  AOverlapTransform* MoveSelection(AOverlapTransform * Selection, bool DirectionUp);


    const TArray<URoot*>& ParseSegmentsIntoRootsystem(bool Correction = false);

  UFUNCTION(BlueprintCallable, Category = "Draw")
    FVector GetNextPosition();

  UFUNCTION(BlueprintCallable, Category = "Draw")
    void StopCollisionUpdate(bool ExludeCaps = false);

  UFUNCTION(BlueprintCallable, Category = "Draw")
    void RestartCollision();

    void UpdateBounds(FVector Bounds);
    UFUNCTION(BlueprintCallable, Category = "Root")
    void ResetGPU();
    void NormRootSystem();
    void MakeIsosurface(const TArray<FVector>& Points,
                        const TArray<FVector>& Normals,
                        const TArray<int>& Triangles);

    void DeleteSegments(const TArray<AOverlapTransform*>& Selection);
  void ZeroRootSystem();

protected:
  // Called when the game starts or when spawned
  virtual void BeginPlay() override;

  TMap<unsigned int,TArray<unsigned int>> RootNumberTree;

  int GetNewRootNumber();
  unsigned int counter;
  void GenerateOneRoot();

  UProceduralMeshComponent* NetworkSurface = nullptr;
  UMaterialInterface* SurfaceMaterial;


public:
  // Called every frame
  virtual void Tick(float DeltaTime) override;
public:
  void OnGrabbed_Implementation() override;


  void OnReleased_Implementation() override;

  void SignalRelease(FVector EndPos);
  AOverlapTransform* SplitSegment(AOverlapTransform* Upper, AOverlapTransform* Lower);

  void PrepareSegmentFromIndicator(AOverlapTransform* AttachBox, FTransform IndicatorPosition);
  AOverlapTransform* MakeSegmentFromIndicator(FTransform EndPos);
  int ActivatedSegmentNumber{-1};
};
