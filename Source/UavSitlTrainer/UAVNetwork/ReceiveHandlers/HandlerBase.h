#pragma once
#include "UAVNetwork/MavLinkIncludes.h"
#include "UAVNetwork/NetworkTypes.h"

class UMavLinkFactStore;

struct FMavHandlerBase
{
	virtual ~FMavHandlerBase() = default;
	virtual void Handle(const mavlink_message_t& Msg, UMavLinkFactStore& FactStore, uint64 FrameCount) = 0;
};

struct FRawFrameHandlerBase
{
	virtual ~FRawFrameHandlerBase() = default;
	virtual void Handle(const FActuatorFrame& ActuatorFrame, UMavLinkFactStore& FactStore, uint64 FrameCount) = 0;
};