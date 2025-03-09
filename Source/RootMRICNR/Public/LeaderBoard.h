// Fill out your copyright notice in the Description page of Project Settings.

#if 0
#pragma once

#include "CoreMinimal.h"
#include "FileTargetBox.h"
#include "Templates/Tuple.h"
#include "LeaderBoard.generated.h"

/**
 * 
 */
UCLASS()
class ROOTMRICNR_API ALeaderBoard : public AFileTargetBox
{
  GENERATED_BODY()
public:
  virtual void OnConstruction(const FTransform& Transform) override;
  
  UFUNCTION(BlueprintCallable)
  void AddToLeaderBoard(FString name);
  UFUNCTION(BlueprintCallable)
  TArray<FString> GetLeaderboardInOrder();

  UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
    float LastTime = 0.f;
  UFUNCTION(BlueprintCallable)
    void StartTimer();
  UFUNCTION(BlueprintCallable)
    void StopTimer();

  UFUNCTION(BlueprintCallable)
    void PrintLeaderboard();

  UFUNCTION(BlueprintCallable)
    void DumpLeaderbord();

  UPROPERTY(EditAnywhere)
    bool bFallbackMode = false;

  virtual void Tick(float DeltaTime) override;

  
  TArray<TTuple<FString,float>> Leaderboard;

protected:
  float TimeAtStart = 0.f;
  // Called when the game starts or when spawned
  virtual void BeginPlay() override;
  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
#endif
