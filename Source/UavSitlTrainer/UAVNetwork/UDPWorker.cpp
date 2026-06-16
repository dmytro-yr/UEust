#include "UDPWorker.h"

FUDPWorker::FUDPWorker(const FVehicleNetworkConfig& Config, FNetworkChannels* InNetworkChannels)
	: Config(Config), NetworkChannels(InNetworkChannels)
{
	StopTaskCounter.Reset();
}

FUDPWorker::~FUDPWorker()
{
	Stop();
}

bool FUDPWorker::Init()
{
	TxSocket = CreateSocket(NAME_DGram, TEXT("SITL_UDP_Tx"));
	RxSocket = CreateSocket(NAME_DGram, TEXT("SITL_UDP_Rx"));
	if (!TxSocket || !RxSocket) {
		return false;
	}

	TxAddr = CreateAddr(Config.IPAdress, Config.PhysicsTxPort);
	RxSocket->Bind(*CreateBindAddr(Config.PhysicsRxPort));

	RecvBuf.SetNumUninitialized(4096);

	return true;
}

void FUDPWorker::Stop()
{
	FThreadWorkerBase::Stop();
	DestroySocket(TxSocket);
	DestroySocket(RxSocket);
}

void FUDPWorker::DrainOutbound()
{
	FString Json;

	if (NetworkChannels->OutboundMavUDP.Dequeue(Json)) {

		FTCHARToUTF8 Conv(*Json);
		int32		 Sent = 0;
		int32		 BytesToSend = Conv.Length() + 1;
		bool		 bIsJsonSended = TxSocket->SendTo((uint8*)Conv.Get(), BytesToSend, Sent, *TxAddr);
	}
}

void FUDPWorker::ReceiveInbound()
{
	uint32 Pending = 0;
	int32  Read = 0;

	TSharedRef<FInternetAddr> From = GetSocketSubsystem()->CreateInternetAddr();
	if (RxSocket->RecvFrom(RecvBuf.GetData(), RecvBuf.Num(), Read, *From)
		&& Read == sizeof(FActuatorFrameOutputPacket)) {

		FActuatorFrameOutputPacket* Data = reinterpret_cast<FActuatorFrameOutputPacket*>(RecvBuf.GetData());

		if (Data->Magic == 18458) {

			TxAddr = From->Clone();
			FActuatorFrame Frame;
			FMemory::Memcpy(&Frame, RecvBuf.GetData(), sizeof(FActuatorFrame));
			NetworkChannels->InboundMavUDP.Enqueue(Frame);
		}
	}
}

void FUDPWorker::OnSleep()
{
	// Check the actual socket state one last time before sleeping
	uint32 PendingDataSize = 0;
	bool   bHadTraffic = RxSocket->HasPendingData(PendingDataSize) && PendingDataSize > 0;

	if (bHadTraffic) {
		// Telemetry frames are actively streaming!
		// Sleep for the standard micro-nap (SleepSeconds) to maintain maximum frequency.
		FPlatformProcess::Sleep(SleepSeconds);
	}
	else {
		// The network pipe is dead quiet.
		// Force a longer 10ms sleep to completely save your CPU cores from spinning.
		FPlatformProcess::Sleep(0.01f);
	}
}