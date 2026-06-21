#include "VehicleLink.h"
#include "Dispatchers/InboundDispatcher.h"
#include "Dispatchers/OutboundDispatcher.h"

bool UVehicleLink::Initialize(const FVehicleNetworkConfig& Config)
{
	VehicleId = Config.VehicleId;
	FactStore = NewObject<UMavLinkFactStore>(this);
	OutboundDispatcher = NewObject<UOutboundDispatcher>(this);
	InboundDispatcher = NewObject<UInboundDispatcher>(this);

	OutboundDispatcher->Initialize(&NetworkChannels, FactStore, VehicleId);
	InboundDispatcher->Initialize(&NetworkChannels, FactStore);

	UDPWorker = MakeUnique<FUDPWorker>(Config, &NetworkChannels);
	TCPWorker = MakeUnique<FTCPMavLinkWorker>(Config, &NetworkChannels);

	UDPThread = FRunnableThread::Create(UDPWorker.Get(), *FString::Printf(TEXT("SITL_UDP_%d"), VehicleId), 0, TPri_AboveNormal);
	TCPThread = FRunnableThread::Create(TCPWorker.Get(), *FString::Printf(TEXT("SITL_UDP_%d"), VehicleId), 0, TPri_AboveNormal);

	return UDPThread && TCPThread;
}

void UVehicleLink::Tick(float DeltaTime)
{
	InboundDispatcher->ReceiveFacts(GFrameCounter);
	OutboundDispatcher->SendCommands();
}

ETickableTickType UVehicleLink::GetTickableTickType() const
{
	return ETickableTickType::Conditional;
}

bool UVehicleLink::IsTickable() const
{
	if (HasAnyFlags(RF_ClassDefaultObject) || IsTemplate()) {
		return false;
	}

	if (VehicleId == INDEX_NONE) {
		return false;
	}

	const UWorld* MyWorld = GetWorld();
	if (!MyWorld || !MyWorld->IsGameWorld()) {
		return false;
	}

	return true;
}

TStatId UVehicleLink::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UVehicleLink, STATGROUP_Tickables);
}

void UVehicleLink::Shutdown()
{
	if (UDPWorker) {
		UDPWorker->Stop();
	}
	if (TCPWorker) {
		TCPWorker->Stop();
	}
	if (UDPThread) {
		UDPThread->WaitForCompletion();
		delete UDPThread;
		UDPThread = nullptr;
	}
	if (TCPThread) {
		TCPThread->WaitForCompletion();
		delete TCPThread;
		TCPThread = nullptr;
	}

	VehicleId = INDEX_NONE;
}
