// Fill out your copyright notice in the Description page of Project Settings.

#include "MavLinkDispatcher.h"

#include "MavLinkHandlers/Handlers/MavActuatorOutputsHandler.h"
#include "MavLinkHandlers/Handlers/MavAttitudeHandler.h"
#include "MavLinkHandlers/Handlers/MavCommandAckHandler.h"
#include "MavLinkHandlers/Handlers/MavHeartbeatHandler.h"
#include "MavLinkHandlers/Handlers/MavParamValueHandler.h"

void UMavLinkDispatcher::Initialize(FNetworkChannels* InNetworkChannels, UMavLinkFactStore* InFactStore)
{
	FactStore = InFactStore;
	NetworkChannels = InNetworkChannels;

	// Dispatcher owns its handler set — nobody else registers here
	RegisterHandler(MAVLINK_MSG_ID_HEARTBEAT,
		MakeUnique<FMavHeartbeatHandler>());
	RegisterHandler(MAVLINK_MSG_ID_ATTITUDE,
		MakeUnique<FMavAttitudeHandler>());
	RegisterHandler(MAVLINK_MSG_ID_PARAM_VALUE,
		MakeUnique<FMavParamValueHandler>());
	RegisterHandler(MAVLINK_MSG_ID_ACTUATOR_OUTPUT_STATUS,
		MakeUnique<FMavActuatorOutputsHandler>());
	RegisterHandler(MAVLINK_MSG_ID_COMMAND_ACK,
		MakeUnique<FMavCommandAckHandler>());
	// FCommandAckHandler injected from UVehicleLink::Initialize after CommandBus exists
}

void UMavLinkDispatcher::RegisterHandler(uint32 MsgId, TUniquePtr<FMavHandlerBase> Handler)
{
	checkf(!HandlerMap.Contains(MsgId),
		TEXT("[Dispatcher] Duplicate handler for msgid %u"), MsgId);
	HandlerMap.Add(MsgId, MoveTemp(Handler));
}

void UMavLinkDispatcher::ReceiveFacts(uint64 FrameCount)
{
	mavlink_message_t Msg;
	while (NetworkChannels->InboundMavTCP.Dequeue(Msg)) {
		UE_LOG(LogTemp, Verbose, TEXT("[UMavLinkDispatcher::ReceiveFacts] InboundMavTCP processed %u"), Msg.msgid);
		if (Msg.sysid == GCS_SYSTEM_ID) {
			continue; // filter own echo
		}
		if (TUniquePtr<FMavHandlerBase>* Handler = HandlerMap.Find(Msg.msgid)) {
			UE_LOG(LogTemp, Verbose, TEXT("[UMavLinkDispatcher::ReceiveFacts] Handled msgid %u"), Msg.msgid);
			(*Handler)->Handle(Msg, *FactStore, FrameCount);
		}
#if !UE_BUILD_SHIPPING
		else {
			UE_LOG(LogTemp, Verbose, TEXT("[UMavLinkDispatcher::ReceiveFacts] Unhandled msgid %u"), Msg.msgid);
		}
#endif
	}
	FActuatorFrame Frame;

	while (NetworkChannels->InboundMavUDP.Dequeue(Frame)) {
		UE_LOG(LogTemp, Warning, TEXT("[UMavLinkDispatcher::ReceiveFacts] InboundMavUDP WE GET ACTUATOR FRAMES"));
		for (int32 i = 0; i < 8; ++i) {
			FName Key(*FString::Printf(TEXT("motor_%d"), i));
			FactStore->Actuators.GetOrCreate(Key)
				.SetValue(Frame.NormalizedThrottle(i), FrameCount);
		}
	}
	FactStore->RefreshAllStaleFacts();
}