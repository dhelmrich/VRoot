// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "RootMRICNRGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class ROOTMRICNR_API ARootMRICNRGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
		FString FilterString(FString Input);
};
