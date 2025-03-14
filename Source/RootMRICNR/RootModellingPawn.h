#pragma once

#include "CoreMinimal.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "MotionControllerComponent.h"
#include <GameFramework/Pawn.h>
#include <functional>
#include <Components/SphereComponent.h>
#include <Camera/CameraComponent.h>
#include "Particles/ParticleSystemComponent.h"
#include "Public/DrawSpace.h"
#include "RootCone.h"
#include <GameFramework/Actor.h>
#include <TimerManager.h>
#include "RootModellingPawn.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FActionDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FUndoDelegate);

UENUM(BlueprintType)
enum class EVRNavigationModes : uint8
{
  nav_mode_none UMETA(DisplayName = "Navigation Mode None"),
  nav_mode_fly UMETA(DisplayName = "Navigation Mode Fly"),
  nav_mode_scroll UMETA(DisplayName = "Navigation Mode Scroll"),
  nav_mode_zoom UMETA(DisplayName = "Navigation Mode Zoom"),
  nav_mode_rot UMETA(DisplayName = "Navigation Mode Rotation"),
};

UENUM(BlueprintType)
enum class EVRInteractionModes : uint8
{
  int_mode_edit UMETA(DisplayName = "Interaction Mode Edit"),
  int_mode_draw UMETA(DisplayName = "Interaction Mode Draw"),
  int_mode_menu UMETA(DisplayName = "Interaction Mode Menu")
};

UENUM(BlueprintType)
enum class EVRSelectionMode : uint8
{
  sel_mode_add UMETA(DisplayName = "Selection Mode ADD"),
  sel_mode_rem UMETA(DisplayName = "Selection Mode REM"),
  sel_mode_tog UMETA(DisplayName = "Selection Mode TOG")
};

UENUM(BlueprintType)
enum class EVRDrawModes : uint8
{
  draw_mode_free UMETA(DisplazName = "Draw Mode Free"),
  draw_mode_drag UMETA(DisplazName = "Draw Mode Drag"),
  draw_mode_scale UMETA(DisplazName = "Draw Mode Scale"),
  draw_mode_pend UMETA(DisplazName = "Draw Mode Pending")
};

class UPrimitiveComponent;
class AOverlapTransform;
class AMenuWidget;



UCLASS()
class ROOTMRICNR_API ARootModellingPawn : public APawn
{
  GENERATED_UCLASS_BODY()

  // Pawn Components that are needed beforehand


protected:
  /** Camera component */
  UPROPERTY(VisibleAnywhere, Category = "Pawn") UCameraComponent* CameraComponent;

  /** Collision component */
  UPROPERTY(Category = Pawn, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
  USphereComponent* CollisionComponent;
public:
  inline USphereComponent* GetCollisionComponent() const
  {
    return CollisionComponent;
  }

  inline UCameraComponent* GetCameraComponent() const
  {
    return CameraComponent;
  }

public:
  /** Scene component. Specifies translation (DisplayCluster hierarchy navigation) direction. */
  UPROPERTY(EditAnywhere, Category = "BasicPawn")
  USceneComponent* TranslationDirection;

  /** Scene component. Specifies rotation center (DisplayCluster hierarchy rotation). */
  UPROPERTY(EditAnywhere, Category = "BasicPawn")
  USceneComponent* RotationAround;


public:

  /************************************************************************/
  /* Category Pawn                                                        */
  /************************************************************************/

  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void OnForward(float Value);
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void OnRight(float Value);
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void OnTurnRate(float Rate);
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void OnLookUpRate(float Rate);
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void OnScale(float Rate);
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void OnIndiScale(float Rate);
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void OnRotationTouchX(float Value);
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void OnRotationTouchY(float Value);
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void OnBeginFire();
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void OnEndFire();
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void OnBeginLeftFire();
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void OnEndLeftFire();
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void OnRise(float Value);
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void OnSink(float Value);
  
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void OnJumpAxisChange(float Value);
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void SplitRoot();
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void SelectRoot();
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void ToggleAutoAppend();
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void ToggleQuickMode();
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void ToggleHandVisDown();
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void ToggleHandVisUp();
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void ToggleModeLeft();
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void TakeScreenshot();
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void MoveSelectionUp();
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void MoveSelectionDown();
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void SelectEverything();
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void OnRotate(float Rate);
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void OnChangeHeight(float Value);
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void ToggleMoveDown();
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void ToggleMoveUp();


  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pawn") void ResetInteraction();

  UPROPERTY() bool bRightFire = false;
  UPROPERTY() bool bScalingActive = false;
  UPROPERTY() bool bLeftFire = false;
  UFUNCTION() void ToggleSelection(AActor* Overlappor);

  UFUNCTION(Category = "Pawn") void NotifyMeOnInteractionChange();
  UFUNCTION(Category = "Pawn") float GetBaseTurnRate() const;
  UFUNCTION(Category = "Pawn") void SetBaseTurnRate(float Value);
  UFUNCTION(Category = "Pawn") UFloatingPawnMovement* GetFloatingPawnMovement();
  UFUNCTION(Category = "Pawn") URotatingMovementComponent* GetRotatingMovementComponent();

  UFUNCTION(Category = "Pawn") UMotionControllerComponent* GetHmdLeftMotionControllerComponent();
  UFUNCTION(Category = "Pawn") UMotionControllerComponent* GetHmdRightMotionControllerComponent();
  UFUNCTION(Category = "Pawn") USceneComponent* GetHeadComponent();
  UFUNCTION(Category = "Pawn") USceneComponent* GetLeftHandComponent();
  UFUNCTION(Category = "Pawn") USceneComponent* GetRightHandComponent();
  UFUNCTION(BlueprintCallable, Category = "XR") void AfterDeleteResetInteraction();
  UFUNCTION(BlueprintCallable, Category = "XR") bool CheckIsHeadMountedModeActive();

  /************************************************************************/
  /* Category Draw                                                        */
  /************************************************************************/

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Draw") UStaticMeshComponent* DrawIndicator;
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Draw") UMaterial* CylinderMaterial;


  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Draw") ADrawSpace* RegisteredDrawSpace;
  UPROPERTY(VisibleAnywhere, Category = "Draw") EVRDrawModes DrawMode = EVRDrawModes::draw_mode_free;
  UPROPERTY() URootCone* PendingCylinderMesh = nullptr;
  UPROPERTY() FTransform OriginPosition;
  UPROPERTY() FTransform ScaleReferencePoint;
  UPROPERTY() float lastWidth = 2.f;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Draw")
  FVector DefaultDrawIndicatorPosition = FVector(50.f, 0.f, 0.f);
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Draw")
  //FVector DefaultDrawIndicatorScale = FVector(20.f, 5.f, 5.f);
  FVector DefaultDrawIndicatorScale = FVector(50.f, 50.f, 50.f);

  /************************************************************************/
  /* Category Draw New System                                             */
  /************************************************************************/
  UStaticMeshComponent* NewNodeIndicator;

  bool bSetNewRoot = false;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Config, Category = "Draw")
  float fMaximumSegmentLength = 100.f;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Config, Category = "Draw")
  bool bDemoVersion = false;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Draw")
  bool bFixMaxLength = false;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Draw")
  bool bFixSegmentSize = false;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Config, Category = "System")
  bool bUseHMD = false;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Config, Category = "Draw")
  bool bAdjustRadiusDuringDrawing = true;
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Edit")
  class AStaticMeshActor* ZeroHighlight;

  UFUNCTION()
  void ToggleFixSegmentSize(bool bNewSegmentPolicy);

  UFUNCTION()
  void HighlightEndingDraw(UPrimitiveComponent* HitComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

  UFUNCTION()
  void StopHighlightEndingDraw(UPrimitiveComponent* HitComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex);


  /************************************************************************/
  /* Category Edit                                                        */
  /************************************************************************/


  // Holding a reference to the component that is currently being edited
  UPROPERTY() USceneComponent* GrabbedComponent;
  // Since asking for this every time is kinda stupid if we just request that this exists
  UPROPERTY() class URootSegment* GrabbedComponentFormerRoot;
  UPROPERTY() USceneComponent* GrabbedComponentFormerAttach = nullptr;

  // this likely also uses
  UPROPERTY() bool bScaling = false;
  UPROPERTY() bool bAddToSelection = true;
  UPROPERTY() bool bCanDraw = true;

  UPROPERTY(EditAnywhere)
  bool bShouldMoveDependencies = true;

  UFUNCTION()
  void SelectionAdd(UPrimitiveComponent* HitComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

  UFUNCTION()
  void SelectionExit(UPrimitiveComponent* HitComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex);

  /************************************************************************/
  /* Category Edit New System                                             */
  /************************************************************************/

  TArray<AOverlapTransform*> Selection;
  TArray<AOverlapTransform*> Dependend;
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Menu") UParticleSystemComponent* DrawConnection;


  bool bWidgetsVisible = false;

  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Edit")
  AMenuWidget* DiaMeta;
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Edit")
  AMenuWidget* MoveMeta;
  UPROPERTY()
  float InvalidationTimer = 0.f;


  /************************************************************************/
  /* Category Menu                                                        */
  /************************************************************************/
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Menu") UParticleSystemComponent* PointIndicator;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu") UStaticMeshComponent* LiteralRightHand;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu") UStaticMeshComponent* LiteralLeftHand;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu") UStaticMeshComponent* TransformPlane;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu") UStaticMesh* HandOpen;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu") UStaticMesh* HandClosed;
  UPROPERTY() FTransform InitialTransformGrabbed;
  UPROPERTY() bool bTwoHandGrabbing = false;
  UPROPERTY() FVector IntialDistance;

  UPROPERTY() bool bMeasuringPerformance = false;
  UFUNCTION(Category = "Pawn") void TogglePerfMeasure();



  /************************************************************************/
  /* Category New Interaction System                                      */
  /************************************************************************/
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Navigation") FTransform ControllerDrag;
  UPROPERTY(VisibleAnywhere, BlueprintAssignable, Category = "Interaction") FActionDelegate ActionTaken;
  UPROPERTY(VisibleAnywhere, BlueprintAssignable, Category = "Interaction") FUndoDelegate UndoTriggered;

  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Config, Category = "Interaction")
  float ArmLength = 80.f;
  UPROPERTY()
  FVector DrawBounds;
  UPROPERTY()
  FVector DrawOrigin;
  UPROPERTY()
  FVector DrawForward;
  UPROPERTY()
  FVector DrawScale;

  FVector TouchInput;
  bool __x, __y;

  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Config, Category = "Interaction")
  float TouchDeadZone = 0.8f;
  UPROPERTY()
  FVector TouchLastInput;
  UPROPERTY()
  bool TouchInitialized = false;
  UPROPERTY()
  float TouchPadDeltaTime;
  UPROPERTY()
  int iJumpFlag = 0;
  UPROPERTY()
  bool RootSystemPanning = false;

  FCollisionObjectQueryParams Params;
  FCollisionQueryParams NewParams;

  //UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Menu") class UWidgetInteractionComponent* MenuInteractor;

private:
  UFUNCTION(Category = "Pawn") FTwoVectors GetHandRay(float Distance);
  UFUNCTION(Category = "Pawn") void HandlePhysicsAndAttachActor(AActor* HitActor);
  void NotifyActorsDependingOnInteractionMode();

  EVRSelectionMode SelectionMode{ EVRSelectionMode::sel_mode_tog };

  // SCALING INTERACTION TIMER
  FTimerDelegate ScaleTimerDel;
  FTimerHandle TimerHandle;

public:

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Draw")
  bool ScaleNotPan = false;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn") EVRNavigationModes NavigationMode = EVRNavigationModes::nav_mode_fly;
protected:
  bool bHadToRemoveMouseSettings = false;
  virtual void BeginPlay() override;
  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
  virtual void Tick(float DeltaSeconds) override;


  virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
  void ForwardForce();
  void BackwardForce();
  virtual UPawnMovementComponent* GetMovementComponent() const override;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn", meta = (AllowPrivateAccess = "true")) float BaseTurnRate = 45.0f;
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pawn", meta = (AllowPrivateAccess = "true")) UFloatingPawnMovement* Movement = nullptr;
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pawn", meta = (AllowPrivateAccess = "true")) URotatingMovementComponent* RotatingMovement = nullptr;

  // Use only when handling cross-device (PC, HMD, CAVE/ROLV) compatibility manually. HMD left  motion controller.
  UPROPERTY() UMotionControllerComponent* HmdLeftMotionController = nullptr;
  // Use only when handling cross-device (PC, HMD, CAVE/ROLV) compatibility manually. HMD right motion controller.
  UPROPERTY() UMotionControllerComponent* HmdRightMotionController = nullptr;

 // UPROPERTY()
 // UXRDeviceVisualizationComponent* HmdLeftMotionControllerVisualization = nullptr;
 //
 // UPROPERTY()
 // UXRDeviceVisualizationComponent* HmdRightMotionControllerVisualization = nullptr;


  // PC: Camera, HMD: Camera, CAVE/ROLV: Shutter glasses.
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pawn", meta = (AllowPrivateAccess = "true")) USceneComponent* Head = nullptr;
  // PC: RootComponent, HMD: HmdLeftMotionController , CAVE/ROLV: regarding to AttachRightHandInCAVE. Useful for line trace (e.g. for holding objects).
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pawn", meta = (AllowPrivateAccess = "true")) USceneComponent* RightHand = nullptr;
  // PC: RootComponent, HMD: HmdRightMotionController, CAVE/ROLV: regarding to AttachLeftHandInCAVE. Useful for line trace (e.g. for holding objects).
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pawn", meta = (AllowPrivateAccess = "true")) USceneComponent* LeftHand = nullptr;

  // Holding a reference to the actor that is currently being grabbed
  UPROPERTY() AActor* GrabbedActor;
  // indicates if the grabbed actor was simulating physics before we grabbed it
  UPROPERTY() bool bDidSimulatePhysics;
  UPROPERTY(EditAnywhere, Config) float MaxGrabDistance = 10000;
  UPROPERTY(EditAnywhere, Config) float MaxClickDistance = 10000;


  UPROPERTY() bool bFiring = false;


  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn") EVRInteractionModes InteractionMode = EVRInteractionModes::int_mode_draw;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn") bool ShowHMDControllers = true;
};


