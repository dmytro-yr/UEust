
#pragma once

#include "ThreadWorkerBase.h"
#include "NetworkTypes.h"

class FUDPWorker : public FThreadWorkerBase
{
public:
	FUDPWorker(const FVehicleNetworkConfig& Config, FNetworkChannels* InNetworkChannels);
	~FUDPWorker() override;

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
