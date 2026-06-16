#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "NetworkTypes.h"
#include "MavLinkCommandBus.generated.h"

class UMavLinkFactStore;

UCLASS()
class UMavLinkCommandBus : public UObject
{
	GENERATED_BODY()
public:
	void Initialize(FNetworkChannels* InNetworkChannels, UMavLinkFactStore* FactStore, int32 InVehicleId);

	// Fire-and-forget (heartbeat, RC override → UDP not needed here, heartbeat → TCP)
	void Enqueue(const mavlink_message_t& Msg, bool bUseTCP = true);

	// Reliable — stores pending, retries, calls OnResult on ACK or timeout
	void SendCommand(uint16 CommandId, const mavlink_message_t& Msg, bool bUseTCP, TFunction<void(bool)> OnResult);

	// Called by Dispatcher's CmdAckHandler
	void OnAckReceived(FName Key, float Result);

	// Called each tick from UVehicleLink::Tick
	void SendCommands();

private:
	struct FPendingCommand
	{
		uint16				  CommandId;
		mavlink_message_t	  Msg;
		bool				  bUseTCP;
		int32				  RetriesLeft;
		double				  Deadline;
		TFunction<void(bool)> OnResult;
	};

	FNetworkChannels*		NetworkChannels = nullptr;
	int32					VehicleId = -1;
	int32					MaxRetries = 5;
	float					RetryDelay = 1.f;
	TArray<FPendingCommand> PendingCommands;
};
