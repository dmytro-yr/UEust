#include "LinkManagerSubsystem.h"
#include "UAVNetwork/VehicleLink.h"

void ULinkManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void ULinkManagerSubsystem::Deinitialize()
{
	TArray<int32> VehicleId;
	VehicleLinks.GetKeys(VehicleId);
	for (int32 Id : VehicleId)
		DisconnectVehicle(Id);

	Super::Deinitialize();
}

bool ULinkManagerSubsystem::ConnectVehicle(const FVehicleNetworkConfig& Config)
{
	if (VehicleLinks.Contains(Config.VehicleId)) {
		UE_LOG(LogTemp, Warning, TEXT("[LinkManager::ConnectVehicle] Vehicle %d already connected."), Config.VehicleId);
		return VehicleLinks[Config.VehicleId];
	}

	UVehicleLink* VehicleLink = NewObject<UVehicleLink>(this);
	if (!VehicleLink->Initialize(Config)) {
		UE_LOG(LogTemp, Error, TEXT("[LinkManager::ConnectVehicle] Failed to initialize Vehicle %d."), Config.VehicleId);
		return false;
	}

	VehicleLinks.Add(Config.VehicleId, VehicleLink);

	UE_LOG(LogTemp, Log, TEXT("[LinkManager::ConnectVehicle] Vehicle %d connected."), Config.VehicleId);
	return VehicleLink;
}

void ULinkManagerSubsystem::DisconnectVehicle(int32 VehicleId)
{
	TObjectPtr<UVehicleLink> FoundVehicleLink = VehicleLinks.FindRef(VehicleId);
	if (!FoundVehicleLink) {
		UE_LOG(LogTemp, Warning, TEXT("[LinkManager::DisconnectVehicle] Vehicle %d already disconnected."), VehicleId);
		return;
	}
	FoundVehicleLink->Shutdown();
	VehicleLinks.Remove(VehicleId);

	UE_LOG(LogTemp, Log, TEXT("[LinkManager::DisconnectVehicle]] Vehicle %d disconnected."), VehicleId);
}

UVehicleLink* ULinkManagerSubsystem::GetVehicleLink(int32 VehicleId) const
{
	const TObjectPtr<UVehicleLink> FoundVehicleLink = VehicleLinks.FindRef(VehicleId);
	return FoundVehicleLink;
}

bool ULinkManagerSubsystem::IsVehicleConnected(int32 VehicleId) const
{
	return VehicleLinks.Contains(VehicleId);
}