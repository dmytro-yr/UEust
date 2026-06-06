// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "Containers/Queue.h"
#include "Sockets.h"
#include "NetworkTypes.h"
#include "MavLinkIncludes.h"
#include "LinkManagerSubsystem.h"

using callback_frame_recieved_t = TFunction<void(const FMavTelemetryFrame&)>;

class UAVSITLTRAINER_API FMavLinkTCPWorker : public FRunnable
{
public:
	FMavLinkTCPWorker(const FVehicleNetworkConfig InConfig, callback_frame_recieved_t InOnFrameReceived);
	~FMavLinkTCPWorker();
	// FRunnable interface
	virtual bool   Init() override;
	virtual uint32 Run() override;
	virtual void   Stop() override;

	bool SendRawBytes(const uint8* Data, int32 Count);

private:
	FVehicleNetworkConfig Config;

public:
	callback_frame_recieved_t OnTelemetryFrameReceived;
	bool					  bIsConnected = false;

private:
	FThreadSafeCounter StopTaskCounter;

	FSocket*				  MavlinkTCPSocket = nullptr;
	TSharedPtr<FInternetAddr> RemoteAddr;
};
