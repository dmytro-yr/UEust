// MavLinkCommandBus.cpp
#include "MavLinkCommandBus.h"
#include "MavLinkFactStore.h"

void UMavLinkCommandBus::Initialize(FNetworkChannels* InNetworkChannels, UMavLinkFactStore* FactStore, int32 InVehicleId)
{
	NetworkChannels = InNetworkChannels;
	VehicleId = InVehicleId;
	FactStore->CmdAck.OnAnyChanged.AddLambda(
		[this](FName Key, float Value) {
			OnAckReceived(Key, Value);
		});
}

void UMavLinkCommandBus::Enqueue(const mavlink_message_t& Msg, bool bUseTCP)
{
	if (bUseTCP) {
		NetworkChannels->OutboundMavTCP.Enqueue(Msg);
	}
	else {
		NetworkChannels->OutboundMavTCP.Enqueue(Msg); // TODO May be divide channels
	}
}

void UMavLinkCommandBus::SendCommand(uint16 CommandId,
	const mavlink_message_t&				Msg,
	bool									bUseTCP,
	TFunction<void(bool)>					OnResult)
{
	FPendingCommand PendingCommand;
	PendingCommand.CommandId = CommandId;
	PendingCommand.Msg = Msg;
	PendingCommand.bUseTCP = bUseTCP;
	PendingCommand.RetriesLeft = MaxRetries;
	PendingCommand.Deadline = FPlatformTime::Seconds() + RetryDelay;
	PendingCommand.OnResult = MoveTemp(OnResult);
	PendingCommands.Add(MoveTemp(PendingCommand));

	NetworkChannels->OutboundMavTCP.Enqueue(Msg);
}

void UMavLinkCommandBus::OnAckReceived(FName Key, float Result)
{
	FString KeyStr = Key.ToString();
	FString IdStr;
	KeyStr.Split(TEXT("_"), nullptr, &IdStr);
	uint16 CommandId = (uint16)FCString::Atoi(*IdStr);

	for (int32 i = PendingCommands.Num() - 1; i >= 0; --i) {
		if (PendingCommands[i].CommandId != CommandId) {
			continue;
		}
		PendingCommands[i].OnResult((uint8)Result == MAV_RESULT_ACCEPTED);
		PendingCommands.RemoveAt(i);
		return;
	}
}

void UMavLinkCommandBus::SendCommands()
{
	double Now = FPlatformTime::Seconds();
	for (int32 i = PendingCommands.Num() - 1; i >= 0; --i) {
		FPendingCommand& Command = PendingCommands[i];
		if (Now < Command.Deadline)
			continue;

		if (Command.RetriesLeft-- > 0) {
			NetworkChannels->OutboundMavTCP.Enqueue(Command.Msg);
			Command.Deadline = Now + RetryDelay;
		}
		else {
			Command.OnResult(false);
			PendingCommands.RemoveAt(i);
		}
	}
}
