#pragma once

#include "MavLinkWorkerBase.h"
#include "NetworkTypes.h"

class FMavLinkTCPWorker : public FMavLinkWorkerBase
{
public:
	FMavLinkTCPWorker(const FVehicleNetworkConfig& Config, FNetworkChannels* InNetworkChannels);
	~FMavLinkTCPWorker() override;

	bool Init() override;
	void Stop() override;

protected:
	bool IsConnectionValid() override { return bIsConnected; }
	void DrainOutbound() override;
	void ReceiveInbound() override;
	void OnAfterLoop() override;

private:
	FVehicleNetworkConfig Config;
	FNetworkChannels*	  NetworkChannels = nullptr;
	FSocket*			  Socket = nullptr;
	bool				  bIsConnected = false;
	mavlink_status_t	  ParseStatus = {};
	TArray<uint8>		  RecvBuf;
};