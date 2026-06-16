#pragma once
#include "SitlNetwork/MavLinkIncludes.h"

class UMavLinkFactStore;

struct FMavHandlerBase
{
	virtual ~FMavHandlerBase() = default;
	virtual void Handle(const mavlink_message_t& Msg, UMavLinkFactStore& FactStore, uint64 FrameCount) = 0;
};
