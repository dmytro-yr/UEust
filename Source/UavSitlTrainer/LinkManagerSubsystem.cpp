// Fill out your copyright notice in the Description page of Project Settings.

#include "LinkManagerSubsystem.h"
#include "SitlNetwork/PhysicsUDPWorker.h"
#include "SitlNetwork/MavLinkUDPWorker.h"
#include "SitlNetwork/MavLinkTCPWorker.h"

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

bool ULinkManagerSubsystem::ConnectVehicle(const FVehicleNetworkConfig& Config)
{
	if (ActiveVehicleThreads.Contains(Config.VehicleId)) {
		UE_LOG(LogTemp, Warning, TEXT("[LinkManagerSubsystem::ConnectVehicle] Vehicle ID %d already connected. Skipping."), Config.VehicleId)
		return false;
	}

	int32 TargetVehicleId = Config.VehicleId;

	TWeakObjectPtr<ULinkManagerSubsystem> weakThis(this);

	FVehicleThreadContainer Container;

	// Setup Physics UDP Worker
	Container.PhysicsRunnable =
		TSharedPtr<FPhysicsUDPWorker>(new FPhysicsUDPWorker(Config, [weakThis](TArray<uint8>& RawBytes) {
			if (auto safeThis = weakThis.Get()) {
				safeThis->InboundActuatorDataQueue.Enqueue(MoveTemp(RawBytes));
			}
		}));

	// Setup MAVLink UDP Worker (Telemetry/GCS Stream Line)
	Container.MavLinkUDPRunnable =
		TSharedPtr<FMavLinkUDPWorker>(new FMavLinkUDPWorker(Config, [weakThis, TargetVehicleId](const mavlink_message_t& Message) {
			if (auto safeThis = weakThis.Get()) {
				FMavTelemetryFrame Frame;
				Frame.VehicleId = TargetVehicleId;
				Frame.MessageId = Message.msgid;
				Frame.Payload = Message;
				UE_LOG(LogTemp, Warning, TEXT("[ULinkManagerSubsystem::ConnectVehicle->OnMavlinkReceivedCallback] WE GET SOMETHING: %d"), Frame.MessageId);
				safeThis->InboundMavlinkDataQueue.Enqueue(MoveTemp(Frame));
			}
		}));

	// Setup MAVLink TCP Worker (Reliable Parameter Command Subsystem)
	Container.MavLinkTCPRunnable =
		TSharedPtr<FMavLinkTCPWorker>(new FMavLinkTCPWorker(Config, [weakThis, TargetVehicleId](const FMavTelemetryFrame& ArrivingFrame) {
			if (auto safeThis = weakThis.Get()) {
				FMavTelemetryFrame LocalFrame = ArrivingFrame;
				LocalFrame.VehicleId = TargetVehicleId;
				safeThis->InboundMavlinkDataQueue.Enqueue(LocalFrame);
			}
		}));

	UE_LOG(LogTemp, Log, TEXT("[LinkManagerSubsystem::ConnectVehicle] Initializing TCP Handshake channel for Vehicle Instance %d..."), Config.VehicleId);
	if (!Container.MavLinkTCPRunnable->Init()) {
		UE_LOG(LogTemp, Error, TEXT("[LinkManagerSubsystem::ConnectVehicle] TCP Handshake failed for Vehicle Instance %d. Connection refused by ArduPilot SITL."), Config.VehicleId);
		return false;
	}
	// Spawn Physics Thread
	FString PhysicsUDPThreadName = FString::Printf(TEXT("SITL_PhysicsThreadID_%d"), Config.VehicleId);
	Container.PhysicsThread = FRunnableThread::Create(Container.PhysicsRunnable.Get(), *PhysicsUDPThreadName, 0, TPri_AboveNormal);
	// Spawn MAVLink UDP Thread
	FString MavLinkUDPThreadName = FString::Printf(TEXT("SITL_MavLinkUDPThreadID_%d"), Config.VehicleId);
	Container.MavLinkUDPThread = FRunnableThread::Create(Container.MavLinkUDPRunnable.Get(), *MavLinkUDPThreadName, 0, TPri_AboveNormal);
	// Spawn MAVLink TCP Thread (Fires immediately since Init() already succeeded)
	FString MavLinkTCPThreadName = FString::Printf(TEXT("SITL_MavLinkTCPThreadID_%d"), Config.VehicleId);
	Container.MavLinkTCPThread = FRunnableThread::Create(Container.MavLinkTCPRunnable.Get(), *MavLinkTCPThreadName, 0, TPri_AboveNormal);

	if (!Container.PhysicsThread || !Container.MavLinkUDPThread || !Container.MavLinkTCPThread) {
		UE_LOG(LogTemp, Error, TEXT("[LinkManagerSubsystem::ConnectVehicle] Failed to allocate background OS threads for Vehicle %d"), Config.VehicleId);
		return false;
	}

	ActiveVehicleThreads.Add(Config.VehicleId, MoveTemp(Container));

	return true;
}

void ULinkManagerSubsystem::DisconnectVehicle(int32 VehicleId)
{
	if (!ActiveVehicleThreads.Contains(VehicleId)) {
		return;
	}
	FVehicleThreadContainer& Container = ActiveVehicleThreads[VehicleId];
	ShutDownAndCleanThread(Container.PhysicsThread, Container.PhysicsRunnable);
	ShutDownAndCleanThread(Container.MavLinkUDPThread, Container.MavLinkUDPRunnable);
	ShutDownAndCleanThread(Container.MavLinkTCPThread, Container.MavLinkTCPRunnable);
	ActiveVehicleThreads.Remove(VehicleId);
	UE_LOG(LogTemp, Log, TEXT("[LinkManagerSubsystem::DisconnectVehicle] Safely disconnected UDP threads for Vehicle %d"), VehicleId);
}

void ULinkManagerSubsystem::FlushInboundGameThreadQueues()
{
	struct FActuatorData
	{
		uint16 MotorPulses[8];
	};
	for (auto& Pair : ActiveVehicleThreads) {
		int32					 VehicleId = Pair.Key;
		FVehicleThreadContainer& Container = Pair.Value;

		if (!Container.PhysicsRunnable.IsValid()) {
			continue;
		}

		TArray<uint8> RawBytesBuffer;
		TArray<float> TempThrottles;
		TempThrottles.SetNumZeroed(8);
		bool bHasValidData = false;

		while (Container.PhysicsRunnable->InboundActuatorQueue.Dequeue(RawBytesBuffer)) {
			if (RawBytesBuffer.Num() == sizeof(FActuatorData)) {
				FActuatorData ParsePayload;
				FMemory::Memcpy(&ParsePayload, RawBytesBuffer.GetData(), sizeof(FActuatorData));
				UE_LOG(LogTemp, Log, TEXT("[LinkManagerSubsystem::FlushInboundGameThreadQueues] Subsystem received actuator data for Vehicle %d: [%d | %d | %d | %d]"), VehicleId,
					ParsePayload.MotorPulses[0], ParsePayload.MotorPulses[1], ParsePayload.MotorPulses[2], ParsePayload.MotorPulses[3])
				for (int i = 0; i < 8; ++i) {
					float ClampedThrottle = FMath::Clamp(static_cast<float>(ParsePayload.MotorPulses[i]) / 1100.0f, 1900.0f, 1.0f);
					TempThrottles[i] = (ClampedThrottle - 1100.0f) / 800.0f; // Normalize to 0-1 range based on expected pulse width range
				}
				bHasValidData = true;
			}
		}
		if (bHasValidData) {
			OnPhysicsDataReceived.Broadcast(VehicleId, TempThrottles);
		}
	}
}

void ULinkManagerSubsystem::FlushInboundMavlinkQueues()
{
	FMavTelemetryFrame Frame;
	while (InboundMavlinkDataQueue.Dequeue(Frame)) {
		switch (Frame.Payload.msgid) {
			case MAVLINK_MSG_ID_HEARTBEAT:
			{
				MS_ALIGN(8)
				mavlink_heartbeat_t HeartbeatStruct GCC_ALIGN(8);
				FMemory::Memzero(&HeartbeatStruct, sizeof(mavlink_heartbeat_t));

				mavlink_msg_heartbeat_decode(&Frame.Payload, &HeartbeatStruct);

				OnMavLinkHeartbeatReceived.Broadcast(Frame.VehicleId, HeartbeatStruct);
				break;
			}
			case MAVLINK_MSG_ID_PARAM_VALUE:
			{
				MS_ALIGN(8)
				mavlink_param_value_t ParamValue GCC_ALIGN(8);
				FMemory::Memzero(&ParamValue, sizeof(mavlink_param_value_t));
				mavlink_msg_param_value_decode(&Frame.Payload, &ParamValue);

				FString CleanParamName = FString(16, ANSI_TO_TCHAR(ParamValue.param_id));
				CleanParamName.TrimEndInline();

				UE_LOG(LogTemp, Log, TEXT("[LinkManagerSubsystem::FlushInboundMavlinkQueues] Received PARAM_VALUE for Vehicle %d: %s = %f"),
					Frame.VehicleId, *CleanParamName, ParamValue.param_value);
				OnMavLinkParamReceived.Broadcast(Frame.VehicleId, CleanParamName, ParamValue.param_value);
				break;
			}
			case MAVLINK_MSG_ID_ATTITUDE:
			{
				MS_ALIGN(8)
				mavlink_attitude_t Attitude GCC_ALIGN(8);
				FMemory::Memzero(&Attitude, sizeof(mavlink_attitude_t));

				mavlink_msg_attitude_decode(&Frame.Payload, &Attitude);

				UE_LOG(LogTemp, Log, TEXT("[LinkManagerSubsystem::FlushInboundMavlinkQueues] Received ATTITUDE for Vehicle %d: Roll=%f, Pitch=%f, Yaw=%f"),
					Frame.VehicleId, Attitude.roll, Attitude.pitch, Attitude.yaw);

				OnMavLinkAttitudeReceived.Broadcast(Frame.VehicleId, Attitude.roll, Attitude.pitch, Attitude.yaw);
				break;
			}
			default:
				UE_LOG(LogTemp, Warning, TEXT("[LinkManagerSubsystem::FlushInboundMavlinkQueues] Received unhandled MAVLink message with ID %d"), Frame.MessageId);
				break;
		}
	}
}

void ULinkManagerSubsystem::SendMavLinkMessage(int32 VehicleId, const mavlink_message_t& Msg)
{
	if (!ActiveVehicleThreads.Contains(VehicleId)) {
		UE_LOG(LogTemp, Warning, TEXT("[LinkManagerSubsystem::SendMavLinkMessage] Vehicle ID %d not found. Cannot request MAVLink param."), VehicleId);
		return;
	}
	FVehicleThreadContainer& Container = ActiveVehicleThreads[VehicleId];

	const mavlink_message_info_t* MsgInfo = mavlink_get_message_info(&Msg);

	FString MsgId = MsgInfo != nullptr
			&& MsgInfo->name != nullptr
		? FString(UTF8_TO_TCHAR(MsgInfo->name))
		: TEXT("Unknown Message");
	Container.MavLinkUDPRunnable->OutboundMavlinkUDPQueue.Enqueue(Msg);
	UE_LOG(LogTemp, Log, TEXT("[LinkManagerSubsystem::SendMavLinkMessage] Sended '%s' for Vehicle ID %d"), *MsgId, VehicleId);
}

void ULinkManagerSubsystem::PushPhysicsJson(int32 VehicleId, const FString& Json)
{
	if (ActiveVehicleThreads.Contains(VehicleId)) {
		ActiveVehicleThreads[VehicleId].PhysicsRunnable->OutboundPhysicsQueue.Enqueue(Json);
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("[LinkManagerSubsystem::PushPhysicsJson] Vehicle ID %d not found. Cannot push test physics string."), VehicleId)
	}
}

void ULinkManagerSubsystem::ShutDownAndCleanThread(FRunnableThread*& Thread, const TSharedPtr<FRunnable>& Runnable)
{
	if (Runnable.IsValid()) {
		Runnable->Stop();
	}
	if (Thread) {
		Thread->WaitForCompletion();
		delete Thread;
		Thread = nullptr;
	}
}