// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RootSegment.h"
#include <Containers/Map.h>

#include "Root.generated.h"

UCLASS()
class ROOTMRICNR_API URoot : public UObject
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
// 	URoot();
	~URoot()
	{
		LCJointPositions.Empty();
		Diameters.Empty();
	}

protected:

public:	

	TArray<FVector> LCJointPositions;
	int RootNumber;
	int Predecessor;
	int StartJoint;
	TArray<float> Diameters;
	bool DiameterOnJoint = true;
		
};
