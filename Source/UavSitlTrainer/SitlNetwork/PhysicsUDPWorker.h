// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "Containers/Queue.h"
#include "Sockets.h"
#include "LinkManagerSubsystem.h"

class UAVSITLTRAINER_API FPhysicsUDPWorker : public FRunnable
{

public:
	FPhysicsUDPWorker(FVehicleNetworkConfig Config);
	virtual ~FPhysicsUDPWorker() override;

	// FRunnable interface
	virtual bool   Init() override;
	virtual uint32 Run() override;
	virtual void   Stop() override;

	TQueue<FString, EQueueMode::Mpsc>		OutboundPhysicsQueue;
	TQueue<TArray<uint8>, EQueueMode::Mpsc> InboundActuatorQueue;

private:
	FVehicleNetworkConfig Config;
	FThreadSafeCounter	  StopTaskCounter;

	FSocket*				  TxSocket = nullptr;
	FSocket*				  RxSocket = nullptr;
	TSharedPtr<FInternetAddr> RemoteAddr;
};
