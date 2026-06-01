// Fill out your copyright notice in the Description page of Project Settings.

#include "LinkManagerSubsystem.h"
#include "SitlNetwork/PhysicsUDPWorker.h"
#include "SitlNetwork/MavLinkUDPWorker.h"
void ULinkManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void ULinkManagerSubsystem::Deinitialize()
{
	// Ensure clean teardown on exit to prevent thread hanging in Editor
	TArray<int32> Keys;
	ActiveVehicleThreads.GetKeys(Keys);
	for (int32 Id : Keys) {
		DisconnectVehicle(Id);
	}
	Super::Deinitialize();
}

void ULinkManagerSubsystem::ConnectVehicle(const FVehicleNetworkConfig& Config)
{
	if (ActiveVehicleThreads.Contains(Config.VehicleId)) {
		UE_LOG(LogTemp, Warning, TEXT("Vehicle ID %d already connected. Skipping."), Config.VehicleId)
	}
	TWeakObjectPtr<ULinkManagerSubsystem> weakThis(this);
	// Physics UDP Thread
	FVehicleThreadContainer Container;
	Container.PhysicsRunnable = TSharedPtr<FPhysicsUDPWorker>(new FPhysicsUDPWorker(Config, [weakThis](TArray<uint8>& RawBytes) {
		if (auto safeThis = weakThis.Get()) {
			safeThis->InboundActuatorDataQueue.Enqueue(MoveTemp(RawBytes));
		}
	}));
	FString PhysicsUDPThreadName = FString::Printf(TEXT("SITL_PhysicsThreadID_%d"), Config.VehicleId);
	Container.PhysicsThread = FRunnableThread::Create(Container.PhysicsRunnable.Get(), *PhysicsUDPThreadName, 0, TPri_AboveNormal);

	// todo MavLink UDP Thread

	ActiveVehicleThreads.Add(Config.VehicleId, Container);
	UE_LOG(LogTemp, Log, TEXT("Successfully establish isolated communication loop for Vehicle Instance %d"), Config.VehicleId);
}

void ULinkManagerSubsystem::DisconnectVehicle(int32 VehicleId)
{
	if (!ActiveVehicleThreads.Contains(VehicleId)) {
		return;
	}
	FVehicleThreadContainer& Container = ActiveVehicleThreads[VehicleId];
	if (Container.PhysicsThread && Container.PhysicsRunnable.IsValid()) {
		Container.PhysicsRunnable->Stop();
		Container.PhysicsThread->WaitForCompletion();
		delete Container.PhysicsThread;
		Container.PhysicsThread = nullptr;
		Container.PhysicsRunnable.Reset();
	}
	ActiveVehicleThreads.Remove(VehicleId);
	UE_LOG(LogTemp, Log, TEXT("Safely disconnected UDP threads for Vehicle %d"), VehicleId);
}

void ULinkManagerSubsystem::TestPushPhysicsString(int32 VehicleId, const FString& TestJson)
{
	if (ActiveVehicleThreads.Contains(VehicleId)) {
		ActiveVehicleThreads[VehicleId].PhysicsRunnable->OutboundPhysicsQueue.Enqueue(TestJson);
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Vehicle ID %d not found. Cannot push test physics string."), VehicleId)
	}
}

void ULinkManagerSubsystem::FlushInboundGameThreadQueues()
{
	for (auto& Pair : ActiveVehicleThreads) {
		int32					 VehicleId = Pair.Key;
		FVehicleThreadContainer& Container = Pair.Value;

		TArray<uint8> RawBytesBuffer;
		int32		  RawBytesPacketThisFrame = 0;
		int32		  LastSize = 0;

		while (Container.PhysicsRunnable->InboundActuatorQueue.Dequeue(RawBytesBuffer)) {
			LastSize = RawBytesBuffer.Num();
			RawBytesPacketThisFrame++;
			UE_LOG(LogTemp, Log, TEXT("Received actuator data for Vehicle %d: %d bytes"), VehicleId, RawBytesBuffer.Num());
		}
		if (RawBytesPacketThisFrame > 0 && GEngine) {
			FString DebugMsg = FString::Printf(TEXT("Inbound Active: %d packets, Last Packet Size: %d bytes"), RawBytesPacketThisFrame, LastSize);
			GEngine->AddOnScreenDebugMessage(VehicleId, 0.1f, FColor::Green, DebugMsg);
		}
	}
}
