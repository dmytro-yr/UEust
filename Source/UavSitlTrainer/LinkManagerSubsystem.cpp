// Fill out your copyright notice in the Description page of Project Settings.

#include "LinkManagerSubsystem.h"
#include "SitlNetwork/PhysicsUDPWorker.h"

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

	// Physics UDP Thread
	FVehicleThreadContainer Container;
	Container.PhysicsRunnable = new FPhysicsUDPWorker(Config);
	FString PhysicsUDPThreadName = FString::Printf(TEXT("SITL_PhysicsThreadID_%d"), Config.VehicleId);
	Container.PhysicsThread = FRunnableThread::Create(Container.PhysicsRunnable, *PhysicsUDPThreadName, 0, TPri_AboveNormal);

	// todo MavLink UDP Thread

	ActiveVehicleThreads.Add(Config.VehicleId, Container);
	UE_LOG(LogTemp, Log, TEXT("Successfully establish isolated communication loop for Vehicle Instance %d"), Config.VehicleId);
}

void ULinkManagerSubsystem::DisconnectVehicle(int32 VehicleId)
{
	if (!ActiveVehicleThreads.Contains(VehicleId)) {
		return;
	}
	FVehicleThreadContainer Container = ActiveVehicleThreads[VehicleId];
	if (Container.PhysicsThread) {
		Container.PhysicsThread->Kill(true);
		delete Container.PhysicsThread;
		delete Container.PhysicsRunnable;
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

FString ULinkManagerSubsystem::TestPopActuatorString(int32 VehicleId)
{
	if (!ActiveVehicleThreads.Contains(VehicleId)) {
		UE_LOG(LogTemp, Warning, TEXT("Vehicle ID %d not found. Cannot pop actuator string."), VehicleId)
		return FString();
	}
	TArray<uint8> ActuatorDataRawBytes;
	if (ActiveVehicleThreads[VehicleId].PhysicsRunnable->InboundActuatorQueue.Dequeue(ActuatorDataRawBytes)) {
		ActuatorDataRawBytes.Add(0);
		return FString(UTF8_TO_TCHAR((char*)ActuatorDataRawBytes.GetData()));
	}
	return FString();
}
