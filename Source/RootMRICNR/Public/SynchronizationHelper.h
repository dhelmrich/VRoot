// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Containers/Queue.h"
#include "ZMQHandle.h"
#include "SynchronizationHelper.generated.h"

// forward declarations
class AFileTargetBox;
class ASliderWidget;

UCLASS(Config = Game)
class ROOTMRICNR_API ASynchronizationHelper : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
  ASynchronizationHelper();
  //ASynchronizationHelper(const FObjectInitializer& ObjectInitializer);

  void MakeFileNameTile(FString FileName, unsigned int ID);



  UPROPERTY(VisibleAnywhere, Config)
    int Port = 12575;
  UPROPERTY(VisibleAnywhere, Config)
    FString ServerName = TEXT("localhost");

  UPROPERTY(VisibleAnywhere)
    UZMQHandle* ServerConnect;

  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UX")
    class ADrawSpace* RootSink;

  UPROPERTY(VisibleAnywhere, BlueprintReadWrite,Category = "UX")
    AFileTargetBox* UpButton;

  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UX")
    AFileTargetBox* DownButton;

  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UX")
    AFileTargetBox* FetchButton;

  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UX")
    AFileTargetBox* SendButton;

  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UX")
    AFileTargetBox* IsoMaxSign;

  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UX")
    AFileTargetBox* IsoMinSign;

  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UX")
    AFileTargetBox* IsoCurSign;

  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UX")
    AFileTargetBox* RootLock;

  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UX")
    AFileTargetBox* ReDrawRoot;

  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UX")
    AFileTargetBox* ToggleMaxLength;

  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UX")
    AFileTargetBox* Undo;


  UFUNCTION(BlueprintCallable,Category = "Network")
   void SendAfterActionTaken();

  ASliderWidget* IsoSlider;

  UFUNCTION(BlueprintCallable, Category = "UX")
    void SetSliderWidget(ASliderWidget* InIsoSlider);

  UFUNCTION(BlueprintCallable, Category = "Debug")
    void StopZMQThread();

  UFUNCTION(BlueprintCallable,Category = "UX")
    void SetupCallbacks();

  void UpdateTiles(int offset = 0);


    void ShowIsosurface(const TArray<FVector>& Points, 
                       const TArray<FVector>& Normals,
                       const TArray<int>& Triangles);
    void RelayBounds(FVector Bounds);

    void ShowState();

    void ShowFiles(TArray<FString> FileNames);

    void ShowRoots(const TArray<class URoot*>& Data);

    void UpdateRange(float l, float u);

    UFUNCTION(BlueprintCallable, Category = "UX")
  void AttachFileTiles(const TArray<AFileTargetBox*>& Tiles);

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug", Config)
    bool bPerformanceMeasurement;

  UPROPERTY(EditAnywhere, Category = "Debug", AdvancedDisplay, meta = (ClampMin = 2), Config)
    int iHowMany = 50;

  UPROPERTY(EditAnywhere, Category = "Debug", AdvancedDisplay, Config)
  int CurrentAmount = 0;
  bool bCsvActive = false;

  UPROPERTY(EditAnywhere, Category = "Debug", AdvancedDisplay, Config)
    float fWaitBetween = 5.f;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

  void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

  float DefaultIsoSurface = 2000.f;
  float IsoSurfaceParam = 2000.f;
  float IsoMin = 0.f;
  float IsoMax = 4000.f;

private:
  TArray<AFileTargetBox*> BoxSet;
  TArray<FString> FileList;
  unsigned int ScrollPos;
  int ActiveFile{-1};
  AFileTargetBox* CurrentlyHighlighted = nullptr;
  int bInit = 0;
  bool bHasFileLists = false;

  // SCALING INTERACTION TIMER
  FTimerDelegate RootRandomDelegate;
  FTimerHandle TimerHandle;
public:
  void BeginDestroy() override;

};
