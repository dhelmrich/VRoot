// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OverlapTransform.generated.h"

UCLASS()
class ROOTMRICNR_API AOverlapTransform : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AOverlapTransform();

	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* Representation;

	UPROPERTY(VisibleAnywhere)
	class UBoxComponent* Collider;

	UPROPERTY(VisibleAnywhere)
    int SegmentNum;

	UPROPERTY()
		int PlaceSegment;

  UPROPERTY(VisibleAnywhere)
    int RootNum;

	UPROPERTY(VisibleAnywhere)
		int Predecessor;

  UPROPERTY(EditAnywhere)
	  FVector Default{ 0.181f,0.462f,0.445f };
	UPROPERTY(EditAnywhere)
	  FVector Active{0,0,1};
	UPROPERTY(EditAnywhere)
	  FVector MightSelect{0,1,0};
	UPROPERTY(EditAnywhere)
	  FVector MightUnselect{1,0,0};

	bool bSelected;
	bool bHighlighted;
	UFUNCTION()
	  void Selected(bool SelectedValue);
	UFUNCTION()
	  void HighLight(int HighlightValue);
	UFUNCTION()
		void SetTemporary(bool NewTemporarilyVisible);

	UPROPERTY(VisibleAnywhere)
		bool bChangePending = false;

	UPROPERTY(VisibleAnywhere)
	 bool bEndCap = false;

	UPROPERTY()
		bool bTemporarilyVisible = false;

	UPROPERTY()
		FVector OriginPosition;

	UPROPERTY()
		FVector OriginScale;

	UPROPERTY()
		FQuat OriginOrientation;

	UPROPERTY()
		TArray<AOverlapTransform*> SideNode;

	UFUNCTION()
		void SaveState();

	UFUNCTION()
		void RetreiveState();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
