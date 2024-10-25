// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RootMRICNR/Clickable.h"
#include "RootDataSwitcher.generated.h"

UCLASS()
class ROOTMRICNR_API ARootDataSwitcher : public AActor, public IClickable
{
  GENERATED_BODY()

public:
  // Sets default values for this actor's properties
  ARootDataSwitcher();

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RootDataSwitcher")
  class UStaticMeshComponent* Button;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RootDataSwitcher")
  class ADrawSpace* DrawSpace;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RootDataSwitcher")
  class UStaticMeshComponent* RootMesh;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RootDataSwitcher")
  class UStaticMesh* RootMeshAsset;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RootDataSwitcher")
  FVector CustomScale {1.0f, 1.0f, 1.0f};

protected:
  // Called when the game starts or when spawned
  virtual void BeginPlay() override;

public:
  // Called every frame
  virtual void Tick(float DeltaTime) override;
  virtual void OnClickSignal_Implementation(FVector WorldPositionOfClick) override;
};
