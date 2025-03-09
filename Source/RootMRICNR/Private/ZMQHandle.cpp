// Fill out your copyright notice in the Description page of Project Settings.


#include "ZMQHandle.h"
#include "Engine/Light.h"
#include "Engine/PointLight.h"
#include "SynchronizationHelper.h"
#include "Components/StaticMeshComponent.h"
#include "Math/NumericLimits.h"

THIRD_PARTY_INCLUDES_START
#include <sstream>
THIRD_PARTY_INCLUDES_END
#include <Containers/StringConv.h>
#include "GenericPlatform/GenericPlatformMath.h"
#include "Root.h"

// Sets default values for this component's properties
// UZMQHandle::UZMQHandle(const FObjectInitializer& ObjectInitializer)
//   : Super(ObjectInitializer)
UZMQHandle::UZMQHandle()
  : Super()
{
  // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
  // off to improve performance if you don't need them.
  PrimaryComponentTick.bCanEverTick = true;
  this->SetComponentTickEnabled(true);
}

// Called when the game starts
void UZMQHandle::BeginPlay()
{
  Super::BeginPlay();
  if (GEngine && false)
  {
    GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::White, TEXT("Creating Broker Thread"));
  }
  UE_LOG(LogTemp, Warning, TEXT("Connecting to %s"), *ServerName);
  Broker = MakeShared<BrokerThread>();
  Broker->Setup(ServerName);
 // Broker->Setup(TEXT("tcp://") + ServerName + TEXT(":") + FString::FromInt(Port));
  //Broker = new BrokerThread();
  //Broker = NewObject<BrokerThread>(this,TEXT("ZMQ Thread Broker"));
  if (Broker->ResponseState == EBrokerState::broker_ready)
  {
    if (GEngine && false)
    {
      GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, TEXT("ZMQ Started!"));
    }
  }
  //Register(TEXT("TESTPACK"),TEXT("SYNCHONIZATIONHELPER"));
}


void UZMQHandle::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  Broker->RequestState = EZMQState::zmq_kill;
  //Broker->Exit();
  
  //delete Broker;
  Super::EndPlay(EndPlayReason);
}

// Called every frame
void UZMQHandle::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
  if (Broker->RequestState == EZMQState::zmq_fin && Broker->ResponseState == EBrokerState::broker_ready)
  {
    Broker->RequestState = EZMQState::zmq_idle;
    return;
  }
  if (Broker->ResponseState == EBrokerState::broker_data)
  {
    if (Broker->RequestState == EZMQState::zmq_files)
    {

      if (GEngine && false)
      {
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, TEXT("Processing file names"));
      }
      TArray<FString> FileNames;
      Broker->List.ParseIntoArray(FileNames, TEXT("/"), true);
      FileNames.Sort();
      unsigned int i = 0;
      HelperActor->ShowFiles(FileNames);
      Broker->RequestState = EZMQState::zmq_fin;
      return;
    }
    else if (Broker->RequestState == EZMQState::zmq_sfile)
    {
      if (GEngine && false)
      {
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, TEXT("Processing isosurface"));
      }
      HelperActor->ShowIsosurface(Broker->PointData, Broker->VectorData, Broker->TriangleData);
      HelperActor->RelayBounds(Broker->BoxRangeMax - Broker->BoxRangeMin);
      if (GEngine && false)
      {
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, TEXT("Processing roots"));
      }
      if(!Broker->bKeepRootSystem)
        HelperActor->ShowRoots(Broker->RootSystem);
      Broker->RequestState = EZMQState::zmq_fin;
    }
    else if (Broker->RequestState == EZMQState::zmq_random)
    {
      HelperActor->ShowRoots(Broker->RootSystem);
      Broker->RequestState = EZMQState::zmq_fin;
    }
    else if (Broker->RequestState == EZMQState::zmq_topology || Broker->RequestState == EZMQState::zmq_undo)
    {
      HelperActor->ShowRoots(Broker->RootSystem);
      Broker->RequestState = EZMQState::zmq_fin;
    }
    else if (Broker->RequestState == EZMQState::zmq_ask_isorange)
    {
      if (GEngine && false)
      {
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, TEXT("Received Isosurface Values!"));
      }
      HelperActor->UpdateRange(Broker->ScalarData[0], Broker->ScalarData[1]);
      Broker->RequestState = EZMQState::zmq_fin;
    }
    else if (Broker->RequestState == EZMQState::zmq_send_cmd)
    {
      if (GEngine && false)
      {
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, TEXT("Processing isosurface"));
      }
      HelperActor->ShowIsosurface(Broker->PointData, Broker->VectorData, Broker->TriangleData);
      Broker->RequestState = EZMQState::zmq_fin;
    }
  }
  else if (Broker->ResponseState == EBrokerState::broker_ok)
  {
    switch (Broker->RequestState)
    {
      case EZMQState::zmq_send_alive:
        if (GEngine && false)
        {
          GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Blue, TEXT("Alive request -> ready"));
        }
        HelperActor->ShowState();
        break;
      case EZMQState::zmq_send_root:
        if (GEngine && false)
        {
          GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, TEXT("root send -> ready"));
        }
        UE_LOG(LogTemp, Display, TEXT("root send -> ready"));
        break;
      default:
        break;
    }
    Broker->RequestState = EZMQState::zmq_fin;
  }
  else
  {

  }
}


void UZMQHandle::StateChanged_Implementation()
{

}

void UZMQHandle::PreInitPreparation()
{
  if (GEngine && false)
  {
    GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::White, TEXT("Creating Broker Thread"));
  }
  //Broker = new BrokerThread();
  Broker = MakeShared<BrokerThread>();
  if (Broker->ResponseState == EBrokerState::broker_ready)
  {
    if (GEngine && false)
    {
      GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, TEXT("ZMQ Started!"));
    }
  }
}

FString UZMQHandle::PrintStates()
{
  const UEnum* EnumPtr1 = FindObject<UEnum>(ANY_PACKAGE, TEXT("EZMQState"), true);
  const UEnum* EnumPtr2 = FindObject<UEnum>(ANY_PACKAGE, TEXT("EBrokerState"), true);
  
  if(EnumPtr1&&EnumPtr2)
    return EnumPtr1->GetDisplayNameTextByIndex((int32)Broker->RequestState).ToString()
      + EnumPtr2->GetDisplayNameTextByIndex((int32)Broker->ResponseState).ToString();
  else
    return FString();
}

void UZMQHandle::SendRootSystem(const TArray<URoot*>& Data)
{
  if (Broker->ResponseState != EBrokerState::broker_ready)
  {
    return;
  }
  Broker->ResponseState = EBrokerState::broker_proc;
  //Broker->RootSystem = Data;
  Broker->RootSystem.Empty();
  Broker->RootSystem.Append(Data);
  Broker->RequestState = EZMQState::zmq_send_root;
}

void UZMQHandle::TryContactingServer()
{
  if (Broker->ResponseState != EBrokerState::broker_ready)
  {
    return;
  }
  Broker->RequestState = EZMQState::zmq_send_alive;
  Broker->ResponseState = EBrokerState::broker_proc;
}

void UZMQHandle::FileRequest()
{
  if(Broker->ResponseState != EBrokerState::broker_ready)
    return;
  Broker->ResponseState = EBrokerState::broker_proc;
  Broker->RequestState = EZMQState::zmq_files;
}

void UZMQHandle::FetchNewIsosurface(float param)
{
  if (Broker->ResponseState != EBrokerState::broker_ready)
    return;
  Broker->IsoSurfaceParameter = param;
  Broker->ResponseState = EBrokerState::broker_proc;
  Broker->RequestState = EZMQState::zmq_send_cmd;
}

void UZMQHandle::RangeOfIsosurfaces()
{
  if (Broker->ResponseState != EBrokerState::broker_ready)
    return;
  Broker->ResponseState = EBrokerState::broker_proc;
  Broker->RequestState = EZMQState::zmq_ask_isorange;
}

void UZMQHandle::SelectedRequest(int FileID)
{
  Broker->FileIDRequest = (unsigned int)FileID;
  Broker->ResponseState = EBrokerState::broker_proc;
  Broker->RequestState = EZMQState::zmq_sfile;
}

void UZMQHandle::AssignStateActor(ASynchronizationHelper* helper_actor)
{
  HelperActor = helper_actor;
  //HelperActor->ServerConnect = this;
}

void UZMQHandle::StopZMQThread()
{
  Broker->RequestState = EZMQState::zmq_kill;
}

void UZMQHandle::UndoAction()
{
  Broker->ResponseState = EBrokerState::broker_proc;
  Broker->RequestState = EZMQState::zmq_undo;
}

void UZMQHandle::RequestRandomRoot(int n)
{
  Broker->FileIDRequest = (unsigned int) (FGenericPlatformMath::Max(2,n));
  Broker->ResponseState = EBrokerState::broker_proc;
  Broker->RequestState = EZMQState::zmq_random;
}

void UZMQHandle::RequestTopologyChange(const TArray<URoot*>& Data, int taproot)
{
  Broker->RootSystem.Empty();
  Broker->RootSystem.Append(Data);
  Broker->FileIDRequest = (unsigned int)(taproot);
  Broker->ResponseState = EBrokerState::broker_proc;
  Broker->RequestState = EZMQState::zmq_topology;
}

bool UZMQHandle::GetRootSystemLock()
{
    return Broker->bKeepRootSystem;
}

void UZMQHandle::ToggleRootSystemLock()
{
  Broker->bKeepRootSystem = !Broker->bKeepRootSystem;
}

BrokerThread::BrokerThread()
{
}

BrokerThread::~BrokerThread()
{
  if (Broker)
  {
    RequestState = EZMQState::zmq_kill;
    //Broker->Kill();
    delete Broker;
  }
  Broker = nullptr;
}

void BrokerThread::Setup(FString Server)
{

  if (GEngine && false)
  {
    GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, Server);
  }
  ServerConnectionName = Server;
  ServerConnectionName = FString("tcp://127.0.0.1:12575");
  Broker = FRunnableThread::Create(this, TEXT("ZMQ"), 0, TPri_BelowNormal);
  UE_LOG(LogTemp, Warning, TEXT("Created Broker Class with Server Name: %s"), *ServerConnectionName);
  
  if (GEngine && false)
  {
    GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::White, TEXT("Created Broker Class"));
  }
}

uint32 BrokerThread::Run()
{
  // we don't want to accidentally trigger two switch cases at once
  // due to bad timing
  // this can be an issue if the case is set two time in rapid succession
  // and we for some reason only want the later one.
  ResponseState = EBrokerState::broker_ready;
  UE_LOG(LogTemp, Warning, TEXT("Broker Thread Running"));
  while (EZMQState::zmq_kill != RequestState)
  {
    FPlatformProcess::Sleep(0.1); // yield regularly to free resources
    EZMQState lock = RequestState;
    uint8* statecontrol = nullptr;
    float* scalarentry = nullptr;
    char cbuf[10];
    if ((ResponseState == EBrokerState::broker_ok || ResponseState == EBrokerState::broker_data)
        && lock != EZMQState::zmq_fin)
      continue;

    switch (lock)
    {
    case EZMQState::zmq_send_alive:
      if (!ensureonce)
      {
        MessageData<> tosend;
        tosend.cmd.cmd = ZMQ_CMD_ALIVE;
        tosend.cmd.scalar = 0x04030201;
        zmq_send(requester, tosend.dat, 8, 0);
        if (zmq_recv(requester, cbuf, 10, 0) >= 0 && cbuf[0] == ZMQ_CMD_ALIVE)
        {
          ResponseState = EBrokerState::broker_ok;
          ensureonce = true;
        }
      }
      break;
    case EZMQState::zmq_ask_isorange:
      MessageData<float> tosend;
      tosend.cmd.cmd = ZMQ_CMD_ISORANG;
      tosend.cmd.scalar = IsoSurfaceParameter;
      BufferTransmission<2,float> recvmessage;
      zmq_send(requester,tosend.dat,sizeof(tosend),0);
      zmq_recv(requester,recvmessage.dat,sizeof(recvmessage),0);
      ScalarData.Empty();
      ScalarData.Add(recvmessage.parse.sizes[0]);
      ScalarData.Add(recvmessage.parse.sizes[1]);
      ResponseState = EBrokerState::broker_data;
      break;
    case EZMQState::zmq_sfile:
      UE_LOG(LogTemp, Warning, TEXT("Fetching Isosurface"));
      HandleReceiveIsosurface();
      UE_LOG(LogTemp, Warning, TEXT("Fetching Roots"));
      CheckForRootSystem();
      UE_LOG(LogTemp, Warning, TEXT("Done Iso+Roots"));
      ResponseState = EBrokerState::broker_data;
      break;
    case EZMQState::zmq_random:
      CheckForRootSystem(EZMQState::zmq_random);
      ResponseState = EBrokerState::broker_data;
      break;
    case EZMQState::zmq_undo:
      CheckForRootSystem(EZMQState::zmq_undo);
      ResponseState = EBrokerState::broker_data;
      break;
    case EZMQState::zmq_topology:
      ExchangeRoot();
      ResponseState = EBrokerState::broker_data;
      break;
    case EZMQState::zmq_files:
      MessageData<> filerequest;
      filerequest.cmd.cmd = ZMQ_CMD_FILES;
      filerequest.cmd.scalar = 0;
      zmq_send(requester, filerequest.dat, 8, 0);
      // we write the response in the same thing
      zmq_recv(requester, filerequest.dat, 8, 0);
      char* FileList;
      // is this forced ensurance maybe causing the leak?
      datalen = filerequest.cmd.scalar + 1;
      FileList = new char[datalen];
      FileList[datalen - 1] = '\0';
      filerequest.cmd.cmd = ZMQ_CMD_ALIVE;
      filerequest.cmd.scalar = 0;
      if (FileList != nullptr)
      {
        zmq_send(requester, filerequest.dat, 8, 0);
        zmq_recv(requester, FileList, datalen, 0);
        List = FString(ANSI_TO_TCHAR(FileList));
        ResponseState = EBrokerState::broker_data;
      }
      delete[] FileList;
      break;
    case EZMQState::zmq_fin:
      ResponseState = EBrokerState::broker_ready;
//       PointData.Empty();
//       ScalarData.Empty();
//       VectorData.Empty();
      break;
    case EZMQState::zmq_send_root:
      SendRootSystem();
      break;
    case EZMQState::zmq_send_cmd:
      HandleReceiveIsosurface(true);
      break;
    case EZMQState::zmq_idle:
    default:
      break;

    }
  }
  return 0;
}

void BrokerThread::Stop()
{
  UE_LOG(LogTemp, Warning, TEXT("Stopping Broker Thread"));
  zmq_disconnect(requester, "tcp://localhost:12575");
  if (requester)
  {
    zmq_close(requester);
    requester = nullptr;
  }
  if (context)
  {
    zmq_ctx_destroy(context);
    context = nullptr;
  }
}

void BrokerThread::Exit()
{
//   requester = nullptr;
//   context = nullptr;
}

bool BrokerThread::IsWaiting() const
{
  return true;
}

bool BrokerThread::IsComplete() const
{

  return true;
}

bool BrokerThread::IsReady() const
{

  return true;
}

void BrokerThread::HandleReceiveIsosurface(bool bUpdate)
{
  BufferTransmission<3> RootSystemWarning;
  BufferTransmission<1> FollowUpRequest;
  BoxRangeMax = FVector(TNumericLimits<float>::Min(),
    TNumericLimits<float>::Min(),
    TNumericLimits<float>::Min());
  BoxRangeMin = FVector(TNumericLimits<float>::Min(),
    TNumericLimits<float>::Min(),
    TNumericLimits<float>::Min());
  PointData.Empty();
  TriangleData.Empty();
  VectorData.Empty();
  if (bUpdate)
  {
    BufferTransmission<1, float> RootSystemRequest;
    RootSystemRequest.parse.cmd = ZMQ_CMD_CHISO;
    RootSystemRequest.parse.sizes[0] = IsoSurfaceParameter;
    zmq_send(requester, RootSystemRequest.dat, sizeof(RootSystemRequest.dat), 0);
  }
  else
  {
    BufferTransmission<1> RootSystemRequest;
    RootSystemRequest.parse.cmd = ZMQ_CMD_SPECI;
    RootSystemRequest.parse.sizes[0] = FileIDRequest;
    zmq_send(requester, RootSystemRequest.dat, sizeof(RootSystemRequest.dat), 0);
  }
  if (zmq_recv(requester, RootSystemWarning.dat, sizeof(RootSystemWarning.dat), 0) < 0 || ZMQ_CMD_OMNI(RootSystemWarning.parse.cmd))
  {
     ResponseState = EBrokerState::broker_data;
    return;
  }
  char* transmission_data;
  unsigned long warning_size = 4 + FGenericPlatformMath::Max(
    FGenericPlatformMath::Max(
      RootSystemWarning.parse.sizes[0],
      RootSystemWarning.parse.sizes[1]),
    RootSystemWarning.parse.sizes[2]
  );
  transmission_data = new char[4 * warning_size];
  if (!transmission_data)
  {
    ResponseState = EBrokerState::broker_down;
    return;
  }
  FollowUpRequest.parse.cmd = ZMQ_CMD_ALIVE;
  zmq_send(requester, FollowUpRequest.dat, sizeof(FollowUpRequest.dat), 0);
  zmq_recv(requester, transmission_data,
    RootSystemWarning.parse.sizes[0] * 4, 0);
  float* VectorPointer = reinterpret_cast<float*>(transmission_data);
  for (unsigned int i = 0; i < RootSystemWarning.parse.sizes[0]; i += 3)
  {
    FVector point = FVector(VectorPointer[i], VectorPointer[i + 1], VectorPointer[i + 2]);
    BoxRangeMax.X = FGenericPlatformMath::Max(point.X, BoxRangeMax.X);
    BoxRangeMax.Y = FGenericPlatformMath::Max(point.Y, BoxRangeMax.Y);
    BoxRangeMax.Z = FGenericPlatformMath::Max(point.Z, BoxRangeMax.Z);
    BoxRangeMin.X = FGenericPlatformMath::Min(point.X, BoxRangeMin.X);
    BoxRangeMin.Y = FGenericPlatformMath::Min(point.Y, BoxRangeMin.Y);
    BoxRangeMin.Z = FGenericPlatformMath::Min(point.Z, BoxRangeMin.Z);
    PointData.Add(point);
  }
  zmq_send(requester, FollowUpRequest.dat, sizeof(FollowUpRequest.dat), 0);
  zmq_recv(requester, transmission_data,
    RootSystemWarning.parse.sizes[1] * 4, 0);
  unsigned int* TriPointer = reinterpret_cast<unsigned int*>(transmission_data);
  for (unsigned int i = 0; i < RootSystemWarning.parse.sizes[1]; ++i)
  {
    TriangleData.Add(TriPointer[i]);
  }
  zmq_send(requester, FollowUpRequest.dat, sizeof(FollowUpRequest.dat), 0);
  zmq_recv(requester, transmission_data,
    RootSystemWarning.parse.sizes[2] * 4, 0);
  VectorPointer = reinterpret_cast<float*>(transmission_data);
  zmq_send(requester, FollowUpRequest.dat, sizeof(FollowUpRequest.dat), 0);
  zmq_recv(requester,RootSystemWarning.dat,sizeof(RootSystemWarning.dat),0);
  for (unsigned int i = 0; i < RootSystemWarning.parse.sizes[2]; i += 3)
  {
    VectorData.Add(FVector(VectorPointer[i], VectorPointer[i + 1], VectorPointer[i + 2]));
  }
  delete[] transmission_data;
  if(bUpdate)
    ResponseState = EBrokerState::broker_data;
}

void BrokerThread::CheckForRootSystem(EZMQState reason)
{
  if (bKeepRootSystem)
  {
    return;
  }
  RootSystem.Empty();
  BufferTransmission<1> RootSystemRequest;
  BufferTransmission<2> RootSystemWarning;
  switch(reason)
  {
  case EZMQState::zmq_random:
    RootSystemRequest.parse.cmd = ZMQ_CMD_RANDOM;
    RootSystemRequest.parse.sizes[0] = FileIDRequest;
    break;
  case EZMQState::zmq_topology:
    RootSystemRequest.parse.cmd = ZMQ_CMD_TOPOLOGY;
    RootSystemRequest.parse.sizes[0] = FileIDRequest;
    break;
  case EZMQState::zmq_undo:
    RootSystemRequest.parse.cmd = ZMQ_CMD_UNDO;
    RootSystemRequest.parse.sizes[0] = FileIDRequest;
    break;
  case EZMQState::zmq_sfile:
  default:
    RootSystemRequest.parse.cmd = ZMQ_CMD_ROOT;
    RootSystemRequest.parse.sizes[0] = 0;
  }
  zmq_send(requester,RootSystemRequest.dat,sizeof(RootSystemRequest),0);
  zmq_recv(requester,RootSystemWarning.dat,sizeof(RootSystemWarning),0);
  unsigned int buffersize = 4* RootSystemWarning.parse.sizes[1] + 8;
  char* databuffer = new char[buffersize];
  if (!databuffer || ZMQ_CMD_OMNI(RootSystemWarning.parse.cmd))
  {
    // TODO error handling, contact server?
    ResponseState = EBrokerState::broker_ok;
    return;
  }
  else
  {
    TBinaryChar<float> FloatBinaryChar;
    TBinaryChar<unsigned int> UIntBinaryChar;
    TBinaryChar<int> IntBinaryChar;
    
    RootSystemRequest.parse.cmd = ZMQ_CMD_ALIVE;

    auto getval = [databuffer](unsigned int i, char* t){
      t[0] = databuffer[i + 0] & 0x000000FF;
      t[1] = databuffer[i + 1] & 0x000000FF;
      t[2] = databuffer[i + 2] & 0x000000FF;
      t[3] = databuffer[i + 3] & 0x000000FF;
    };
    unsigned int numroots = RootSystemWarning.parse.sizes[0];
    for(unsigned int rn = 0; rn < numroots; ++rn)
    {
      zmq_send(requester, RootSystemRequest.dat, sizeof(RootSystemRequest), 0);
      zmq_recv(requester, databuffer, buffersize, 0);
      URoot* cr = NewObject<URoot>(GetTransientPackage(),
        MakeUniqueObjectName(GetTransientPackage(), URoot::StaticClass()), EObjectFlags::RF_Standalone);
      // root number
      getval(0, UIntBinaryChar.bin);
      cr->RootNumber = UIntBinaryChar.value;

      // previous root
      getval(4, IntBinaryChar.bin);
      cr->Predecessor = IntBinaryChar.value;

      getval(8,UIntBinaryChar.bin);
      unsigned int numPoints = UIntBinaryChar.value;

      getval(12,IntBinaryChar.bin);
      cr->StartJoint = IntBinaryChar.value;

      unsigned int i = 16;
//       float* pointpointer = reinterpret_cast<float*>(databuffer[i]);
//       float* diapointer = reinterpret_cast<float*>(databuffer[12 + numPoints * 3]);
//       // point list parsing
//       unsigned int u = 0;
//       for (int p = 0; p < numPoints; ++p)
//       {
//         FVector v;
//         v.
//       }
      while (i < 16 + numPoints*3*4)
      {
        FVector v;
        getval(i,FloatBinaryChar.bin);
        v.X = FloatBinaryChar.value;
        i += 4;
        getval(i, FloatBinaryChar.bin);
        v.Y = FloatBinaryChar.value;
        i += 4;
        getval(i, FloatBinaryChar.bin);
        v.Z = FloatBinaryChar.value;
        i += 4;
        cr->LCJointPositions.Add(v);
      }

      // diameter parsing
      unsigned int end = i + 4*numPoints;
      while (i < 16+(numPoints*4*4))
      {
        getval(i, FloatBinaryChar.bin);

        cr->Diameters.Add(FloatBinaryChar.value);
        //cr->Diameters.Add(0.15f);
        i += 4;
      }
      RootSystem.Add(cr);
    }
  }
  delete [] databuffer;
}

void BrokerThread::SendRootSystem()
{
  BufferTransmission<1> message;
  BufferTransmission<1> answer;
  message.parse.cmd = ZMQ_CMD_SAVE;
  message.parse.sizes[0] = (unsigned int) RootSystem.Num();
  zmq_send(requester,message.dat,sizeof(message),0);
  zmq_recv(requester,answer.dat,sizeof(answer),0);
  unsigned int maxsize = 0;
  for (URoot* root : RootSystem)
    maxsize = FGenericPlatformMath::Max(maxsize,(unsigned int)root->LCJointPositions.Num());
  char* boof = new char[16 * maxsize + 16];
  int* boofi = reinterpret_cast<int*>(boof);
  float* booff = reinterpret_cast<float*>(boof);
  if (answer.parse.cmd == ZMQ_CMD_ALIVE)
  {
    for (URoot* root : RootSystem)
    {
      unsigned int size = 16 + 4*4*root->LCJointPositions.Num();
      boofi[0] = root->RootNumber;
      boofi[1] = root->Predecessor;
      boofi[2] = root->LCJointPositions.Num();
      boofi[3] = root->StartJoint;
      for(int s = 0; s < root->LCJointPositions.Num(); ++s)
      {
        booff[3*s + 4] = root->LCJointPositions[s].X;
        booff[3*s + 4 + 1] = root->LCJointPositions[s].Y;
        booff[3*s + 4 + 2] = root->LCJointPositions[s].Z;
        booff[s + 4 + 3* root->LCJointPositions.Num()] = root->Diameters[s];
      }
      zmq_send(requester,boof,size,0);
      zmq_recv(requester,answer.dat,sizeof(answer),0);
      if (answer.parse.cmd != ZMQ_CMD_ALIVE)
      {
        break;
      }
    }
  }
  delete[] boof;
  ResponseState = EBrokerState::broker_ok;
}

void BrokerThread::CheckForRandomRoot()
{
  RootSystem.Empty();
  BufferTransmission<1> RootSystemRequest;
  BufferTransmission<2> RootSystemWarning;
  RootSystemRequest.parse.cmd = ZMQ_CMD_RANDOM;
  RootSystemRequest.parse.sizes[0] = FileIDRequest;
  zmq_send(requester, RootSystemRequest.dat, sizeof(RootSystemRequest), 0);
  zmq_recv(requester, RootSystemWarning.dat, sizeof(RootSystemWarning), 0);
  unsigned int buffersize = 4 * RootSystemWarning.parse.sizes[1] + 8;
  if (ZMQ_CMD_OMNI(RootSystemWarning.parse.cmd))
  {
    return;
  }
  else
  {
    char* databuffer = new char[buffersize];
    if (!databuffer)
    {
      // TODO error handling, contact server?
      return;
    }
    TBinaryChar<float> FloatBinaryChar;
    TBinaryChar<unsigned int> UIntBinaryChar;
    TBinaryChar<int> IntBinaryChar;

    RootSystemRequest.parse.cmd = ZMQ_CMD_ALIVE;

    auto getval = [databuffer](unsigned int i, char* t) {
      t[0] = databuffer[i + 0] & 0x000000FF;
      t[1] = databuffer[i + 1] & 0x000000FF;
      t[2] = databuffer[i + 2] & 0x000000FF;
      t[3] = databuffer[i + 3] & 0x000000FF;
    };
    unsigned int numroots = RootSystemWarning.parse.sizes[0];
    for (unsigned int rn = 0; rn < numroots; ++rn)
    {
      zmq_send(requester, RootSystemRequest.dat, sizeof(RootSystemRequest), 0);
      zmq_recv(requester, databuffer, buffersize, 0);
      URoot* cr = NewObject<URoot>(GetTransientPackage(),
        MakeUniqueObjectName(GetTransientPackage(), URoot::StaticClass()), EObjectFlags::RF_Standalone);
      // root number
      getval(0, UIntBinaryChar.bin);
      cr->RootNumber = UIntBinaryChar.value;

      // previous root
      getval(4, IntBinaryChar.bin);
      cr->Predecessor = IntBinaryChar.value;

      getval(8, UIntBinaryChar.bin);
      unsigned int numPoints = UIntBinaryChar.value;

      getval(12, IntBinaryChar.bin);
      cr->StartJoint = IntBinaryChar.value;

      unsigned int i = 16;
      while (i < 16 + numPoints * 3 * 4)
      {
        FVector v;
        getval(i, FloatBinaryChar.bin);
        v.X = FloatBinaryChar.value;
        i += 4;
        getval(i, FloatBinaryChar.bin);
        v.Y = FloatBinaryChar.value;
        i += 4;
        getval(i, FloatBinaryChar.bin);
        v.Z = FloatBinaryChar.value;
        i += 4;
        cr->LCJointPositions.Add(v);
      }

      // diameter parsing
      unsigned int end = i + 4 * numPoints;
      while (i < 16 + (numPoints * 4 * 4))
      {
        getval(i, FloatBinaryChar.bin);

        cr->Diameters.Add(FloatBinaryChar.value);
        //cr->Diameters.Add(0.15f);
        i += 4;
      }
      RootSystem.Add(cr);
    }
    delete[] databuffer;
  }
}

void BrokerThread::ExchangeRoot()
{
  BufferTransmission<1> message;
  BufferTransmission<1> answer;
  message.parse.cmd = ZMQ_CMD_TOPOLOGY;
  message.parse.sizes[0] = (unsigned int)RootSystem.Num();
  zmq_send(requester, message.dat, sizeof(message), 0);
  zmq_recv(requester, answer.dat, sizeof(answer), 0);
  unsigned int maxsize = 0;
  for (URoot* root : RootSystem)
    maxsize = FGenericPlatformMath::Max(maxsize, (unsigned int)root->LCJointPositions.Num());
  char* boof = new char[16 * maxsize + 16];
  int* boofi = reinterpret_cast<int*>(boof);
  float* booff = reinterpret_cast<float*>(boof);
  if (answer.parse.cmd == ZMQ_CMD_ALIVE)
  {
    for (URoot* root : RootSystem)
    {
      unsigned int size = 16 + 4 * 4 * root->LCJointPositions.Num();
      boofi[0] = root->RootNumber;
      boofi[1] = root->Predecessor;
      boofi[2] = root->LCJointPositions.Num();
      boofi[3] = root->StartJoint;
      for (int s = 0; s < root->LCJointPositions.Num(); ++s)
      {
        booff[3 * s + 4] = root->LCJointPositions[s].X;
        booff[3 * s + 4 + 1] = root->LCJointPositions[s].Y;
        booff[3 * s + 4 + 2] = root->LCJointPositions[s].Z;
        booff[s + 4 + 3 * root->LCJointPositions.Num()] = root->Diameters[s];
      }
      zmq_send(requester, boof, size, 0);
      zmq_recv(requester, answer.dat, sizeof(answer), 0);
      if (answer.parse.cmd != ZMQ_CMD_ALIVE)
      {
        break;
      }
    }
  }

  delete[] boof;

  message.parse.cmd = ZMQ_CMD_TOPOLOGY;
  message.parse.sizes[0] = FileIDRequest;


  RootSystem.Empty();
  BufferTransmission<2> RootSystemWarning;

  zmq_send(requester, message.dat, sizeof(message), 0);
  zmq_recv(requester, RootSystemWarning.dat, sizeof(RootSystemWarning), 0);
  unsigned int buffersize = 4 * RootSystemWarning.parse.sizes[1] + 8;
  boof = new char[buffersize];
  if (!boof || ZMQ_CMD_OMNI(RootSystemWarning.parse.cmd))
  {
    // TODO error handling, contact server?
    return;
  }
  else
  {
    TBinaryChar<float> FloatBinaryChar;
    TBinaryChar<unsigned int> UIntBinaryChar;
    TBinaryChar<int> IntBinaryChar;

    message.parse.cmd = ZMQ_CMD_ALIVE;

    auto getval = [boof](unsigned int i, char* t) {
      t[0] = boof[i + 0] & 0x000000FF;
      t[1] = boof[i + 1] & 0x000000FF;
      t[2] = boof[i + 2] & 0x000000FF;
      t[3] = boof[i + 3] & 0x000000FF;
    };
    unsigned int numroots = RootSystemWarning.parse.sizes[0];
    for (unsigned int rn = 0; rn < numroots; ++rn)
    {
      zmq_send(requester, message.dat, sizeof(message), 0);
      zmq_recv(requester, boof, buffersize, 0);
      URoot* cr = NewObject<URoot>(GetTransientPackage(),
        MakeUniqueObjectName(GetTransientPackage(), URoot::StaticClass()), EObjectFlags::RF_Standalone);
      // root number
      getval(0, UIntBinaryChar.bin);
      cr->RootNumber = UIntBinaryChar.value;

      // previous root
      getval(4, IntBinaryChar.bin);
      cr->Predecessor = IntBinaryChar.value;

      getval(8, UIntBinaryChar.bin);
      unsigned int numPoints = UIntBinaryChar.value;

      getval(12, IntBinaryChar.bin);
      cr->StartJoint = IntBinaryChar.value;

      unsigned int i = 16;
      while (i < 16 + numPoints * 3 * 4)
      {
        FVector v;
        getval(i, FloatBinaryChar.bin);
        v.X = FloatBinaryChar.value;
        i += 4;
        getval(i, FloatBinaryChar.bin);
        v.Y = FloatBinaryChar.value;
        i += 4;
        getval(i, FloatBinaryChar.bin);
        v.Z = FloatBinaryChar.value;
        i += 4;
        cr->LCJointPositions.Add(v);
      }

      // diameter parsing
      unsigned int end = i + 4 * numPoints;
      while (i < 16 + (numPoints * 4 * 4))
      {
        getval(i, FloatBinaryChar.bin);

        cr->Diameters.Add(FloatBinaryChar.value);
        //cr->Diameters.Add(0.15f);
        i += 4;
      }
      RootSystem.Add(cr);
    }
  }

  delete[] boof;
}

bool BrokerThread::Init()
{
  UE_LOG(LogTemp,Warning,TEXT("%s"),*ServerConnectionName);
  context = zmq_ctx_new();
  requester = zmq_socket(context, ZMQ_REQ);
#if defined UE_BUILD_DEBUG && false
  int timeout = 10000;
  zmq_setsockopt(requester, ZMQ_RCVTIMEO,&timeout,sizeof(int));
#endif

  //rc = zmq_connect(requester, TCHAR_TO_ANSI(*ServerConnectionName));
  rc = zmq_connect(requester, "tcp://127.0.0.1:12575");
  if (rc != 0)
  {
    zmq_disconnect(requester, "tcp://localhost:12575");
    ResponseState = EBrokerState::broker_down;
  }
  return rc == 0;
}

bool BrokerThread::SendBytes(uint32 length, char* bytearray, bool okonly /*= true*/)
{
  return false;
}
