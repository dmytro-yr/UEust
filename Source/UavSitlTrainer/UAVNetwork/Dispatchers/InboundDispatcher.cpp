// Fill out your copyright notice in the Description page of Project Settings.

#include "InboundDispatcher.h"

#include "UAVNetwork/ReceiveHandlers/MavHandlers/MavActuatorsOutputsHandler.h"
#include "UAVNetwork/ReceiveHandlers/MavHandlers/MavAttitudeHandler.h"
#include "UAVNetwork/ReceiveHandlers/MavHandlers/MavCommandAckHandler.h"
#include "UAVNetwork/ReceiveHandlers/MavHandlers/MavHeartbeatHandler.h"
#include "UAVNetwork/ReceiveHandlers/MavHandlers/MavParamValueHandler.h"
#include "UAVNetwork/ReceiveHandlers/Handlers/ActuatorsOutputsHandler.h"

void UInboundDispatcher::Initialize(FNetworkChannels* InNetworkChannels, UMavLinkFactStore* InFactStore)
{
	FactStore = InFactStore;
	NetworkChannels = InNetworkChannels;

	// InboundDispatcher owns its handler set — nobody else registers here
	RegisterMavHandler(MAVLINK_MSG_ID_HEARTBEAT,
		MakeUnique<FMavHeartbeatHandler>());
	RegisterMavHandler(MAVLINK_MSG_ID_ATTITUDE,
		MakeUnique<FMavAttitudeHandler>());
	RegisterMavHandler(MAVLINK_MSG_ID_PARAM_VALUE,
		MakeUnique<FMavParamValueHandler>());
	RegisterMavHandler(MAVLINK_MSG_ID_ACTUATOR_OUTPUT_STATUS,
		MakeUnique<FMavActuatorsOutputsHandler>());
	RegisterMavHandler(MAVLINK_MSG_ID_COMMAND_ACK,
		MakeUnique<FMavCommandAckHandler>());
	// Raw UDP Handler
	RegisterRawFrameHandler(RAW_ACTUATORS_OUTPUT,
		MakeUnique<FActuatorsOutputsHandler>());
	// FCommandAckHandler injected from UVehicleLink::Initialize after OutboundDispatcher exists
}

void UInboundDispatcher::RegisterMavHandler(uint32 MsgId, TUniquePtr<FMavHandlerBase> Handler)
{
	checkf(!MavHandlerMap.Contains(MsgId),
		TEXT("[InboundDispatcher] Duplicate handler for msgid %u"), MsgId);
	MavHandlerMap.Add(MsgId, MoveTemp(Handler));
}

void UInboundDispatcher::RegisterRawFrameHandler(uint32 MsgId, TUniquePtr<FRawFrameHandlerBase> Handler)
{
	checkf(!RawFrameHandlerMap.Contains(MsgId),
		TEXT("[InboundDispatcher] Duplicate handler for msgid %u"), MsgId);
	RawFrameHandlerMap.Add(MsgId, MoveTemp(Handler));
}

void UInboundDispatcher::ReceiveFacts(uint64 FrameCount)
{
	mavlink_message_t Msg;
	while (NetworkChannels->InboundMavTCP.Dequeue(Msg)) {
		UE_LOG(LogTemp, Verbose, TEXT("[UInboundDispatcher::ReceiveFacts] InboundMavTCP processed %u"), Msg.msgid);
		if (Msg.sysid == GCS_SYSTEM_ID) {
			continue; // filter own echo
		}
		if (TUniquePtr<FMavHandlerBase>* Handler = MavHandlerMap.Find(Msg.msgid)) {
			UE_LOG(LogTemp, Verbose, TEXT("[UInboundDispatcher::ReceiveFacts] Handled msgid %u"), Msg.msgid);
			(*Handler)->Handle(Msg, *FactStore, FrameCount);
		}
#if !UE_BUILD_SHIPPING
		else {
			UE_LOG(LogTemp, Verbose, TEXT("[UInboundDispatcher::ReceiveFacts] Unhandled msgid %u"), Msg.msgid);
		}
#endif
	}

	FActuatorFrame Frame;
	while (NetworkChannels->InboundMavUDP.Dequeue(Frame)) {
		if (TUniquePtr<FRawFrameHandlerBase>* Handler = RawFrameHandlerMap.Find(RAW_ACTUATORS_OUTPUT)) {
			UE_LOG(LogTemp, Verbose, TEXT("[UInboundDispatcher::ReceiveFacts] Handled raw Frame %u"), RAW_ACTUATORS_OUTPUT);
			(*Handler)->Handle(Frame, *FactStore, FrameCount);
		}
#if !UE_BUILD_SHIPPING
		else {
			UE_LOG(LogTemp, Verbose, TEXT("[UInboundDispatcher::ReceiveFacts] Unhandled raw Frame %u"), RAW_ACTUATORS_OUTPUT);
		}
#endif
	}
	FactStore->RefreshAllStaleFacts();
}