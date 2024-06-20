// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UIExtender.generated.h"

class AFileTargetBox;


UCLASS()
class ROOTMRICNR_API AUIExtender : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AUIExtender();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Interaction")
    class UStaticMeshComponent* Showcase;
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Interaction")
    AFileTargetBox* RadiusLocking;
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Interaction")
    AFileTargetBox* Undo;
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Interaction")
    AFileTargetBox* Reset;
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Interaction")
    AFileTargetBox* Delete;

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	  void RegisterRuntime();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	//virtual void OnConstruction(const FTransform& Transform) override;
};
