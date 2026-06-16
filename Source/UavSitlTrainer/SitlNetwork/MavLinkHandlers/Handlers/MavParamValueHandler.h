#pragma once
#include "SitlNetwork/MavLinkHandlers/MavHandlerBase.h"

struct FMavParamValueHandler : FMavHandlerBase
{
	void Handle(const mavlink_message_t& Msg, UMavLinkFactStore& FactStore, uint64 FrameCount) override
	{
		mavlink_param_value_t Param;
		mavlink_msg_param_value_decode(&Msg, &Param);
		FName Key(Param.param_id);
		FactStore.Params.GetOrCreate(Key).SetValue(Param.param_value, FrameCount);
	}
};