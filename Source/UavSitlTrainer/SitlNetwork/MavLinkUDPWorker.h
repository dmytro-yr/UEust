
#pragma once

#include "MavLinkWorkerBase.h"
#include "NetworkTypes.h"

class FMavLinkUDPWorker : public FMavLinkWorkerBase
{
public:
	FMavLinkUDPWorker(const FVehicleNetworkConfig& Config, FNetworkChannels* InNetworkChannels);
	~FMavLinkUDPWorker() override;

	bool Init() override;
	void Stop() override;

protected:
	void DrainOutbound() override;
	void ReceiveInbound() override;
	void OnSleep() override;

private:
	FVehicleNetworkConfig	  Config;
	FNetworkChannels*		  NetworkChannels = nullptr;
	FSocket*				  TxSocket = nullptr;
	FSocket*				  RxSocket = nullptr;
	TSharedPtr<FInternetAddr> TxAddr;
	TArray<uint8>			  RecvBuf;
};
