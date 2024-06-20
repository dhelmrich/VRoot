// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "ProceduralMeshComponent.h"
#include "RootCone.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ROOTMRICNR_API URootCone : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	URootCone();

	UFUNCTION()
	void Build();

	UFUNCTION()
	void RegisterAtRuntime();

	//UPROPERTY()
	UProceduralMeshComponent* ConeMesh;

	UPROPERTY()
	class UMaterialInstanceDynamic* MatInstance;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Geometry")
	int resolution = 20;

	/**
	 * StartPoints: Tarray that contains points for each side
   * We use splitting normals here
	 */
	UPROPERTY()
  TArray<FVector> StartPoints;

  UPROPERTY()
  TArray<FVector> EndPoints;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Geometry")
  float StartRadius = 0.5f;
	UFUNCTION()
	void SetStartRadius(float start_radius);

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Geometry")
    float EndRadius = 0.7f;
  UFUNCTION()
    void SetEndRadius(float end_radius);

	UFUNCTION()
    void RenderCaps();

  UFUNCTION()
    void UpdateCap(bool start);

	UPROPERTY()
	  bool RenderFlats = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geometry")
	  bool NormalSplitting = true;

	int counter = 10;
	bool countingup = true;

	TArray<FVector> Normals;
	TArray<FVector> MovingNormal;
	TArray<FLinearColor> VertexColors;
	TArray<FVector2D> UVCoords;
	TArray<int32> Triangles;
	TArray<FProcMeshTangent> Tangents;

	TArray<FVector> StartCapPoints;
	TArray<FVector> EndCapPoints;
	TArray<FVector> StartCapNormals;
	TArray<FVector> EndCapNormals;
	TArray<FLinearColor> CapColor;
	TArray<FVector2D> CapUV;
	TArray<int32> StartCapTris;
	TArray<int32> EndCapTris;
	TArray<FProcMeshTangent> CapTangents;


private:
	void BuildNoSplit();
	void UpdateNoSplitStart(float start_radius);
	void UpdateNoSplitEnd(float end_radius);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
