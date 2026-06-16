#include "LinkManagerSubsystem.h"
#include "SitlNetwork/MavVehicleLink.h"

void ULinkManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void ULinkManagerSubsystem::Deinitialize()
{
	TArray<int32> VehicleId;
	MavVehicleLinks.GetKeys(VehicleId);
	for (int32 Id : VehicleId)
		DisconnectVehicle(Id);

	Super::Deinitialize();
}

bool ULinkManagerSubsystem::ConnectVehicle(const FVehicleNetworkConfig& Config)
{
	if (MavVehicleLinks.Contains(Config.VehicleId)) {
		UE_LOG(LogTemp, Warning, TEXT("[LinkManager::ConnectVehicle] Vehicle %d already connected."), Config.VehicleId);
		return MavVehicleLinks[Config.VehicleId];
	}

	UMavVehicleLink* VehicleLink = NewObject<UMavVehicleLink>(this);
	if (!VehicleLink->Initialize(Config)) {
		UE_LOG(LogTemp, Error, TEXT("[LinkManager::ConnectVehicle] Failed to initialize Vehicle %d."), Config.VehicleId);
		return false;
	}

	MavVehicleLinks.Add(Config.VehicleId, VehicleLink);

	UE_LOG(LogTemp, Log, TEXT("[LinkManager::ConnectVehicle] Vehicle %d connected."), Config.VehicleId);
	return VehicleLink;
}

void ULinkManagerSubsystem::DisconnectVehicle(int32 VehicleId)
{
	TObjectPtr<UMavVehicleLink> FoundVehicleLink = MavVehicleLinks.FindRef(VehicleId);
	if (!FoundVehicleLink) {
		UE_LOG(LogTemp, Warning, TEXT("[LinkManager::DisconnectVehicle] Vehicle %d already disconnected."), VehicleId);
		return;
	}
	FoundVehicleLink->Shutdown();
	MavVehicleLinks.Remove(VehicleId);

	UE_LOG(LogTemp, Log, TEXT("[LinkManager::DisconnectVehicle]] Vehicle %d disconnected."), VehicleId);
}

UMavVehicleLink* ULinkManagerSubsystem::GetMavVehicleLink(int32 VehicleId) const
{
	const TObjectPtr<UMavVehicleLink> FoundVehicleLink = MavVehicleLinks.FindRef(VehicleId);
	return FoundVehicleLink;
}

bool ULinkManagerSubsystem::IsVehicleConnected(int32 VehicleId) const
{
	return MavVehicleLinks.Contains(VehicleId);
}