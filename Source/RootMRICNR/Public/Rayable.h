// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UObject/Interface.h"
#include "Rayable.generated.h"

// This class does not need to be modified.
UINTERFACE(BlueprintType)
class ROOTMRICNR_API URayable : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

/**
 * 
 */
class IRayable
{
	GENERATED_IINTERFACE_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Gameplay)
    void OnRay(FVector WorldPositionOfHit);
};
