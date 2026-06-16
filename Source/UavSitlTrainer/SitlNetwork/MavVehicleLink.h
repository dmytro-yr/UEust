// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include "NetworkTypes.h"
#include "MavLinkUDPWorker.h"
#include "MavLinkTCPWorker.h"

#include "MavVehicleLink.generated.h"

class UMavLinkFactStore;
class UMavLinkDispatcher;
class UMavLinkCommandBus;

UCLASS()
class UAVSITLTRAINER_API UMavVehicleLink : public UObject, public FTickableGameObject
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
	UMavLinkCommandBus* GetCommandBus() const { return CommandBus; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SITL")
	UMavLinkDispatcher* GetDispatcher() const { return Dispatcher; }
	// TODO need avoid it, use command bus or dispatcher
	FNetworkChannels* GetNetworkChannels() { return &NetworkChannels; }

private:
	void SpawnThread(FRunnable* Worker, const FString& Name, FRunnableThread*& OutThread);
	void ShutdownThread(FRunnable* Worker, FRunnableThread*& OutThread);

private:
	int32 VehicleId = INDEX_NONE;

	FNetworkChannels NetworkChannels;

	TUniquePtr<FMavLinkUDPWorker> UDPWorker;
	TUniquePtr<FMavLinkTCPWorker> TCPWorker;

	FRunnableThread* UDPThread = nullptr;
	FRunnableThread* TCPThread = nullptr;

	UPROPERTY()
	TObjectPtr<UMavLinkFactStore> FactStore;

	UPROPERTY()
	TObjectPtr<UMavLinkCommandBus> CommandBus;

	UPROPERTY()
	TObjectPtr<UMavLinkDispatcher> Dispatcher;
};
