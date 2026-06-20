#include "MavLinkHeartbeatComponent.h"
#include "Vehicle/VehiclePawn.h"
#include "UAVNetwork/VehicleLink.h"
#include "UAVNetwork/MavLinkFactStore.h"
#include "UAVNetwork/OutboundDispatcher.h"

// Sets default values for this component's properties
UMavLinkHeartbeatComponent::UMavLinkHeartbeatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// Called when the game starts
void UMavLinkHeartbeatComponent::BeginPlay()
{
	Super::BeginPlay();
	const auto* VehiclePawn = Cast<AVehiclePawn>(GetOwner());
	if (!VehiclePawn) {
		UE_LOG(LogTemp, Error, TEXT("[UMavLinkHeartbeatComponent::BeginPlay] No VehiclePawn"));
		return;
	}

	VehicleLinkPtr = VehiclePawn->GetVehicleLinkPtr();
	int32 VehicleId = VehiclePawn->GetVehicleId();
	if (!VehicleLinkPtr) {
		UE_LOG(LogTemp, Error, TEXT("[UMavLinkHeartbeatComponent::BeginPlay] No VehicleLink for id %d"), VehicleId);
	}

	VehicleLinkPtr->GetFactStore()->Status.OnAnyChanged.AddUObject(this, &UMavLinkHeartbeatComponent::OnStatusFactChanged);
	if (auto World = GetWorld()) {
		World->GetTimerManager().SetTimer(
			HeartbeatTimer,
			this,
			&UMavLinkHeartbeatComponent::SendHeartbeat,
			HeartbeatInterval,
			true);
	}
}

void UMavLinkHeartbeatComponent::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	if (auto World = GetWorld()) {
		World->GetTimerManager().ClearTimer(HeartbeatTimer);
		World->GetTimerManager().ClearTimer(TimeoutTimer);
		if (VehicleLinkPtr) {
			VehicleLinkPtr->GetFactStore()->Status.OnAnyChanged.RemoveAll(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void UMavLinkHeartbeatComponent::SendHeartbeat()
{
	if (!VehicleLinkPtr) {
		UE_LOG(LogTemp, Error, TEXT("[UMavLinkHeartbeatComponent::SendHeartbeat] The VehicleLink was not set."));
		return;
	}

	mavlink_message_t	Msg;
	mavlink_heartbeat_t Data;
	FMemory::Memzero(&Data, sizeof(mavlink_heartbeat_t));

	Data.type = MAV_TYPE_GCS;
	Data.autopilot = MAV_AUTOPILOT_INVALID;
	Data.base_mode = 0;
	Data.system_status = MAV_STATE_ACTIVE;
	UE_LOG(LogTemp, Log, TEXT("[UMavLinkHeartbeatComponent::SendHeartbeat] Sended for VehicleLink with id %d"), VehicleLinkPtr->GetVehicleId());
	mavlink_msg_heartbeat_encode(GCS_SYSTEM_ID, MAV_COMP_ID_MISSIONPLANNER, &Msg, &Data);
	VehicleLinkPtr->GetOutboundDispatcher()->Enqueue(Msg);
}

void UMavLinkHeartbeatComponent::OnStatusFactChanged(FName Key, float Value)
{
	// We only care about the heartbeat fact — ignore everything else in Status
	if (Key != FName("system_status")) {
		return;
	}

	if (const auto* World = GetWorld()) {
		World->GetTimerManager().SetTimer(
			TimeoutTimer,
			this,
			&UMavLinkHeartbeatComponent::HandleConnectionTimeout,
			ConnectionTimeout,
			false);
	}

	if (!bIsConnected) {
		bIsConnected = true;
		UE_LOG(LogTemp, Log,
			TEXT("[UMavLinkHeartbeatComponent::OnStatusFactChanged] Vehicle %d connected."), VehicleLinkPtr->GetVehicleId());
		OnConnectionStatusChanged.Broadcast(true);
	}
}

void UMavLinkHeartbeatComponent::HandleConnectionTimeout()
{
	if (bIsConnected) {
		bIsConnected = false;
		UE_LOG(LogTemp, Warning,
			TEXT("[MavLinkHeartbeatComponent::HandleConnectionTimeout] VehicleId %d connection to ArduPilot lost(Timeout)."), VehicleLinkPtr->GetVehicleId());
		OnConnectionStatusChanged.Broadcast(false);
	}
}