#pragma once
#include "UAVNetwork/ReceiveHandlers/HandlerBase.h"

struct FActuatorsOutputsHandler : FRawFrameHandlerBase
{
	void Handle(const FActuatorFrame& ActuatorFrame, UMavLinkFactStore& FactStore, uint64 FrameCount) override
	{
		UE_LOG(LogTemp, Warning, TEXT("[FMavActuatorOutputsHandler::Handle]WE GET ACTUATOR FRAMES"));
		auto& Category = FactStore.Actuators;
		for (int32 i = 0; i < 16; ++i) {
			if (ActuatorFrame.MotorPulses[i] == 0) {
				continue;
			}
			FName Key(*FString::Printf(TEXT("motor_%d"), i));
			float Throttle = ActuatorFrame.NormalizedThrottle(i);
			Category.GetOrCreate(Key).SetValue(Throttle, FrameCount);
		}
	}
};