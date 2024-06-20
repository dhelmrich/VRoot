// Fill out your copyright notice in the Description page of Project Settings.


#include "SliderWidget.h"

void ASliderWidget::OnGrabbed_Implementation()
{
  OriginPosition = GetActorTransform();
}

void ASliderWidget::OnReleased_Implementation()
{
  CallbackRef(OriginPosition);
}

void ASliderWidget::SetReleaseCallback(TFunction<void(const FTransform&)> InCallbackRef)
{
  CallbackRef=InCallbackRef;
}

void ASliderWidget::BeginPlay()
{
  BeginPlayLocation = GetActorLocation();
}
