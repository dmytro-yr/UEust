
#pragma once

#include "CoreMinimal.h"
#include "MavLinkIncludes.h"

#include "NetworkTypes.generated.h"

static constexpr uint8 GCS_SYSTEM_ID = 255;

USTRUCT(blueprintType)
struct FVehicleNetworkConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SITL Network Config")
	int32 VehicleId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SITL Network Config")
	FString IPAdress = TEXT("127.0.0.1");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SITL Network Config")
	int32 PhysicsTxPort = 9002; // UE5 to AP

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SITL Network Config")
	int32 PhysicsRxPort = 9003; // AP to UE5

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SITL Network Config")
	int32 MavlinkUDPPort = 14550; // GCS MAVLink streaming port

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SITL Network Config")
	int32 MavlinkTCPPort = 5760; // AP MAVLink TCP port
};

USTRUCT(BlueprintType)
struct FMavTelemetryFrame
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "SITL Networking")
	int32 VehicleId = 0;

	UPROPERTY(BlueprintReadOnly, Category = "SITL Networking")
	int32 MessageId = 0;

	mavlink_message_t Payload;
};