#include "MavVehicleLink.h"
#include "MavLinkUDPWorker.h"
#include "MavLinkTCPWorker.h"
#include "MavLinkDispatcher.h"
#include "MavLinkCommandBus.h"

bool UMavVehicleLink::Initialize(const FVehicleNetworkConfig& Config)
{
	VehicleId = Config.VehicleId;
	FactStore = NewObject<UMavLinkFactStore>(this);
	CommandBus = NewObject<UMavLinkCommandBus>(this);
	Dispatcher = NewObject<UMavLinkDispatcher>(this);

	CommandBus->Initialize(&NetworkChannels, FactStore, VehicleId);
	Dispatcher->Initialize(&NetworkChannels, FactStore);

	UDPWorker = MakeUnique<FMavLinkUDPWorker>(Config, &NetworkChannels);
	TCPWorker = MakeUnique<FMavLinkTCPWorker>(Config, &NetworkChannels);

	UDPThread = FRunnableThread::Create(UDPWorker.Get(), *FString::Printf(TEXT("SITL_UDP_%d"), VehicleId), 0, TPri_AboveNormal);
	TCPThread = FRunnableThread::Create(TCPWorker.Get(), *FString::Printf(TEXT("SITL_UDP_%d"), VehicleId), 0, TPri_AboveNormal);

	return UDPThread && TCPThread;
}

void UMavVehicleLink::Tick(float DeltaTime)
{
	Dispatcher->ReceiveFacts(GFrameCounter);
	CommandBus->SendCommands();
}

ETickableTickType UMavVehicleLink::GetTickableTickType() const
{
	return ETickableTickType::Conditional;
}

bool UMavVehicleLink::IsTickable() const
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

TStatId UMavVehicleLink::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UMavVehicleLink, STATGROUP_Tickables);
}

void UMavVehicleLink::Shutdown()
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
