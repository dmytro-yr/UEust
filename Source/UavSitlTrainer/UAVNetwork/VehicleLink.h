// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include "NetworkTypes.h"
#include "UDPWorker.h"
#include "TCPMavLinkWorker.h"

#include "VehicleLink.generated.h"

class UMavLinkFactStore;
class UInboundDispatcher;
class UOutboundDispatcher;

UCLASS()
class UAVSITLTRAINER_API UVehicleLink : public UObject, public FTickableGameObject
{
	GENERATED_BODY()

public:
	bool Initialize(const FVehicleNetworkConfig& Config); // manger connect

	virtual void			  Tick(float DeltaTime); // manager tick
	virtual ETickableTickType GetTickableTickType() const override;
	virtual TStatId			  GetStatId() const override;
	virtual bool			  IsTickable() const override;

	void Shutdown(); // in manager disconnect

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SITL")
	UMavLinkFactStore* GetFactStore() const { return FactStore; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SITL")
	UOutboundDispatcher* GetOutboundDispatcher() const { return OutboundDispatcher; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SITL")
	UInboundDispatcher* GetInboundDispatcher() const { return InboundDispatcher; }
	// TODO need avoid it, use command bus or dispatcher
	FNetworkChannels* GetNetworkChannels() { return &NetworkChannels; }

private:
	void SpawnThread(FRunnable* Worker, const FString& Name, FRunnableThread*& OutThread);
	void ShutdownThread(FRunnable* Worker, FRunnableThread*& OutThread);

private:
	int32 VehicleId = INDEX_NONE;

	FNetworkChannels NetworkChannels;

	TUniquePtr<FUDPWorker>		  UDPWorker;
	TUniquePtr<FTCPMavLinkWorker> TCPWorker;

	FRunnableThread* UDPThread = nullptr;
	FRunnableThread* TCPThread = nullptr;

	UPROPERTY()
	TObjectPtr<UMavLinkFactStore> FactStore;

	UPROPERTY()
	TObjectPtr<UOutboundDispatcher> OutboundDispatcher;

	UPROPERTY()
	TObjectPtr<UInboundDispatcher> InboundDispatcher;
};
