#pragma once
#include "SitlNetwork/MavLinkHandlers/MavHandlerBase.h"

struct FMavHeartbeatHandler : FMavHandlerBase
{
	void Handle(const mavlink_message_t& Msg, UMavLinkFactStore& FactStore, uint64 FrameCount) override
	{
		mavlink_heartbeat_t Heartbeat;
		mavlink_msg_heartbeat_decode(&Msg, &Heartbeat);

		auto	   Category = FactStore.Status;
		const bool bIsArmed = (Heartbeat.base_mode & MAV_MODE_FLAG_SAFETY_ARMED) != 0;
		Category.GetOrCreate("armed").SetValue(bIsArmed ? 1.f : 0.f, FrameCount);
		Category.GetOrCreate("system_status").SetValue(Heartbeat.system_status, FrameCount);
		Category.GetOrCreate("flight_mode").SetValue(Heartbeat.custom_mode, FrameCount);
		Category.GetOrCreate("vehicle_type").SetValue(Heartbeat.type, FrameCount);
	}
};