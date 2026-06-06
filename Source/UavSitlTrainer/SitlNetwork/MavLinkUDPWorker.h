// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "Containers/Queue.h"
#include "Sockets.h"
#include "NetworkTypes.h"
#include "MavLinkIncludes.h"
#include "LinkManagerSubsystem.h"

using mavlink_callback_t = TFunction<void(const mavlink_message_t&)>;

class UAVSITLTRAINER_API FMavLinkUDPWorker : public FRunnable
{
public:
	FMavLinkUDPWorker(FVehicleNetworkConfig Config, mavlink_callback_t OnReceivedCallback = nullptr);
	~FMavLinkUDPWorker() override;

	// FRunnable interface
	virtual bool   Init() override;
	virtual uint32 Run() override;
	virtual void   Stop() override;

public:
	// Thread safe outbound transmission pipeline
	TQueue<mavlink_message_t, EQueueMode::Mpsc> OutboundMavlinkUDPQueue;

private:
	FVehicleNetworkConfig Config;
	FThreadSafeCounter	  StopTaskCounter;

	FSocket*				  MavlinkUDPSocket = nullptr;
	TSharedPtr<FInternetAddr> RemoteAddr;
	mavlink_callback_t		  OnMavlinkReceivedCallback;
	// Mavlink parser state machines !!!! may be no need
	mavlink_status_t  MavStatus;
	mavlink_message_t MavMessage;
};
