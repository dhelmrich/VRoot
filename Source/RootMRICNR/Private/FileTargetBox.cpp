// Fill out your copyright notice in the Description page of Project Settings.


#include "FileTargetBox.h"
#include "Engine/CanvasRenderTarget2D.h"
#include <UObject/ConstructorHelpers.h>
#include <Materials/MaterialInstanceDynamic.h>
#include "Kismet/KismetRenderingLibrary.h"
#include "Engine/CanvasRenderTarget2D.h"
#include "Engine/Canvas.h"
#include "Fonts/CompositeFont.h"
#include "UObject/UObjectGlobals.h"
#include "ImageUtils.h"

AFileTargetBox::AFileTargetBox(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer), CallbackRef{ []() {} }
{
  PrimaryActorTick.bCanEverTick = true;
  PrimaryActorTick.bStartWithTickEnabled = false;
  PrimaryActorTick.bAllowTickOnDedicatedServer = true;
  
  static ConstructorHelpers::FObjectFinder<UStaticMesh> BoxAsset(TEXT("StaticMesh'/Game/SpawnfriendlyBoxUVW'"));
  static ConstructorHelpers::FObjectFinder<UMaterial> MatAsset(TEXT("Material'/Game/3DUI/mat_file_text'"));
  if (BoxAsset.Succeeded() && MatAsset.Succeeded())
  {
    this->GetStaticMeshComponent()->SetStaticMesh(BoxAsset.Object);
    TextMaterialParent = MatAsset.Object;
  }
  // ARLRDBD_Font
  ConstructorHelpers::FObjectFinder<UFont> FontObject(TEXT("Font'/Game/UI/ARLRDBD_Font'"));
  if(FontObject.Succeeded())
    Font = FontObject.Object;
  SetActorScale3D(FVector(100,100,2));
  SetActorRelativeRotation(FRotator(0, 0, 90));

  static ConstructorHelpers::FObjectFinder<UTexture2D> TexAsset(TEXT("UTexture2D'/game/cleantexture'"));
  if (TexAsset.Succeeded())
    CleanBoardTexture = TexAsset.Object;
}

void AFileTargetBox::OnConstruction(const FTransform& Transform)
{
  Super::OnConstruction(Transform);
}

void AFileTargetBox::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  Super::EndPlay(EndPlayReason);
}

void AFileTargetBox::BeginPlay()
{
  Super::BeginPlay();

//   TArray<FColor> colordat;
//   colordat.Add(FColor());
//   EObjectFlags f;
//   FCreateTexture2DParameters p;
//   CleanBoardTexture = FImageUtils::CreateTexture2D(1, 1, colordat, this, MakeUniqueObjectName(this, UTexture2D::StaticClass()).ToString(), f, p);



}

void AFileTargetBox::RegisterRuntime(FString filename, int fileid, int lFontSize)
{
  DynMatInst = UMaterialInstanceDynamic::Create(TextMaterialParent, GetStaticMeshComponent());
  this->GetStaticMeshComponent()->SetMaterial(0, DynMatInst);
  RenderTarget = UCanvasRenderTarget2D::CreateCanvasRenderTarget2D(GEngine->GetWorld(), UCanvasRenderTarget2D::StaticClass(), 4096, 4096);
  DynMatInst->SetTextureParameterValue("TextTexture",RenderTarget);
  
  FileName = filename;
  FileID = fileid;
  UCanvas* Canvas;
  FVector2D CanvasSize;
  FDrawToRenderTargetContext context;
  Font->LegacyFontSize = lFontSize;

  UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(this,RenderTarget,Canvas,CanvasSize,context);
  
  Canvas->DrawText(Font,FText::FromString(FileName),0.f,0.f,1.f,1.f);

  UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(this,context);
}

void AFileTargetBox::Write(FString filename, int fileid, int lFontSize)
{
  if(!IsValid(this))
    return;
  FileName = filename;
  FileID = fileid;
  UCanvas* Canvas;
  FVector2D CanvasSize;
  FDrawToRenderTargetContext context;
  Font->LegacyFontSize = lFontSize;


//   TArray<FColor> colordat;
//   colordat.Add(FColor());
//   if(CleanBoardTexture == nullptr)
//   {
//     EObjectFlags f;
//     FCreateTexture2DParameters p;
//     CleanBoardTexture = FImageUtils::CreateTexture2D(1, 1, colordat, this, MakeUniqueObjectName(this, UTexture2D::StaticClass()).ToString(), f, p);
//   }


  UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(this, RenderTarget, Canvas, CanvasSize, context);

  Canvas->DrawTile(CleanBoardTexture,Canvas->OrgX,Canvas->OrgY,Canvas->SizeX,Canvas->SizeY,0.f,0.f,1.f,1.f);
  Canvas->DrawText(Font, FText::FromString(FileName), 0.f, 0.f, 1.f, 1.f);

  UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(this, context);
}

void AFileTargetBox::UpdateColor(FVector color)
{
  this->GetStaticMeshComponent()->SetVectorParameterValueOnMaterials(TEXT("Background"), color);
  OldColor = color;
}

void AFileTargetBox::SetClickCallback(TFunction<void(void)> InCallbackRef)
{
  CallbackRef = InCallbackRef;
}

void AFileTargetBox::OnClickSignal_Implementation(FVector WorldPositionOfClick)
{
  InformBlueprint.Broadcast(this->FileID);
  HighlightTime = 2.f;
  this->SetActorTickEnabled(bAutoResetColor);
  this->GetStaticMeshComponent()->SetVectorParameterValueOnMaterials(TEXT("Background"),FVector(1.f,0.f,0.f));
  CallbackRef();
}

void AFileTargetBox::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);
  HighlightTime -= DeltaTime;
  if (HighlightTime <= 0.f)
  {
    this->GetStaticMeshComponent()->SetVectorParameterValueOnMaterials(TEXT("Background"), OldColor);
    this->SetActorTickEnabled(false);
  }

}

