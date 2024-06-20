// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Misc/DateTime.h"

THIRD_PARTY_INCLUDES_START
#if PLATFORM_WINDOWS
#include "StaticZMQLibrary/zmq.h"
#else
#include "zmq.h"
#include <unistd.h>
#endif
THIRD_PARTY_INCLUDES_END
#include "HAL/RunnableThread.h"
#include "HAL/Runnable.h"

#include "ZMQHandle.generated.h"

#define ZMQ_SCALAR_LEN ((1 + (sizeof(float)/sizeof(char)))*sizeof(char))
#define ZMQ_SCALAR_REQ 0b00000100
#define ZMQ_DATA_REQ 0b10000000
#define ZMQ_NUM_MASK 0b01111111
#define ZMQ_FILE_REQ 0b00000010
#define ZMQ_OK_REP 0b00000000

#define ZMQ_CMD_ALIVE       0b00000001         // send this rep normal
#define ZMQ_CMD_FILES       0b00000010
#define ZMQ_CMD_SPECI       0b00000011
#define ZMQ_CMD_ISO         0b00000100
#define ZMQ_CMD_ROOT        0b00000101
#define ZMQ_CMD_OMNI(M)   ((0b00000010&M)>>1)
#define ZMQ_CMD_CHISO       0b00001000
#define ZMQ_CMD_SAVE        0b00010000
#define ZMQ_CMD_SAVEOK      0b00010001
#define ZMQ_CMD_SAVEDAT     0b00010010
#define ZMQ_CMD_SAVETHX     0b00010100
#define ZMQ_CMD_ISORANG     0b00100000         // here rep is two float
#define ZMQ_CMD_ERR         0b11111111
#define ZMQ_CMD_RANDOM      0b00101000
#define ZMQ_CMD_TOPOLOGY    0b00111000
#define ZMQ_CMD_UNDO        0b00111001

class UZMQHandle;
class APointLight;
class UStaticMeshComponent;
class ASynchronizationHelper;
class URoot;

// test if this is visible at all

UENUM(BlueprintType)
enum class EZMQState : uint8
{
  zmq_send_cmd UMETA(DisplayName = "Requesting float param submit"),
  zmq_send_root UMETA(DisplayName = "Requesting root submit"),
  zmq_send_alive UMETA(DisplayName = "Request Server State"),
  zmq_ask_isorange UMETA(DisplayName = "Request Range of Isovalues"),
  zmq_fin UMETA(DisplayName = "Acknowledge response"),
  zmq_files UMETA(DisplayName = "Request file list"),
  zmq_sfile UMETA(DisplayName = "Request specific file"),
  zmq_idle UMETA(DisplayName = "UE Side idle"),
  zmq_random UMETA(DisplayName = "UE side perf testing"),
  zmq_topology UMETA(DisplayName = "UE needs topology redraw"),
  zmq_undo UMETA(DisplayName = "UE needs the rsml before last action"),
  zmq_kill UMETA(DisplayName = "UE shuts down")
};

UENUM(BlueprintType)
enum class EBrokerState : uint8
{
  broker_ok UMETA(DisplayName = "Server responded OK"),
  broker_data UMETA(DisplayName = "Server has sent data"),
  broker_down UMETA(DisplayName = "Server is down or sth"),
  broker_ready UMETA(DisplayName = "Broker Side idle"),
  broker_proc UMETA(DisplayName = "Broker is in operation")
};



template < int N, typename T = unsigned int >
 struct BufferContent
{
  unsigned int cmd;
  T sizes[N];
};

template < int N, typename T = unsigned int >
union BufferTransmission
{
  char dat [sizeof(BufferContent<N,T>)];
  BufferContent<N, T> parse;
};

template < typename T = unsigned int >
struct MessageResponse
{
  unsigned int cmd;
  T scalar;
};

template < typename T = unsigned int >
union MessageData
{
  char dat[4+sizeof(T)];
  MessageResponse<T> cmd;
};

template < typename T >
union TBinaryChar
{
  T value;
  char bin[sizeof(T)];
};

class BrokerThread : public FRunnable
{
  friend class UZMQHandle;
public:
  BrokerThread();
  ~BrokerThread();

  void Setup(FString Server);


  virtual uint32 Run() override;
  virtual void Stop() override;
  virtual void Exit() override;
  virtual bool Init() override;


  bool SendBytes(uint32 length, char* bytearray, bool okonly = true);

  // CONSUME VARIABLES
  EZMQState RequestState = EZMQState::zmq_idle;
  unsigned int FileIDRequest = 0;
  TArray<FVector> RootSystemJoints;
  TArray<float> Diameters;
  float IsoSurfaceParameter = 2000.f;

  // Ambiguous
  TArray<URoot*> RootSystem;

  // OUTPUT VARIABLES
  EBrokerState ResponseState = EBrokerState::broker_down;
  bool IsWaiting() const;
  bool IsComplete() const;
  bool IsReady() const;
  TArray<FVector> PointData;
  TArray<FVector> VectorData;
  TArray<float> ScalarData;
  FVector BoxRangeMin;
  FVector BoxRangeMax;
  TArray<int> TriangleData;
  FString List;
  FDateTime Heart;
  bool ensureonce = false;
  bool bKeepRootSystem = false;

private:
  FString ServerConnectionName;
  void* context = nullptr;
  void* requester = nullptr;
  int rc = 0;
  unsigned int datalen = 0;
  bool ready = false;
  char buffer[20];
  char scalarbuffer[ZMQ_SCALAR_LEN];
  MessageData<> cmdbuf{0,0};

  void HandleReceiveIsosurface(bool bUpdate = false);
  void CheckForRootSystem(EZMQState reason = EZMQState::zmq_sfile);
  void SendRootSystem();
  void CheckForRandomRoot();
  void ExchangeRoot();
  FRunnableThread* Broker = nullptr;

  EZMQState state;
  bool bStopThread = false;
};


// TODO DEMOTE THIS FROM SCENE COMPONENT TO THE LOWEST CLASS THAT SUPPORTS
// EVENT TICK FUNCTIONS
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ROOTMRICNR_API UZMQHandle : public USceneComponent
{
  GENERATED_BODY()

public:
  // Sets default values for this component's properties
  UZMQHandle();
  //UZMQHandle(const FObjectInitializer& ObjectInitializer);
  FThreadSafeCounter StateCounter;

  UPROPERTY()
    int Port;
  UPROPERTY()
    FString ServerName;

  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Network")
    void StateChanged();

    void PreInitPreparation();

  FString PrintStates();

  UFUNCTION(BlueprintCallable, Category = "Network")
    void SendRootSystem(const TArray<URoot*>& Data);

  UFUNCTION(BlueprintCallable, Category = "Network")
    void TryContactingServer();

  UFUNCTION(BlueprintCallable, Category = "Network")
    void FileRequest();

  UFUNCTION(BlueprintCallable, Category = "Network")
    void FetchNewIsosurface(float param);

  UFUNCTION(BlueprintCallable, Category = "Network")
    void RangeOfIsosurfaces();

  UFUNCTION(BlueprintCallable, Category = "Network")
    void SelectedRequest(int FileID);

  UFUNCTION(BlueprintCallable, Category = "Actor")
    void AssignStateActor(ASynchronizationHelper* helper_actor);

  UFUNCTION(BlueprintCallable, Category = "Network")
    void StopZMQThread();

  UFUNCTION(BlueprintCallable, Category = "Network")
    void UndoAction();

  UFUNCTION(BlueprintCallable, Category = "Testing")
    void RequestRandomRoot(int n);

  UFUNCTION(BlueprintCallable, Category = "Network")
    void RequestTopologyChange(const TArray<URoot*>& Data, int taproot);



  bool GetRootSystemLock();
  void ToggleRootSystemLock();

private:
  //BrokerThread* Broker;

  TSharedPtr<BrokerThread> Broker;
  TQueue<EZMQState> StateQueue;
  ASynchronizationHelper* HelperActor;
  int counter = 0;
protected:
  // Called when the game starts
  virtual void BeginPlay() override;
  void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
  // Called every frame
  virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
