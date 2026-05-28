// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include "Containers/Queue.h"
#include "LinkManagerSubsystem.generated.h"

USTRUCT(blueprintType)
struct FVehicleNetworkConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SITL Config")
	int32 VehicleId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SITL Config")
	FString IPAdress = TEXT("127.0.0.1");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SITL Config")
	int32 PhysicsTxPort = 9002; // UE5 to AP

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SITL Config")
	int32 PhysicsRxPort = 9003; // AP to UE5

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SITL Config")
	int32 MavlinkPort = 14550; // GCS MAVLink streaming port
};

class FPhysicsUDPWorker;
class FMavlinkUDPWorker;

UCLASS()
class UAVSITLTRAINER_API ULinkManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Mavlink SITL")
	void ConnectVehicle(const FVehicleNetworkConfig& Config);

	UFUNCTION(BlueprintCallable, Category = "Mavlink SITL")
	void DisconnectVehicle(int32 VehicleId);

	// test methods

	UFUNCTION(BlueprintCallable, Category = "Mavlink SITL")
	void TestPushPhysicsString(int32 VehicleId, const FString& TestJson);

	UFUNCTION(BlueprintCallable, Category = "Mavlink SITL")
	FString TestPopActuatorString(int32 VehicleId);

private:
	struct FVehicleThreadContainer
	{
		FPhysicsUDPWorker* PhysicsRunnable = nullptr;
		FRunnableThread*   PhysicsThread = nullptr;

		FMavlinkUDPWorker* MavlinkRunnable = nullptr;
		FRunnableThread*   MavlinkThread = nullptr;
	};

	TMap<int32, FVehicleThreadContainer> ActiveVehicleThreads;
};
