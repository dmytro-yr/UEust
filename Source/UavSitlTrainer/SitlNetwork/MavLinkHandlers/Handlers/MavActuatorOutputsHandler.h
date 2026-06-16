#pragma once
#include "SitlNetwork/MavLinkHandlers/MavHandlerBase.h"

struct FMavActuatorOutputsHandler : FMavHandlerBase
{
	void Handle(const mavlink_message_t& Msg, UMavLinkFactStore& FactStore, uint64 FrameCount) override
	{
		mavlink_actuator_output_status_t Actuators;
		mavlink_msg_actuator_output_status_decode(&Msg, &Actuators);

		auto& Category = FactStore.Actuators;
		for (int32 i = 0; i < FMath::CountBits(Actuators.active); ++i) {
			FName Key(*FString::Printf(TEXT("motor_%d"), i));
			// TODO remove magic numbers
			// Pulse width 1100–1900 µs → 0.0–1.0
			float Normalized = FMath::Clamp((Actuators.actuator[i] - 1100.f) / 800.f, 0.f, 1.f);
			Category.GetOrCreate(Key).SetValue(Normalized, FrameCount);
		}
	}
};