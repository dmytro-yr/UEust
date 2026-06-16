#pragma once
#include "SitlNetwork/MavLinkHandlers/MavHandlerBase.h"

struct FMavAttitudeHandler : FMavHandlerBase
{
	void Handle(const mavlink_message_t& Msg, UMavLinkFactStore& FactStore, uint64 FrameCount) override
	{
		mavlink_attitude_t Attitude;
		mavlink_msg_attitude_decode(&Msg, &Attitude);

		auto Category = FactStore.Attitude;
		Category.GetOrCreate("roll").SetValue(Attitude.roll, FrameCount);
		Category.GetOrCreate("pitch").SetValue(Attitude.pitch, FrameCount);
		Category.GetOrCreate("yaw").SetValue(Attitude.yaw, FrameCount);
		Category.GetOrCreate("rollspeed").SetValue(Attitude.rollspeed, FrameCount);
		Category.GetOrCreate("pitchspeed").SetValue(Attitude.pitchspeed, FrameCount);
		Category.GetOrCreate("yawspeed").SetValue(Attitude.yawspeed, FrameCount);
	}
};