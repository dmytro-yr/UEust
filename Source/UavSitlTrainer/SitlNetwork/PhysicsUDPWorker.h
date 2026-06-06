// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "Containers/Queue.h"
#include "Sockets.h"
#include "NetworkTypes.h"
#include "LinkManagerSubsystem.h"

using callback_t = TFunction<void(TArray<uint8>&)>;

class UAVSITLTRAINER_API FPhysicsUDPWorker : public FRunnable
{

public:
	FPhysicsUDPWorker(FVehicleNetworkConfig Config, callback_t OnReceivedCallback = nullptr);
	virtual ~FPhysicsUDPWorker() override;

	// FRunnable interface
	virtual bool   Init() override;
	virtual uint32 Run() override;
	virtual void   Stop() override;

public:
	TQueue<FString, EQueueMode::Mpsc>		OutboundPhysicsQueue;
	TQueue<TArray<uint8>, EQueueMode::Mpsc> InboundActuatorQueue;

private:
	FVehicleNetworkConfig Config;
	FThreadSafeCounter	  StopTaskCounter;

	FSocket*				  TxSocket = nullptr;
	FSocket*				  RxSocket = nullptr;
	TSharedPtr<FInternetAddr> RemoteAddr;
	callback_t				  OnDataReceivedCallback;
};
