// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include "Containers/Queue.h"
#include "SitlNetwork/NetworkTypes.h"
#include "LinkManagerSubsystem.generated.h"

class FPhysicsUDPWorker;
class FMavLinkUDPWorker;
class FMavLinkTCPWorker;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPhysicsDataReceived, int32, VehicleId, const TArray<float>&, NormalizedThrottles);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMavLinkParamReceived, int32, VehicleId, const FString&, ParamName, float, ParamValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnMavLinkAttitudeReceived, int32, VehicleId, float, Roll, float, Pitch, float, Yaw);

// heartbeat
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnMavLinkHeartbeatReceived, int32, const mavlink_heartbeat_t&);

UCLASS()
class UAVSITLTRAINER_API ULinkManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Mavlink SITL")
	bool ConnectVehicle(const FVehicleNetworkConfig& Config);

	UFUNCTION(BlueprintCallable, Category = "Mavlink SITL")
	void DisconnectVehicle(int32 VehicleId);

	void FlushInboundGameThreadQueues();

	void FlushInboundMavlinkQueues();

	void SendMavLinkMessage(int32 VehicleId, const mavlink_message_t& Msg);

	UFUNCTION(BlueprintCallable, Category = "Mavlink SITL")
	void PushPhysicsJson(int32 VehicleId, const FString& Json);

private:
	void ShutDownAndCleanThread(FRunnableThread*& Thread, const TSharedPtr<FRunnable>& Runnable);

public:
	UPROPERTY(BlueprintAssignable, Category = "Mavlink SITL | Signals")
	FOnPhysicsDataReceived OnPhysicsDataReceived;
	UPROPERTY(BlueprintAssignable, Category = "Mavlink SITL | Signals")
	FOnMavLinkParamReceived OnMavLinkParamReceived;
	UPROPERTY(BlueprintAssignable, Category = "Mavlink SITL | Signals")
	FOnMavLinkAttitudeReceived OnMavLinkAttitudeReceived;
	// heartbeat
	FOnMavLinkHeartbeatReceived OnMavLinkHeartbeatReceived;

private:
	struct FVehicleThreadContainer
	{
		TSharedPtr<FPhysicsUDPWorker> PhysicsRunnable;
		FRunnableThread*			  PhysicsThread = nullptr;

		TSharedPtr<FMavLinkUDPWorker> MavLinkUDPRunnable;
		FRunnableThread*			  MavLinkUDPThread = nullptr;

		TSharedPtr<FMavLinkTCPWorker> MavLinkTCPRunnable;
		FRunnableThread*			  MavLinkTCPThread = nullptr;
	};

	TMap<int32, FVehicleThreadContainer> ActiveVehicleThreads;

	TQueue<TArray<uint8>, EQueueMode::Mpsc>		 InboundActuatorDataQueue;
	TQueue<FMavTelemetryFrame, EQueueMode::Mpsc> InboundMavlinkDataQueue;
};
