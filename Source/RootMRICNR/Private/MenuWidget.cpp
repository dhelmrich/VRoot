// Fill out your copyright notice in the Description page of Project Settings.


#include "MenuWidget.h"

void AMenuWidget::OnGrabbed_Implementation()
{
  OriginPosition = GetActorTransform();
}

void AMenuWidget::OnReleased_Implementation()
{
  SetActorTransform(OriginPosition);
}
