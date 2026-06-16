#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "NetworkTypes.h"
#include "OutboundDispatcher.generated.h"

class UMavLinkFactStore;

UCLASS()
class UOutboundDispatcher : public UObject
{
	GENERATED_BODY()
public:
	void Initialize(FNetworkChannels* InNetworkChannels, UMavLinkFactStore* FactStore, int32 InVehicleId);

	void Enqueue(const mavlink_message_t& Msg);

	void Enqueue(const FString& OutboundJson);

	// Reliable — stores pending, retries, calls OnResult on ACK or timeout
	void SendCommand(uint16 CommandId, const mavlink_message_t& Msg, TFunction<void(bool)> OnResult);

	// Called by InboundDispatcher's CmdAckHandler
	void OnAckReceived(FName Key, float Result);

	// Called each tick from UVehicleLink::Tick
	void SendCommands();

private:
	struct FPendingCommand
	{
		uint16				  CommandId;
		mavlink_message_t	  Msg;
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
