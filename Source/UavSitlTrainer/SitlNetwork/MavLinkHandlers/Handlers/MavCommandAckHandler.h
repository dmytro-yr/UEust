#pragma once

#include "SitlNetwork/MavLinkHandlers/MavHandlerBase.h"

struct FMavCommandAckHandler : FMavHandlerBase
{

	void Handle(const mavlink_message_t& Msg, UMavLinkFactStore& FactStore, uint64 FactNum)
	{
		mavlink_command_ack_t Ack;
		mavlink_msg_command_ack_decode(&Msg, &Ack);
		FString KeyName = FString::Printf(TEXT("ack_%d"), Ack.command);
		FName	Key(*KeyName);
		FactStore.CmdAck.GetOrCreate(Key).SetValue(Ack.result, FactNum);
	}
};