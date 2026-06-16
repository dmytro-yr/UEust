
#pragma once

#include "CoreMinimal.h"
#include "MavLinkIncludes.h"

#include "NetworkTypes.generated.h"

static constexpr uint8 GCS_SYSTEM_ID = 255;
static constexpr float PWM_MIN = 1100.f;
static constexpr float PWM_RANGE = 800.f;
#define RAW_ACTUATORS_OUTPUT FActuatorFrame::MsgId

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

// ardupilot raw output
struct FActuatorFrameOutputPacket
{
	uint16 Magic;
	uint16 FrameRate;
	uint32 FrameCount;
	uint16 PWM[16];
};

struct FActuatorFrame
{
	static constexpr uint32 MsgId = 181600;
	uint16					MotorPulses[16] = {};

	float NormalizedThrottle(int32 i) const
	{
		return FMath::Clamp((MotorPulses[i] - PWM_MIN) / PWM_RANGE, 0.f, 1.f);
	}
};

struct FNetworkChannels
{
	// TCP channel  (MAVLink)
	TQueue<mavlink_message_t, EQueueMode::Mpsc> InboundMavTCP;
	TQueue<mavlink_message_t, EQueueMode::Mpsc> OutboundMavTCP;

	// UDP channel  (physics)
	TQueue<FActuatorFrame, EQueueMode::Mpsc> InboundMavUDP;
	TQueue<FString, EQueueMode::Mpsc>		 OutboundMavUDP;
};