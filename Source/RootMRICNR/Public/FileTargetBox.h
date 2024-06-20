// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include <Materials/MaterialInstanceDynamic.h>
#include "Engine/FontFace.h"
#include "Engine/Font.h"
#include "../Clickable.h"
#include "FileTargetBox.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUnrealFileRelease,int,FileID);

/**
 *
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ROOTMRICNR_API AFileTargetBox : public AStaticMeshActor, public IClickable
{
  GENERATED_BODY()

public:
  //AFileTargetBox();

  AFileTargetBox(const FObjectInitializer& ObjectInitializer);
  virtual void OnConstruction(const FTransform& Transform) override;
  
  UFUNCTION(BlueprintCallable , Category = "Actor")
    void RegisterRuntime(FString filename, int fileid, int lFontSize = 300);

  UFUNCTION(BlueprintCallable, Category = "Actor")
    void Write(FString filename,int fileid, int lFontSize = 300);

    void UpdateColor(FVector color);

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Actor")
    UMaterialInstanceDynamic* DynMatInst;

  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Representation")
    int FileID = 0;

  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Representation")
    FString TextOnMesh;

  UPROPERTY(EditAnywhere)
    UMaterial* TextMaterialParent;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category ="Representation")
  bool bAutoResetColor = true;

  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Representation")
    class UCanvasRenderTarget2D* RenderTarget = nullptr;

  UPROPERTY(EditAnywhere, BlueprintAssignable, Category = "Representation")
    FUnrealFileRelease InformBlueprint;

  UFont* Font;
  FString FileName;
  float HighlightTime;

  void OnClicked_Implementation(FVector WorldPositionOfClick) override;

  void SetClickCallback(TFunction<void(void)> InCallbackRef);
protected:
  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

  TFunction<void(void)> CallbackRef;

  void BeginPlay() override;
  class UTexture* CleanBoardTexture;
  FVector OldColor{FVector(0.f,0.f,0.f)};

public:
  // Called every frame
  virtual void Tick(float DeltaTime) override;

};
