#pragma once

#include "ThreadWorkerBase.h"
#include "UAVNetwork/NetworkTypes.h"

class FTCPMavLinkWorker : public FThreadWorkerBase
{
public:
	FTCPMavLinkWorker(const FVehicleNetworkConfig& Config, FNetworkChannels* InNetworkChannels);
	~FTCPMavLinkWorker() override;

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