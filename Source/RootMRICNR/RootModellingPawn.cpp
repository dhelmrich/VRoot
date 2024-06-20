#include "RootModellingPawn.h"

#include "Camera/CameraComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/InputSettings.h"
#include "GameFramework/WorldSettings.h"
#include "UObject/UObjectGlobals.h"
#include "Kismet/GameplayStatics.h"


#include "HeadMountedDisplayFunctionLibrary.h"
#include "GrabbingBehaviorComponent.h"
#include "Grabable.h"
#include "Clickable.h"
#include "Rayable.h"
#include "Public/RootSegment.h"
#include <IXRTrackingSystem.h>
#include <IHeadMountedDisplay.h>
#include <StereoRendering.h>
#include <Templates/SharedPointerInternals.h>
#include <Templates/SharedPointer.h>
#include <Components/PrimitiveComponent.h>
//#include "Components/WidgetInteractionComponent.h"
#include <Math/Rotator.h>
#include "OverlapTransform.h"
#include "MenuWidget.h"
#include "Camera/CameraActor.h"
#include <Kismet/KismetMathLibrary.h>
#include "Engine/StaticMeshActor.h"

#include "HighResScreenshot.h"
#include "Engine/BlockingVolume.h"

#include "UnrealClient.h"
#include "GameFramework/PlayerStart.h"

// ARootModellingPawn

ARootModellingPawn::ARootModellingPawn(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
  // Collision component
  CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent0"));
  CollisionComponent->InitSphereRadius(35.0f);
  CollisionComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
  CollisionComponent->CanCharacterStepUpOn = ECB_No;
  CollisionComponent->SetCanEverAffectNavigation(true);
  CollisionComponent->bDynamicObstacle = true;
  CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

  // Collision component must always be a root
  RootComponent = CollisionComponent;
  // Camera
  CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("VRCamera"));
  CameraComponent->AttachToComponent(CollisionComponent, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));
  CameraComponent->bUsePawnControlRotation = false;
  //CameraComponent->SetUsingAbsoluteLocation(false);
  //CameraComponent->SetUsingAbsoluteRotation(false);

  bUseControllerRotationYaw = true;
  bUseControllerRotationPitch = true;
  bUseControllerRotationRoll = true;

  PrimaryActorTick.bCanEverTick = true;
  bFindCameraComponentWhenViewTarget = true;
  SetCanBeDamaged(false);
  bReplicates = false;
  SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

  AutoPossessPlayer = EAutoReceiveInput::Player0; // Necessary for receiving motion controller events.

  Movement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("Movement"));
  Movement->UpdatedComponent = RootComponent;

  RotatingMovement = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotatingMovement"));
  RotatingMovement->UpdatedComponent = RootComponent;
  RotatingMovement->bRotationInLocalSpace = false;
  RotatingMovement->PivotTranslation = FVector::ZeroVector;
  RotatingMovement->RotationRate = FRotator::ZeroRotator;

  Head = CreateDefaultSubobject<USceneComponent>(TEXT("Head"));
  Head->SetupAttachment(RootComponent);
  RightHand = CreateDefaultSubobject<USceneComponent>(TEXT("RightHand"));
  RightHand->SetupAttachment(RootComponent);
  LeftHand = CreateDefaultSubobject<USceneComponent>(TEXT("LeftHand"));
  LeftHand->SetupAttachment(RootComponent);



  HmdLeftMotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("HmdLeftMotionController"));
  HmdLeftMotionController->SetupAttachment(RootComponent);
  HmdLeftMotionController->SetTrackingSource(EControllerHand::Left);
  HmdLeftMotionController->SetShowDeviceModel(true);
  HmdLeftMotionController->SetVisibility(false);

  HmdRightMotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("HmdRightMotionController"));
  HmdRightMotionController->SetupAttachment(RootComponent);
  HmdRightMotionController->SetTrackingSource(EControllerHand::Right);
  HmdRightMotionController->SetShowDeviceModel(true);
  HmdRightMotionController->SetVisibility(false);



  // INITIALIZE MODES
  static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereAsset((TEXT("StaticMesh'/Game/SpawnfriendlySphere'")));
  static ConstructorHelpers::FObjectFinder<UStaticMesh> DSphereAsset((TEXT("StaticMesh'/Game/DrawIndicatorSphere'")));
  static ConstructorHelpers::FObjectFinder<UStaticMesh> PlateAsset(TEXT("StaticMesh'/Game/3DUI/highlight_flat'"));
  static ConstructorHelpers::FObjectFinder<UStaticMesh> CylAsset(TEXT("StaticMesh'/Game/SpawnfriendlyCylinder'"));
  static ConstructorHelpers::FObjectFinder<UMaterial> CylAssetMat(TEXT("Material'/Game/mat_glow'"));
  static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("StaticMesh'/Engine/BasicShapes/Plane'"));
  static ConstructorHelpers::FObjectFinder<UMaterial> HelperMaterial(TEXT("Material'/Game/mat_helper'"));

  // INITIALIZE DRAW MODE
  DrawIndicator = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Draw Position Indicator"));
  if (DSphereAsset.Succeeded())
  {
    DrawIndicator->SetStaticMesh(DSphereAsset.Object);
    DrawIndicator->SetupAttachment(RightHand);
    DrawIndicator->SetRelativeLocation(DefaultDrawIndicatorPosition);
    DrawIndicator->SetWorldScale3D(DefaultDrawIndicatorScale);
    DrawIndicator->SetCastShadow(false);
  }
  else
  {
    throw "Sphere not loaded";
  }
  DrawIndicator->SetVisibility(true); // for thesis picture
  this->DrawIndicator->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
  DrawIndicator->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
  DrawIndicator->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
  DrawIndicator->OnComponentBeginOverlap.AddDynamic(this, &ARootModellingPawn::HighlightEndingDraw);
  DrawIndicator->OnComponentEndOverlap.AddDynamic(this, &ARootModellingPawn::StopHighlightEndingDraw);


  NewNodeIndicator = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Glowy Thing"));
  NewNodeIndicator->SetupAttachment(RightHand);
  NewNodeIndicator->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  if (CylAsset.Succeeded())
  {
    NewNodeIndicator->SetStaticMesh(CylAsset.Object);
    if (CylAssetMat.Succeeded())
    {
      NewNodeIndicator->SetMaterial(0,CylAssetMat.Object);
    }
  }

  TransformPlane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Helper Board"));
  TransformPlane->SetupAttachment(RightHand);
  TransformPlane->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  TransformPlane->SetVisibility(false);
  if (PlaneMesh.Succeeded())
  {
    TransformPlane->SetStaticMesh(PlaneMesh.Object);
    if (HelperMaterial.Succeeded())
    {
      TransformPlane->SetMaterial(0, HelperMaterial.Object);
    }
  }
  else
  {
    UE_LOG(LogTemp, Error, TEXT("Could not find the plane asset"));
  }

  this->InteractionMode = EVRInteractionModes::int_mode_menu;
  NewParams.AddIgnoredActor(RegisteredDrawSpace);
  //INITALIZE MENU MODE WITH STATIC MESHES
//  LiteralLeftHand = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Left Hand Model"));
// 	LiteralLeftHand->RegisterComponent();
// 	LiteralLeftHand->SetStaticMesh(HandOpen);
// 	LiteralLeftHand->SetupAttachment(LeftHand);
// 	LiteralLeftHand->SetRelativeLocation(FVector(50, 0, 0));
// 	LiteralLeftHand->SetWorldScale3D(FVector(1, -1, 1));
// 	LiteralLeftHand->SetVisibility(true);
// 
//   LiteralRightHand = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Right Hand Model"));
// 	LiteralRightHand->RegisterComponent();
// 	LiteralRightHand->SetStaticMesh(HandOpen);
// 	LiteralRightHand->SetupAttachment(RightHand);
//  LiteralRightHand->SetRelativeLocation(FVector(50, 0, 0));
// 	LiteralRightHand->SetVisibility(true);

}

void ARootModellingPawn::OnForward_Implementation(float Value)
{
  if (Value != 0.f && Head && (NavigationMode == EVRNavigationModes::nav_mode_fly))
  {
    AddMovementInput(Head->GetForwardVector(), Value);
  }
}

void ARootModellingPawn::OnRight_Implementation(float Value)
{
  if (Head && (NavigationMode == EVRNavigationModes::nav_mode_fly))
  {
    AddMovementInput(Head->GetRightVector(), Value);
  }
}

void ARootModellingPawn::OnTurnRate_Implementation(float Rate)
{
  AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds() * CustomTimeDilation);
}

void ARootModellingPawn::OnLookUpRate_Implementation(float Rate)
{
  AddControllerPitchInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds() * CustomTimeDilation);
}

float ARootModellingPawn::GetBaseTurnRate() const
{
  return BaseTurnRate;
}

void ARootModellingPawn::SetBaseTurnRate(float Value)
{
  BaseTurnRate = Value;
}

UFloatingPawnMovement* ARootModellingPawn::GetFloatingPawnMovement()
{
  return Movement;
}

URotatingMovementComponent* ARootModellingPawn::GetRotatingMovementComponent()
{
  return RotatingMovement;
}

UMotionControllerComponent* ARootModellingPawn::GetHmdLeftMotionControllerComponent()
{
  return HmdLeftMotionController;
}

UMotionControllerComponent* ARootModellingPawn::GetHmdRightMotionControllerComponent()
{
  return HmdRightMotionController;
}

USceneComponent* ARootModellingPawn::GetHeadComponent()
{
  return Head;
}

USceneComponent* ARootModellingPawn::GetLeftHandComponent()
{
  return LeftHand;
}

USceneComponent* ARootModellingPawn::GetRightHandComponent()
{
  return RightHand;
}

void ARootModellingPawn::AfterDeleteResetInteraction()
{
  Selection.Empty();
  MoveMeta->SetActorHiddenInGame(true);
  MoveMeta->SetActorEnableCollision(false);
  if (!bDemoVersion)
  {
    DiaMeta->SetActorHiddenInGame(true);
    DiaMeta->SetActorEnableCollision(false);
  }
  bWidgetsVisible = false;
}

// USceneComponent* ARootModellingPawn::GetTrackingOriginComponent()
// {
// 	return TrackingOrigin;
// }

bool ARootModellingPawn::CheckIsHeadMountedModeActive()
{
  // Aachen version of detecting HMD presence
 //return UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayConnected();
//  return UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled();

//   IHeadMountedDisplay* hmd = nullptr;
//   TSharedPtr<IStereoRendering, ESPMode::ThreadSafe> stereo = nullptr;
//   if (GEngine && GEngine->XRSystem)
//   {
//     hmd = GEngine->XRSystem->GetHMDDevice();
//     stereo = GEngine->XRSystem->GetStereoRenderingDevice();
//     return hmd->IsHMDEnabled() && hmd->IsHMDConnected() && stereo->IsStereoEnabled();
//   }
//   return false;
  return bUseHMD;
}

void ARootModellingPawn::BeginPlay()
{
  Super::BeginPlay();

  //APlayerController* PController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
  //if (PController)
  //{
  //  PController->ConsoleCommand(TEXT("r.ViewDistanceScale 0"), true);
  //  PController->ConsoleCommand(TEXT("r.PostProcessAAQuality 1"), true);
  //  PController->ConsoleCommand(TEXT("sg.ShadowQuality 0"), true);
  //  PController->ConsoleCommand(TEXT("sg.TextureQuality 0"), true);
  //  PController->ConsoleCommand(TEXT("vr.bEnableHMD 1"), true);
  //  PController->ConsoleCommand(TEXT("vr.bEnableStereo"), true);
  //  PController->ConsoleCommand(TEXT("t.maxFPS 90"), true);
  //  PController->ConsoleCommand(TEXT("r.ScreenPercentage 200"), true);
  //  PController->ConsoleCommand(TEXT("r.PixelDensity 2"), true);
  //  PController->ConsoleCommand(TEXT("r.SceneRenderTargetResizeMethod 2"), true);
  //}

  ScaleTimerDel.BindLambda([this]()
  {
    RegisteredDrawSpace->UpdateSections(Selection, bFixSegmentSize);
  });

  if (CheckIsHeadMountedModeActive())
  {
    bHadToRemoveMouseSettings = true;
    UInputSettings::GetInputSettings()->RemoveAxisMapping(FInputAxisKeyMapping("TurnRate", EKeys::MouseX));
    UInputSettings::GetInputSettings()->RemoveAxisMapping(FInputAxisKeyMapping("LookUpRate", EKeys::MouseY));
    UInputSettings::GetInputSettings()->RemoveAxisMapping(FInputAxisKeyMapping("MoveForward", EKeys::W));
    UInputSettings::GetInputSettings()->RemoveAxisMapping(FInputAxisKeyMapping("MoveForward", EKeys::S));
    UInputSettings::GetInputSettings()->RemoveAxisMapping(FInputAxisKeyMapping("MoveRight", EKeys::A));
    UInputSettings::GetInputSettings()->RemoveAxisMapping(FInputAxisKeyMapping("MoveRight", EKeys::D));
    

    HmdLeftMotionController->SetVisibility(ShowHMDControllers);
    HmdRightMotionController->SetVisibility(ShowHMDControllers);

    LeftHand->AttachToComponent(HmdLeftMotionController, FAttachmentTransformRules::SnapToTargetIncludingScale);
    RightHand->AttachToComponent(HmdRightMotionController, FAttachmentTransformRules::SnapToTargetIncludingScale);
    Head->AttachToComponent(GetCameraComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
    CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    auto PlayerStart = UGameplayStatics::GetActorOfClass(GetWorld(), APlayerStart::StaticClass());
    

    //this->SetActorLocation({ 0,0,0 });
    //CameraComponent->SetWorldLocation({ 0,0,150.f });
    //Head->SetWorldLocation({ 0,0,150.f });
  }
  else //Desktop
  {
    Head->AttachToComponent(GetCameraComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);

    //also attach the hands to the camera component so we can use them for interaction
    LeftHand->AttachToComponent(GetCameraComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
    RightHand->AttachToComponent(GetCameraComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
    LeftHand->SetRelativeLocation({ 10,-20,0 });
    RightHand->SetRelativeLocation({ 10,20,0 });
    CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    CameraComponent->SetRelativeLocation({ 0,0,0 });
    Head->SetRelativeLocation({ 0,0,0 });
    //move to eyelevel
    //GetCameraComponent()->SetRelativeLocation(FVector(0, 0, 160));
  }


//   UInputSettings::GetInputSettings()->RemoveAxisMapping(FInputAxisKeyMapping("TurnRate", EKeys::MouseX));
//   UInputSettings::GetInputSettings()->RemoveAxisMapping(FInputAxisKeyMapping("LookUpRate", EKeys::MouseY));
// 
//   HmdLeftMotionController->SetVisibility(ShowHMDControllers);
//   HmdRightMotionController->SetVisibility(ShowHMDControllers);
//   if (HmdTracker1->IsActive()) {
//     HmdTracker1->SetVisibility(ShowHMDControllers);
//   }
//   if (HmdTracker2->IsActive()) {
//     HmdTracker2->SetVisibility(ShowHMDControllers);
//   }
// 
//   LeftHand->AttachToComponent(HmdLeftMotionController, FAttachmentTransformRules::SnapToTargetIncludingScale);
//   RightHand->AttachToComponent(HmdRightMotionController, FAttachmentTransformRules::SnapToTargetIncludingScale);
//   Head->AttachToComponent(GetCameraComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);

  //In ADisplayClusterPawn::BeginPlay() input is disabled on all slaves, so we cannot react to button presses, e.g. on the flystick correctly.
  //Therefore, we activate it again:
  UWorld* World = GetWorld();
  if (World)
  {
    APlayerController* PlayerController = World->GetFirstPlayerController();
    if (PlayerController)
    {
      this->EnableInput(PlayerController);
    }
  }

  // INITALIZE MENU MODE HERE BECAUSE OF DEFERRED LOADING IN BLUEPRINTS

  //PointIndicator = FindComponentByClass<UParticleSystemComponent>();
  DrawConnection = Cast<UParticleSystemComponent>(GetDefaultSubobjectByName(TEXT("DrawHelpLaser")));
  PointIndicator = Cast<UParticleSystemComponent>(GetDefaultSubobjectByName(TEXT("PointyLaser")));
  PointIndicator->SetActive(true, true);
  DrawConnection->SetActive(false, true);
  DrawIndicator->SetRelativeLocation(DefaultDrawIndicatorPosition);
  DrawIndicator->SetWorldScale3D(DefaultDrawIndicatorScale);

  TArray<AActor*> Found;
  UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABlockingVolume::StaticClass(),Found);
  NewParams.AddIgnoredActors(Found);

  NewParams.bTraceComplex = true;
  NewParams.AddIgnoredActor(this);

  

//   DrawIndicator->SetVisibility(false,false);
//   PointIndicator->SetActive(false,true);

}

void ARootModellingPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  Super::EndPlay(EndPlayReason);
  if (bHadToRemoveMouseSettings)
  {
    UInputSettings::GetInputSettings()->AddAxisMapping(FInputAxisKeyMapping("TurnRate", EKeys::MouseX,1.f));
    UInputSettings::GetInputSettings()->AddAxisMapping(FInputAxisKeyMapping("LookUpRate", EKeys::MouseY,-1.f));
    UInputSettings::GetInputSettings()->AddAxisMapping(FInputAxisKeyMapping("MoveForward", EKeys::W, 1.f));
    UInputSettings::GetInputSettings()->AddAxisMapping(FInputAxisKeyMapping("MoveForward", EKeys::S, -1.f));
    UInputSettings::GetInputSettings()->AddAxisMapping(FInputAxisKeyMapping("MoveRight", EKeys::D, 1.f));
    UInputSettings::GetInputSettings()->AddAxisMapping(FInputAxisKeyMapping("MoveRight", EKeys::A, -1.f));
  }
}

void ARootModellingPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
  Super::SetupPlayerInputComponent(PlayerInputComponent);
  if (PlayerInputComponent)
  {
    PlayerInputComponent->BindAxis("MoveForward", this, &ARootModellingPawn::OnForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &ARootModellingPawn::OnRight);
    PlayerInputComponent->BindAxis("TurnRate", this, &ARootModellingPawn::OnTurnRate);
    PlayerInputComponent->BindAxis("LookUpRate", this, &ARootModellingPawn::OnLookUpRate);
    PlayerInputComponent->BindAxis("ScaleWidth", this, &ARootModellingPawn::OnScale);
    PlayerInputComponent->BindAxis("ScaleIndicator", this, &ARootModellingPawn::OnIndiScale);
    PlayerInputComponent->BindAxis("MoveUp", this, &ARootModellingPawn::OnRise);
    PlayerInputComponent->BindAxis("Down", this, &ARootModellingPawn::OnSink);

    PlayerInputComponent->BindAxis("RotateTouchX", this, &ARootModellingPawn::OnRotationTouchX);
    PlayerInputComponent->BindAxis("RotateTouchY", this, &ARootModellingPawn::OnRotationTouchY);
    // function bindings for grabbing and releasing
    PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ARootModellingPawn::OnBeginFire);
    PlayerInputComponent->BindAction("Fire", IE_Released, this, &ARootModellingPawn::OnEndFire);

    PlayerInputComponent->BindAction("FireLeft", IE_Pressed, this, &ARootModellingPawn::OnBeginLeftFire);
    PlayerInputComponent->BindAction("FireLeft", IE_Released, this, &ARootModellingPawn::OnEndLeftFire);
    PlayerInputComponent->BindAction("ForwardForce", IE_Released, this, &ARootModellingPawn::ForwardForce);
    PlayerInputComponent->BindAction("BackwardForce", IE_Released, this, &ARootModellingPawn::BackwardForce);
    //PlayerInputComponent->BindAction("ToggleDrawingAllowed", IE_Pressed, this, &ARootModellingPawn::SelectRoot);
    PlayerInputComponent->BindAction("TogglePerfMeasurement",IE_Pressed, this, &ARootModellingPawn::TogglePerfMeasure);
    PlayerInputComponent->BindAction("Action5", IE_Pressed, this, &ARootModellingPawn::SelectEverything);
    // ToggleHandVis disabled in editor
    // TODO REMOVE THE MESSINESS OF THIS BY REMOVING OLD INTERACTION METHODS/SIGNALS
    PlayerInputComponent->BindAction("ResetInteraction", IE_Pressed, this, &ARootModellingPawn::ResetInteraction);
    PlayerInputComponent->BindAction("SplitRoot", IE_Pressed, this, &ARootModellingPawn::ToggleAutoAppend);
    PlayerInputComponent->BindAction("DeleteAction", IE_Pressed, this, &ARootModellingPawn::ToggleQuickMode);
    PlayerInputComponent->BindAction("ToggleHandVis", IE_Pressed, this, &ARootModellingPawn::ToggleHandVis);
    PlayerInputComponent->BindAction("SwitchSelectionMode",IE_Pressed,this,&ARootModellingPawn::ToggleModeLeft);
    PlayerInputComponent->BindAction("RootSystemNavUp",IE_Pressed,this,&ARootModellingPawn::MoveSelectionUp);
    PlayerInputComponent->BindAction("RootSystemNavDown",IE_Pressed,this,&ARootModellingPawn::MoveSelectionDown);
  }
}

void ARootModellingPawn::OnScale_Implementation(float Rate)
{
  if(Rate > 0.0001f)
  {
    float chg = 1.f + Rate*0.5f;
    for (AOverlapTransform* ovlp : Selection)
    {
      ovlp->SetActorRelativeScale3D(ovlp->GetActorRelativeScale3D() * chg);
    }
    GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, ScaleTimerDel,1,false, 5.f);
  }
}

void ARootModellingPawn::OnIndiScale_Implementation(float Rate)
{
}

void ARootModellingPawn::ForwardForce()
{

  if (bRightFire == true || DrawMode != EVRDrawModes::draw_mode_free || bUseHMD)
    return;
  AddMovementInput(LeftHand->GetForwardVector(), 1);
}

void ARootModellingPawn::BackwardForce()
{

  if (bRightFire == true || DrawMode != EVRDrawModes::draw_mode_free || bUseHMD)
    return;
  AddMovementInput(RightHand->GetForwardVector(), -1);
}

void ARootModellingPawn::HandlePhysicsAndAttachActor(AActor* HitActor)
{
  UPrimitiveComponent* PhysicsComp = HitActor->FindComponentByClass<UPrimitiveComponent>();

  bDidSimulatePhysics = PhysicsComp->IsSimulatingPhysics(); // remember if we need to tun physics back on or not	
  PhysicsComp->SetSimulatePhysics(false);
  FAttachmentTransformRules Rules = FAttachmentTransformRules::KeepWorldTransform;
  Rules.bWeldSimulatedBodies = true;
  HitActor->AttachToComponent(this->LeftHand, Rules);
}

void ARootModellingPawn::NotifyActorsDependingOnInteractionMode()
{

}


void ARootModellingPawn::NotifyMeOnInteractionChange()
{

}

FTwoVectors ARootModellingPawn::GetHandRay(float Length)
{
  FVector Start = this->LeftHand->GetComponentLocation();
  FVector Direction = this->LeftHand->GetForwardVector();
  FVector End = Start + Length * Direction;

  return FTwoVectors(Start, End);
}

UPawnMovementComponent* ARootModellingPawn::GetMovementComponent() const
{
  return Movement;
}

void ARootModellingPawn::SelectRoot_Implementation()
{
  bCanDraw = !bCanDraw;
  if (GEngine && false)
  {
    GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("Can Draw: %d"),bCanDraw));
  }

}

void ARootModellingPawn::SplitRoot_Implementation()
{

}

void ARootModellingPawn::ToggleAutoAppend_Implementation()
{
  int RootNum = -1, NewSelectPlace{};
  if (Selection.Num() != 2)
  {
    if (GEngine && false)
    {
      GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Selection is not two"));
    }
  }
  else if(Selection[0]->RootNum != Selection[1]->RootNum || FGenericPlatformMath::Abs(Selection[0]->PlaceSegment - Selection[1]->PlaceSegment) != 1)
  {
    if (Selection[0]->PlaceSegment == 1 && Selection[0]->Predecessor == Selection[1]->RootNum)
    {
      AOverlapTransform* Depend = nullptr;
      for (AOverlapTransform* chck : RegisteredDrawSpace->FindDependencies(Selection[1]))
      {
        if (chck->RootNum == Selection[0]->RootNum && chck->PlaceSegment==0)
        {
          Depend = chck;
        }
      }
      if (Depend)
      {
        AOverlapTransform* temp = RegisteredDrawSpace->SplitSegment(Depend, Selection[0]);
        RootNum = temp->RootNum;
        NewSelectPlace = temp->PlaceSegment;
        for (auto* o : Selection) o->Selected(false);
        Selection.Empty();
        //Selection.Add(temp);
        //temp->Selected(true);
      }
    }
    else if (Selection[1]->PlaceSegment == 1 && Selection[1]->Predecessor == Selection[0]->RootNum)
    {
      AOverlapTransform* Depend = nullptr;
      for (AOverlapTransform* chck : RegisteredDrawSpace->FindDependencies(Selection[0]))
      {
        if (chck->RootNum == Selection[0]->RootNum && chck->PlaceSegment == 0)
        {
          Depend = chck;
        }
      }

      if (Depend)
      {
        AOverlapTransform* temp = RegisteredDrawSpace->SplitSegment(Depend, Selection[1]);
        RootNum = temp->RootNum;
        NewSelectPlace = temp->PlaceSegment;
        for (auto* o : Selection) o->Selected(false);
        Selection.Empty();
        //Selection.Add(temp);
        //temp->Selected(true);
      }
    }
  }
  else
  {
    if (GEngine && false)
    {
      GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, TEXT("Splitting"));
    }
    int u = (Selection[0]->PlaceSegment < Selection[1]->PlaceSegment) ? 0 : 1;
    AOverlapTransform* temp = RegisteredDrawSpace->SplitSegment(Selection[u], Selection[1 - u]);
    RootNum = temp->RootNum;
    NewSelectPlace = temp->PlaceSegment;
    for (auto* o : Selection) o->Selected(false);
    Selection.Empty();
    //Selection.Add(temp);
    //temp->Selected(true);
  }
  ActionTaken.Broadcast();
  //RegisteredDrawSpace->ResetGPU();
  if (RootNum > 0)
  {
    auto* o = RegisteredDrawSpace->FindTransformByRootNumber(RootNum, NewSelectPlace);
    o->Selected(true);
    Selection.Add(o);
  }
}

void ARootModellingPawn::ToggleQuickMode_Implementation()
{

  if (GEngine && false)
  {
    GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Deleting the selection"));
  }
  RegisteredDrawSpace->DeleteSegments(Selection);
  Selection.Empty();
  MoveMeta->SetActorHiddenInGame(true);
  MoveMeta->SetActorEnableCollision(false);
  if(!bDemoVersion)
  {
    DiaMeta->SetActorHiddenInGame(true);
    DiaMeta->SetActorEnableCollision(false);
  }
  bWidgetsVisible = false;
  ActionTaken.Broadcast();
}

void ARootModellingPawn::ToggleHandVis_Implementation()
{

  if (GEngine && false)
  {
    GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Black, TEXT("Toggled Hand vis (not implemented yet)"));
  }



}

void ARootModellingPawn::ToggleModeLeft_Implementation()
{
  SelectionMode = EVRSelectionMode::sel_mode_tog;
 //   switch (SelectionMode)
 //   {
 //   case EVRSelectionMode::sel_mode_add:
 //     SelectionMode = EVRSelectionMode::sel_mode_rem;
 //     if(GEngine && true)
 //     GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Black, TEXT("SelectionMode==Remove"));
 //     break;
 //   case EVRSelectionMode::sel_mode_rem:
 //     SelectionMode = EVRSelectionMode::sel_mode_tog;
 //     if (GEngine && false)
 //     GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Black, TEXT("SelectionMode==Toggle"));
 //     break;
 //   case EVRSelectionMode::sel_mode_tog:
 //     SelectionMode = EVRSelectionMode::sel_mode_add;
 //     if (GEngine && false)
 //     GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Black, TEXT("SelectionMode==Add"));
 //     break;
 //   default:
 //     break;
 //   }
}

void ARootModellingPawn::TakeScreenshot_Implementation()
{
//   ACameraActor* screenshotcamera = GetWorld()->SpawnActor<ACameraActor>(ACameraActor::StaticClass(),Head->GetComponentTransform(), FActorSpawnParameters());
// 
//   auto config = FHighResScreenshotConfig();
// 
//   screenshotcamera->Destroy();
}

void ARootModellingPawn::ToggleFixSegmentSize(bool bNewSegmentPolicy)
{
  ResetInteraction();
  bFixSegmentSize = bNewSegmentPolicy;
}

void ARootModellingPawn::HighlightEndingDraw(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
  AOverlapTransform* OtherCasted = Cast< AOverlapTransform >(OtherActor);
  if (OtherCasted)
  {
    if (!OtherCasted->bSelected && SelectionMode != EVRSelectionMode::sel_mode_rem)
    {
      OtherCasted->HighLight(1);
    }
    else if(SelectionMode != EVRSelectionMode::sel_mode_add)
    {
      OtherCasted->HighLight(-1);
    }
  }
}

void ARootModellingPawn::StopHighlightEndingDraw(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
  AOverlapTransform* OtherCasted = Cast< AOverlapTransform >(OtherActor);
  if (OtherCasted)
  {
    OtherCasted->HighLight(0);
  }
}

void ARootModellingPawn::SelectionAdd(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
  TArray<AOverlapTransform*>::SizeType index = 0;
  AOverlapTransform* OtherCasted = Cast< AOverlapTransform > (OtherActor);
  if(OtherCasted)
  {
    if (Selection.Find(OtherCasted, index))
    {
      if(!bAddToSelection)
      {
        Selection.RemoveAt(index);
        OtherCasted->HighLight(0);
      }
    }
    else
    {
      if (bAddToSelection)
      {
        OtherCasted->Selected(true);
        Selection.Add(OtherCasted);
      }
    }
  }
}

void ARootModellingPawn::SelectionExit(class UPrimitiveComponent* HitComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

}

void ARootModellingPawn::TogglePerfMeasure()
{
  if (bMeasuringPerformance)
  {
    APlayerController* PController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PController)
    {
      PController->ConsoleCommand(TEXT("csvprofile stop"), true);
    }
    bMeasuringPerformance = false;
  }
  else
  {
    APlayerController* PController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PController)
    {
      PController->ConsoleCommand(TEXT("csvprofile start"), true);
    }
    bMeasuringPerformance = true;
  }
}

void ARootModellingPawn::OnBeginFire_Implementation()
{
  if(bCanDraw && (!RegisteredDrawSpace->HasRoot() || Selection.Num() > 0))
    bRightFire = true;
  else
  {
    if (GEngine && false)
    {
      GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("CANNOT DRAW")));
    }
  }
  // |Selection| == 1 -> Draw Mode entered
  // 
  // 
  // 
  // 
  // 
  // 
  // 
  // 
  // 
  // 
  // 
  //

  if (GEngine && false)
  {
    GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, TEXT("Fire Right"));
  }
  
  TArray<AActor*> OverlappingEndings;
  this->DrawIndicator->GetOverlappingActors(OverlappingEndings, AOverlapTransform::StaticClass());
  if (OverlappingEndings.Num() > 0)
  {
    bRightFire = false;
    if (GEngine && false)
    {
      GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("were overlapping something"));
    }
    for (AActor* ovlpact : OverlappingEndings)
    {
      AOverlapTransform* ovl = Cast<AOverlapTransform>(ovlpact);
      switch (SelectionMode)
      {
      case EVRSelectionMode::sel_mode_add:
          ovl->bTemporarilyVisible = false;
         Selection.AddUnique(ovl);
          ovl->Selected(true);
          RegisteredDrawSpace->SyncRadius(ovl);
        break;
      case EVRSelectionMode::sel_mode_rem:
        Selection.Remove(ovl);
          ovl->Selected(false);
        break;
      case EVRSelectionMode::sel_mode_tog:
        ToggleSelection(ovlpact);
        break;
      }
    }
    if (Selection.Num() >= 1)
    {
      // spawn the interaction spheres
      // we then put the root segments to the sphere attach
      // and the spheres always resume their original position
      MoveMeta->SetActorHiddenInGame(false);
      MoveMeta->SetActorEnableCollision(true);
      if(!bDemoVersion)
      {
        DiaMeta->SetActorHiddenInGame(false);
        DiaMeta->SetActorEnableCollision(true);
      }
//       MoveMeta->SetActorLocation({});
//       DiaMeta->SetActorLocation({});
      bWidgetsVisible = true;

      // TODO HERE NEXT
    }
    else
    {
      MoveMeta->SetActorHiddenInGame(true);
      MoveMeta->SetActorEnableCollision(false);
      if(!bDemoVersion)
      {
        DiaMeta->SetActorHiddenInGame(true);
        DiaMeta->SetActorEnableCollision(false);
      }
      bWidgetsVisible = false;
    }
    return;
  }
  if (Selection.Num() > 1)
  {
    // nothing because we cannot draw
    bRightFire = false;
  }
  else if(DrawMode == EVRDrawModes::draw_mode_free && bRightFire)
  {
    //OriginPosition = DrawIndicator->GetComponentTransform();
    
    
    if (Selection.Num() == 1 && bRightFire)
    {
      //if(Selection[0]->RootNum == 1 && Selection[0]->PlaceSegment == 0)
      //{
      //  bRightFire = false;
      //  return;
      //}
      this->DrawIndicator->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
      this->DrawIndicator->SetVectorParameterValueOnMaterials(TEXT("LUT"), FVector(1, 0, 0));
      bSetNewRoot = RegisteredDrawSpace->NeedsInitialRadius(Selection[0]); // MARKER
      FTransform CopyTransform = DrawIndicator->GetComponentTransform();
      this->RegisteredDrawSpace->PrepareSegmentFromIndicator(Cast<AOverlapTransform>(Selection[0]), CopyTransform);
      this->OriginPosition = Selection[0]->GetActorTransform();
      if (GEngine && false)
      {
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Free and firing WITH a selection"));
      }
      // dehighlight the thing (or everything)
      // affix the indicator at the hand and make line towards the goal by setting the POSITION of the ray
      // which i assume goes from attachparents to the position
    }
    else if (Selection.Num() == 0 && bRightFire)
    {
      FTransform CopyTransform = DrawIndicator->GetComponentTransform();
      this->DrawIndicator->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
      this->DrawIndicator->SetVectorParameterValueOnMaterials(TEXT("LUT"), FVector(1, 0, 0));
      this->DrawIndicator->GetOverlappingActors(OverlappingEndings, AOverlapTransform::StaticClass());
      this->DrawConnection->SetWorldLocation(this->DrawIndicator->GetComponentLocation());

      this->RegisteredDrawSpace->PrepareSegmentFromIndicator(nullptr, CopyTransform);
      this->OriginPosition = DrawIndicator->GetComponentTransform();
      bSetNewRoot = true;
    }

    DrawIndicator->AttachToComponent(RightHand, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    DrawIndicator->SetRelativeLocation(DefaultDrawIndicatorPosition);
    DrawIndicator->SetWorldScale3D(DefaultDrawIndicatorScale);
    //DrawIndicator->SetRelativeLocation({ 0,0,0 });
    //DrawIndicator->SetRelativeRotation(FQuat::FindBetweenVectors({ 1,0,0 }, { -1,0,0 }));
    DrawConnection->SetWorldLocation(DrawIndicator->GetComponentLocation());
    NewNodeIndicator->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
    NewNodeIndicator->SetWorldLocation(OriginPosition.GetLocation());
    NewNodeIndicator->SetVisibility(true,true);
    DrawMode = EVRDrawModes::draw_mode_drag;
    return;
  }

  

}

void ARootModellingPawn::ToggleSelection(AActor* Overlappor)
{
  AOverlapTransform* Ovlptrnsfrm = Cast<AOverlapTransform>(Overlappor);
  TArray<AOverlapTransform*>::SizeType index = 0;
  if (Selection.Find(Ovlptrnsfrm, index))
  {
    Selection.RemoveAt(index);
    Ovlptrnsfrm->Selected(false);
  }
  else
  {
    Ovlptrnsfrm->Selected(true);
    Selection.Add(Ovlptrnsfrm);
    RegisteredDrawSpace->SyncRadius(Ovlptrnsfrm);
  }
}

void ARootModellingPawn::OnEndFire_Implementation()
{
  if (GEngine && false)
  {
    GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Fire Right End"));
  }
  if(bRightFire == true)
    bRightFire = false;
  else
    return;

  if(DrawMode == EVRDrawModes::draw_mode_drag)
  {
    NewNodeIndicator->SetVisibility(false, true);
    NewNodeIndicator->AttachToComponent(RightHand, FAttachmentTransformRules::SnapToTargetIncludingScale);
    FTransform CopyTransform = DrawIndicator->GetComponentTransform();
    CopyTransform.SetScale3D({RegisteredDrawSpace->CurrentStoredRadius,RegisteredDrawSpace->CurrentStoredRadius,RegisteredDrawSpace->CurrentStoredRadius});
    AOverlapTransform*  Newselect = 
      RegisteredDrawSpace->MakeSegmentFromIndicator(CopyTransform);
    DrawIndicator->AttachToComponent(RightHand, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    DrawIndicator->SetRelativeLocation(DefaultDrawIndicatorPosition);
    DrawIndicator->SetWorldScale3D(DefaultDrawIndicatorScale);
    if (Selection.Num() == 0)
    {
      bWidgetsVisible = true;
      if(!bDemoVersion)
      {
        DiaMeta->SetActorHiddenInGame(false);
        DiaMeta->SetActorEnableCollision(true);
      }
      MoveMeta->SetActorHiddenInGame(false);
      MoveMeta->SetActorEnableCollision(true);
    }
    for(auto* s : Selection) s->Selected(false);
    Selection.Empty();
    Newselect->Selected(true);
    Newselect->SetTemporary(false);
    if(IsValid(Newselect))
      Selection.Add(Newselect);
    // we NEED the new actor that is spawned for selection here!
    // then we add that to the selection and reset the draw mode
    // either here or we actually just take it with the

    DrawConnection->SetRelativeLocation({ 0,0,0 });
    // TODO Update selection to contain the new overlapper
    DrawMode = EVRDrawModes::draw_mode_free;
    ActionTaken.Broadcast();
  }
  if (GrabbedActor != nullptr)
  {
    // let the grabbed object reacot to release
    Cast<IGrabable>(GrabbedActor)->OnReleased_Implementation();
    AMenuWidget* MovedSomeStuff = Cast<AMenuWidget>(GrabbedActor);
    if (MovedSomeStuff && MovedSomeStuff->bNeedReleaseOfComponents)
    {
      TArray<AActor*> AttachAct;
      MovedSomeStuff->GetAttachedActors(AttachAct);
      for (AActor* CheckForAttach : AttachAct)
      {
        AOverlapTransform* CheckForSelection = Cast<AOverlapTransform>(CheckForAttach);
        if (CheckForSelection)
        {
          CheckForSelection->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
          CheckForSelection->AttachToActor(RegisteredDrawSpace, FAttachmentTransformRules::KeepWorldTransform);
        }
      }
    }

    // Detach the Actor

    UPrimitiveComponent* PhysicsComp = GrabbedActor->FindComponentByClass<UPrimitiveComponent>();
    UGrabbingBehaviorComponent* Behavior = GrabbedActor->FindComponentByClass<UGrabbingBehaviorComponent>();
    //UGrabbingBehaviorComponent* Behavior = nullptr;
    if (Behavior == nullptr)
    {
      GrabbedActor->GetRootComponent()->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
      PhysicsComp->SetSimulatePhysics(bDidSimulatePhysics);
    }

    // forget about the actor
    GrabbedActor = nullptr;

  }
}

void ARootModellingPawn::OnBeginLeftFire_Implementation()
{
  // select only when draw mode free otherwise go on with drawing
  // reset button when drawing does reset the drawing process
  // otherwise reset is resetti spaghetti seletti
  if(bRightFire == true || DrawMode != EVRDrawModes::draw_mode_free)
    return;
  bLeftFire = true;

  FTwoVectors StartEnd = GetHandRay(MaxClickDistance);
  FVector Start = StartEnd.v1;
  FVector End = StartEnd.v2;

  // will be filled by the Line Trace Function
  bool bImportant = false;
  FHitResult Hit;
  AActor* HitActor;
  TArray<FHitResult> MHits;
  //if hit was not found return  
  FVector origin,extends;
//   RegisteredDrawSpace->GetActorBounds(false,origin,extends);
//   if (UKismetMathLibrary::IsPointInBox(LeftHand->GetComponentLocation(), origin, extends))
//   {
//     NewParams.AddIgnoredActor(RegisteredDrawSpace);
//   }
  
  if ((!GetWorld()->LineTraceMultiByObjectType(MHits, Start, End, Params, NewParams) || MHits.Num() == 0) && bUseHMD)
  {
      ControllerDrag = LeftHand->GetComponentTransform();
      auto vec = LeftHand->GetForwardVector();
      float zcomp = LeftHand->GetForwardVector().Z;
      float planecomp = LeftHand->GetForwardVector() | FVector(0, 0, 1);
	  UE_LOG(LogTemp, Warning, TEXT("Trigger went in the direction of (%d,%d,%d) with projection %d"), vec.X, vec.Y, vec.Z, planecomp);
	  this->RegisteredDrawSpace->GetActorBounds(false, DrawOrigin, this->DrawBounds, false);
      if (planecomp < 0.1)
      {
        NavigationMode = EVRNavigationModes::nav_mode_zoom;
        DrawScale = RegisteredDrawSpace->GetActorScale3D();
        DrawOrigin = RegisteredDrawSpace->GetActorLocation();
        // TODO DRAW BOUNDS
        DrawForward = LeftHand->GetForwardVector();
      }
      else if (planecomp > 0.9)
      {
		    NavigationMode = EVRNavigationModes::nav_mode_scroll;
        DrawOrigin = RegisteredDrawSpace->GetActorLocation();
      }
  }
  else if(MHits.Num() > 0)
  {
    for (const FHitResult& TfoB : MHits)
    {
      if (GEngine && false)
      {
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, TfoB.GetActor()->GetFName().ToString());
      }
      if (TfoB.GetActor() == DiaMeta)
      {
        ZeroHighlight->SetActorHiddenInGame(false);
        ZeroHighlight->SetActorLocation(DiaMeta->GetActorLocation());

        bImportant = true;
        Hit = TfoB;
        break;
      }
      else if (TfoB.GetActor() == MoveMeta)
      {
        bImportant = true;
        Hit = TfoB;
        break;
      }
    }
    if (!bImportant)
    {
      Hit = MHits[0];
    }

    HitActor = Hit.GetActor();

    // try to cast HitActor int a Grabable if not succeeded will become a nullptr
    IGrabable* GrabableActor = Cast<IGrabable>(HitActor);
    IClickable* ClickableActor = Cast<IClickable>(HitActor);

    if (GEngine && false)
    {
      GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, HitActor->GetFName().ToString());
    }

    // attention we do not want to grapple anything so long as we are drawing
    // reset the drawing first and then try to grab anything
    if (GrabableActor != nullptr && Hit.Distance < MaxGrabDistance && DrawMode == EVRDrawModes::draw_mode_free)
    {
      // call grabable actors function so he reacts to our grab
      GrabableActor->OnGrabbed_Implementation();

      if (GrabableActor == MoveMeta || GrabableActor == DiaMeta)
      {
        Dependend.Empty();
        OriginPosition.SetRotation(LeftHand->GetComponentQuat());
        for (AOverlapTransform* Selected : Selection)
        {
          Dependend.Append(RegisteredDrawSpace->FindDependencies(Selected));
          Selected->SaveState();
          if (GEngine && false)
          {
            GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, Selected->OriginScale.ToString());
          }
        }
        for (AOverlapTransform* Depend : Dependend)
        {
          Depend->HighLight(1);
          Depend->SaveState();
        }
      }
      if (GrabableActor == MoveMeta)
      {
        for (AOverlapTransform* Selected : Selection)
        {
          Selected->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
          Selected->AttachToComponent(LeftHand, FAttachmentTransformRules::KeepWorldTransform);
        }
        for (AOverlapTransform* Depend : Dependend)
        {
          Depend->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
          Depend->AttachToComponent(LeftHand, FAttachmentTransformRules::KeepWorldTransform);
        }
      }

      UGrabbingBehaviorComponent* Behavior = HitActor->FindComponentByClass<UGrabbingBehaviorComponent>();
      if (Behavior == nullptr)
        HandlePhysicsAndAttachActor(HitActor);
      else
      {

      }

      // we save the grabbedActor in a general form to access all of AActors functions easily later
      GrabbedActor = HitActor;
    }
    else if (ClickableActor != nullptr && Hit.Distance < MaxClickDistance)
    {
      ClickableActor->OnClicked_Implementation(Hit.Location);
    }
  }
  // if no actor has been hit, we enter a different interaction mode. this is only relevant with an hmd

}

void ARootModellingPawn::OnEndLeftFire_Implementation()
{
  bLeftFire = false;
  if(bUseHMD)
    NavigationMode = EVRNavigationModes::nav_mode_none;
  // if we didnt grab anyone there is no need to release
  if (GrabbedActor == nullptr)
    return;

  if (GrabbedActor == MoveMeta)
  {
    for (AOverlapTransform* Selected : Selection)
    {
      Selected->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
      Selected->AttachToActor(RegisteredDrawSpace, FAttachmentTransformRules::KeepWorldTransform);
    }
    for (AOverlapTransform* Depend : Dependend)
    {
      Depend->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
      Depend->AttachToActor(RegisteredDrawSpace, FAttachmentTransformRules::KeepWorldTransform);
    }
    ActionTaken.Broadcast();
  }
  if (GrabbedActor == DiaMeta || GrabbedActor == MoveMeta)
  {
    for (AOverlapTransform* Depend : Dependend)
    {
      Depend->HighLight(0);
    }
    Dependend.Append(Selection);
    RegisteredDrawSpace->UpdateSections(Dependend,bFixSegmentSize,bAdjustRadiusDuringDrawing);
    ZeroHighlight->SetActorHiddenInGame(true);
    Dependend.Empty();
    ActionTaken.Broadcast();
  }
  // let the grabbed object reacot to release
  Cast<IGrabable>(GrabbedActor)->OnReleased_Implementation();

  // Detach the Actor

  UPrimitiveComponent* PhysicsComp = GrabbedActor->FindComponentByClass<UPrimitiveComponent>();
  UGrabbingBehaviorComponent* Behavior = GrabbedActor->FindComponentByClass<UGrabbingBehaviorComponent>();
  //UGrabbingBehaviorComponent* Behavior = nullptr;
  if (Behavior == nullptr)
  {
    GrabbedActor->GetRootComponent()->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
    PhysicsComp->SetSimulatePhysics(bDidSimulatePhysics);
  }

  // forget about the actor
  GrabbedActor = nullptr;
}

void ARootModellingPawn::ResetInteraction_Implementation()
{
  if (DrawMode != EVRDrawModes::draw_mode_free)
  {
    NewNodeIndicator->SetVisibility(false, true);
    NewNodeIndicator->AttachToComponent(RightHand, FAttachmentTransformRules::SnapToTargetIncludingScale);
    DrawIndicator->AttachToComponent(RightHand, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    DrawIndicator->SetRelativeLocation(DefaultDrawIndicatorPosition);
    DrawIndicator->SetWorldScale3D(DefaultDrawIndicatorScale);

    DrawMode = EVRDrawModes::draw_mode_free;
  }
  else if (Selection.Num() > 0)
  {
    for(AOverlapTransform* act : Selection)
    {
      act->Selected(false);
    }
    Selection.Empty();

    MoveMeta->SetActorHiddenInGame(true);
    MoveMeta->SetActorEnableCollision(false);
      if(!bDemoVersion)
      {
        DiaMeta->SetActorHiddenInGame(true);
        DiaMeta->SetActorEnableCollision(false);
      }
    bWidgetsVisible = false;
  }
}

void ARootModellingPawn::Tick(float DeltaSeconds)
{
  Super::Tick(DeltaSeconds);
  if(!(DrawIndicator && RegisteredDrawSpace))
    return;

  // Scene Update not relevant for Tick() logic
  if (bWidgetsVisible && GrabbedActor != MoveMeta && GrabbedActor != DiaMeta && Selection.Num() > 0)
  {
    const FTransform& LookAtTransform = Selection[Selection.Num() - 1]->GetActorTransform();
    FVector DistVec = Head->GetComponentLocation() - LookAtTransform.GetLocation();
    DistVec = FVector::CrossProduct(DistVec, RegisteredDrawSpace->GetActorUpVector());
    DistVec /= DistVec.Size();
    if(bDemoVersion)
    {
    MoveMeta->SetActorLocation(LookAtTransform.GetLocation()
      //+ (100.f + RegisteredDrawSpace->GetActorScale3D().Y) * DistVec
      + (20.f) * DistVec);
    }
    else
    {
      MoveMeta->SetActorLocation(LookAtTransform.GetLocation()
      //+ (100.f + RegisteredDrawSpace->GetActorScale3D().Y) * DistVec
      + (20.f) * DistVec
      + RegisteredDrawSpace->GetActorUpVector() * 20.f);
      DiaMeta->SetActorLocation(LookAtTransform.GetLocation()
      //+ (100.f + RegisteredDrawSpace->GetActorScale3D().Y) * DistVec
      + (20.f) * DistVec
      - RegisteredDrawSpace->GetActorUpVector() * 20.f);
      DiaMeta->SetActorRotation((UKismetMathLibrary::FindLookAtRotation(DiaMeta->GetActorLocation(), Head->GetComponentLocation()).Quaternion())
        * FQuat::FindBetweenVectors({ 1,0,0 }, { 0,-1,0 }));
    }

    // now correct loooking direction
    MoveMeta->SetActorRotation((UKismetMathLibrary::FindLookAtRotation(MoveMeta->GetActorLocation(), Head->GetComponentLocation()).Quaternion())
      * FQuat::FindBetweenVectors({ 1,0,0 }, { 0,-1,0 }));


//     MoveMeta->SetActorLocation((Head->GetForwardVector() * 500.f) + (Head->GetRightVector() * -200.f) + Head->GetComponentLocation() + (Head->GetUpVector() * 50));
//     DiaMeta->SetActorLocation((Head->GetForwardVector() * 500.f) + (Head->GetRightVector() * -200.f) + Head->GetComponentLocation() + (Head->GetUpVector() * -50));
  }

  if (__x && __y)
  {
    __x = false; __y = false;

    if (!TouchInitialized)
    {
      TouchInitialized = true;
    }
    else if (TouchInput.Size() > 0.9 && (TouchLastInput - TouchInput).Size() > 0.01)
    {
      FVector PivotDistance = RegisteredDrawSpace->HightlightActor->GetRelativeScale3D() / 2.f;
      auto increment = FRotator::MakeFromEuler(
        FVector(0.f, 0.f, -FGenericPlatformMath::Sign(FVector::CrossProduct(TouchLastInput, TouchInput).Z)
          * ((TouchInput | TouchLastInput) / (TouchInput.Size() * TouchLastInput.Size())))
      );
      RegisteredDrawSpace->AddActorWorldRotation(
        increment
      );
      //RegisteredDrawSpace->AddActorWorldOffset(-(PivotDistance - increment.RotateVector(PivotDistance)));
    }
    TouchLastInput = TouchInput;
    InvalidationTimer = 0.5f;
  }
  else
  {
    InvalidationTimer -= DeltaSeconds;
    if (InvalidationTimer <= 0.f)
    {
      TouchInitialized = false;
    }
  }

  if (NavigationMode == EVRNavigationModes::nav_mode_zoom)
  {
    // intended behavior:
    // resting position: original scale
    // arm forward by half an arm length: +50% scale
    // arm backward by half an arm length: -50% scale
    FVector PivotDistance = RegisteredDrawSpace->HightlightActor->GetRelativeScale3D() / 2.f;
    FVector distance = ControllerDrag.GetLocation() - LeftHand->GetComponentLocation();
    float projected = (distance | DrawForward) / ArmLength;
    projected = (projected < 0.f) ? 0.5f*projected : projected;
    projected = (projected * (DrawScale.X)) + (DrawScale.X);
    projected = FGenericPlatformMath::Max(0.05f,FGenericPlatformMath::Min(20.f,projected));
    RegisteredDrawSpace->SetActorScale3D(FVector(projected, projected, projected));
    //RegisteredDrawSpace->SetActorLocation((projected - 1.f)*PivotDistance);
  }
  else if (NavigationMode == EVRNavigationModes::nav_mode_scroll)
  {
    FVector distance = ControllerDrag.GetLocation() - LeftHand->GetComponentLocation();
    float projected = distance.Z / ArmLength * DrawBounds.Z;
    RegisteredDrawSpace->SetActorLocation(DrawOrigin + FVector(0.f,0.f,-projected));
  }

  else if (GrabbedActor != nullptr)
  {
    if (GrabbedActor == DiaMeta)
    {
      FVector AttachmentPoint = OriginPosition.GetLocation();
      FVector ConstraintAxis = OriginPosition.GetRotation().GetRightVector();
      FVector Direction = LeftHand->GetForwardVector();
      FVector Temp = FVector::CrossProduct(AttachmentPoint - LeftHand->GetComponentLocation(), ConstraintAxis);
      Temp.Normalize();
      FVector PlaneNormal = FVector::CrossProduct(ConstraintAxis, Temp);
      FVector Intersection = FMath::LinePlaneIntersection(LeftHand->GetComponentLocation(), LeftHand->GetComponentLocation() + Direction, AttachmentPoint, PlaneNormal);
      FVector FromOriginToIntersection = Intersection - AttachmentPoint;
      FVector NewPosition = FVector::DotProduct(FromOriginToIntersection, ConstraintAxis) * ConstraintAxis;
      float zsize = ZeroHighlight->GetActorScale().X;
      float distance = (DiaMeta->GetActorLocation() - ZeroHighlight->GetActorLocation()).Size() - (0.5f*zsize);
      FVector newscale2 = FVector(distance/ zsize + 1, distance/ zsize + 1, distance/ zsize + 1);
      float comp = (FVector::DotProduct({0.33,0.33,0.33},NewPosition) / 10.f);
      float ocomp = comp;
      float sign = ((comp) < 0.f) ? -10.f : 10.f;
      comp = FGenericPlatformMath::Abs(comp);
      FVector newscale;
//       if (GEngine && false)
//       {
//         GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("All: %f, %f, %f, %f"), comp, sign, (1.f / FGenericPlatformMath::Log2(2.f + comp)), FGenericPlatformMath::Log2(2.f + comp)));
//       }
      for (AOverlapTransform* ovlp : Selection)
      {
        
//         if (sign < -0001.f)
//         {
//           newscale = ovlp->OriginScale;
// //           if (GEngine && false)
// //           {
// //             GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, newscale.ToString());
// //           }
//           newscale *= (1.5f / FGenericPlatformMath::Log2(2.f + comp));
// //           if (GEngine && false)
// //           {
// //             GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("All: %f"), (1.f / FGenericPlatformMath::Log2(2.f + comp))));
// //           }
// //           if (GEngine && false)
// //           {
// //             GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, newscale.ToString());
// //           }
//         }
//         else if (sign > 0.001f)
//         {
//           newscale = ovlp->OriginScale * (FGenericPlatformMath::Log2(4.f + comp) - 1.f);
//         }
//         newscale = newscale.GetAbs();
        //newscale = newscale.GetClampedToSize(0.001,1000);
        ovlp->SetActorRelativeScale3D(newscale2 * ovlp->OriginScale);
      }
      for (AOverlapTransform* ovlp : Dependend)
      {
//         if (sign < -0001.f)
//         {
//           newscale = ovlp->OriginScale;
//           newscale *= (2.f / FGenericPlatformMath::Log2(4.f + comp));
//         }
//         else if (sign > 0.001f)
//         {
//           newscale = ovlp->OriginScale * (FGenericPlatformMath::Log2(8.f + comp) - 2.f);
//         }
//         newscale = newscale.GetAbs();
        ovlp->SetActorRelativeScale3D(newscale2* ovlp->OriginScale);
      }
      return;
    }

    UGrabbingBehaviorComponent* Behavior = GrabbedActor->FindComponentByClass<UGrabbingBehaviorComponent>();
    //UGrabbingBehaviorComponent* Behavior = nullptr;
    // if our Grabable Actor is not constrained
    if (Behavior != nullptr)
    {
      // specifies the hand in space
      FVector HandPos = this->LeftHand->GetComponentLocation();
      FQuat HandQuat = this->LeftHand->GetComponentQuat();

      Behavior->HandleNewPositionAndDirection(HandPos, HandQuat);
    }
    return;
  }

  // if this is an annoyance we change it but usually it should be fine because
  // this is really only for a completely new root
  else if (DrawMode == EVRDrawModes::draw_mode_free && !bFiring)
    DrawIndicator->SetWorldRotation(FQuat::FindBetweenVectors({1.f,0.f,0.f},
        -RegisteredDrawSpace->GetActorUpVector()));
  else if (DrawMode == EVRDrawModes::draw_mode_drag)
  {
    FVector diff = DrawIndicator->GetComponentLocation() - OriginPosition.GetLocation();
    DrawIndicator->SetWorldRotation(diff.ToOrientationRotator());
    DrawConnection->SetBeamSourcePoint(0, DrawIndicator->GetRelativeLocation(), 0);
    DrawConnection->SetBeamEndPoint(0, (OriginPosition * (GetActorTransform().Inverse())).GetLocation());
    NewNodeIndicator->SetWorldScale3D({diff.Size(), RegisteredDrawSpace->CurrentStoredRadius,RegisteredDrawSpace->CurrentStoredRadius});
    NewNodeIndicator->SetWorldRotation(UKismetMathLibrary::FindLookAtRotation(
    OriginPosition.GetLocation(),
      DrawIndicator->GetComponentLocation()).Quaternion());

    if (bFixMaxLength && diff.Size() > this->fMaximumSegmentLength)
    {
      FVector controlleddiff = (diff / diff.Size()) * (diff.Size() - this->fMaximumSegmentLength);
      NewNodeIndicator->SetWorldScale3D({ this->fMaximumSegmentLength, 5.f,5.f });
      //DrawIndicator->SetWorldLocation(RightHand->GetComponentLocation() - controlleddiff);
      if (GEngine && false)
      {
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, FString::Printf(TEXT("TV ")).Append(diff.ToString()).Append(TEXT(" - ")).Append(controlleddiff.ToString()));
      }
    }
  }
}

void ARootModellingPawn::OnRotationTouchX_Implementation(float Value)
{
  if(Value != 0.f && !__x)
  {
    TouchInput.X = Value;
    __x = true;
  }
}

void ARootModellingPawn::OnRotationTouchY_Implementation(float Value)
{
  if(Value != 0.f && !__y)
  {
    TouchInput.Y = Value;
    __y = true;
  }
}

void ARootModellingPawn::MoveSelectionUp_Implementation()
{
  UE_LOG(LogTemp,Display,TEXT("Going UP the root"));
  AOverlapTransform * NewSelect;
  if(Selection.Num() == 1)
  {
    NewSelect = RegisteredDrawSpace->MoveSelection(Selection[0],true);
    if(NewSelect)
    {
      Selection[0]->Selected(false);
      Selection.Empty();
      Selection.Add(NewSelect);
      Selection[0]->Selected(true);
      RegisteredDrawSpace->SyncRadius(NewSelect);
    }
  }
}

void ARootModellingPawn::MoveSelectionDown_Implementation()
{
  UE_LOG(LogTemp,Display,TEXT("Going DOWN the root"));
  AOverlapTransform * NewSelect;
  if(Selection.Num() == 1)
  {
    NewSelect = RegisteredDrawSpace->MoveSelection(Selection[0],false);
    if(NewSelect)
    {
      Selection[0]->Selected(false);
      Selection.Empty();
      Selection.Add(NewSelect);
      Selection[0]->Selected(true);

      RegisteredDrawSpace->SyncRadius(NewSelect);
    }
  }
}

void ARootModellingPawn::SelectEverything_Implementation()
{
  Selection.Empty();
  TArray<AActor*> Overlaps;
  UGameplayStatics::GetAllActorsOfClass(GetWorld(), AOverlapTransform::StaticClass(), Overlaps);
  for (AActor* act : Overlaps)
  {
    AOverlapTransform* oact = Cast<AOverlapTransform>(act);
    if (oact)
    {
      oact->Selected(true);
      Selection.Add(oact);
    }
  }
    MoveMeta->SetActorHiddenInGame(false);
    MoveMeta->SetActorEnableCollision(true);
      if(!bDemoVersion)
      {
        DiaMeta->SetActorHiddenInGame(false);
        DiaMeta->SetActorEnableCollision(true);
      }
  return;
}

void ARootModellingPawn::OnRise_Implementation(float Value)
{
  if (Value != 0.f && IsValid(Head))
  {
    AddMovementInput(Head->GetUpVector(), Value);
  }
}
void ARootModellingPawn::OnSink_Implementation(float Value)
{
  if (Value != 0.f && IsValid(Head))
  {
    AddMovementInput(Head->GetUpVector(), Value);
  }
}
