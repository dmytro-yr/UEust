#include "MavLinkHeartbeatComponent.h"
#include "LinkManagerSubsystem.h"
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
	const auto* World = GetWorld();

	if (!World) {
		return;
	}

	if (const UGameInstance* GI = World->GetGameInstance()) {
		auto LinkManager = GI->GetSubsystem<ULinkManagerSubsystem>();
		if (!LinkManager) {
			UE_LOG(LogTemp, Warning, TEXT("[UMavLinkHeartbeatComponent::BeginPlay] Failed to get LinkManagerSubsystem"));
			return;
		}

		VehicleLink = LinkManager->GetVehicleLink(VehicleId);
	}

	if (!VehicleLink) {
		UE_LOG(LogTemp, Error, TEXT("[UMavLinkHeartbeatComponent::BeginPlay] No VehicleLink for id %d"), VehicleId);
		return;
	}

	VehicleLink->GetFactStore()->Status.OnAnyChanged.AddUObject(this, &UMavLinkHeartbeatComponent::OnStatusFactChanged);

	World->GetTimerManager().SetTimer(
		HeartbeatTimer,
		this,
		&UMavLinkHeartbeatComponent::SendHeartbeat,
		HeartbeatInterval,
		true);
}

void UMavLinkHeartbeatComponent::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	if (auto World = GetWorld()) {
		World->GetTimerManager().ClearTimer(HeartbeatTimer);
		World->GetTimerManager().ClearTimer(TimeoutTimer);
		if (VehicleLink) {
			VehicleLink->GetFactStore()->Status.OnAnyChanged.RemoveAll(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void UMavLinkHeartbeatComponent::SendHeartbeat()
{
	if (!VehicleLink) {
		UE_LOG(LogTemp, Error, TEXT("[UMavLinkHeartbeatComponent::SendHeartbeat] No VehicleLink for id %d"), VehicleId);
	}

	mavlink_message_t	Msg;
	mavlink_heartbeat_t Data;
	FMemory::Memzero(&Data, sizeof(mavlink_heartbeat_t));

	Data.type = MAV_TYPE_GCS;
	Data.autopilot = MAV_AUTOPILOT_INVALID;
	Data.base_mode = 0;
	Data.system_status = MAV_STATE_ACTIVE;
	UE_LOG(LogTemp, Log, TEXT("CmdLog:[UMavLinkHeartbeatComponent::SendHeartbeat] Sended for VehicleLink with id %d"), VehicleId);
	mavlink_msg_heartbeat_encode(GCS_SYSTEM_ID, MAV_COMP_ID_MISSIONPLANNER, &Msg, &Data);
	VehicleLink->GetOutboundDispatcher()->Enqueue(Msg);
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
			TEXT("[UMavLinkHeartbeatComponent::OnStatusFactChanged] Vehicle %d connected."), VehicleId);
		OnConnectionStatusChanged.Broadcast(true);
	}
}

void UMavLinkHeartbeatComponent::HandleConnectionTimeout()
{
	if (bIsConnected) {
		bIsConnected = false;
		UE_LOG(LogTemp, Warning, TEXT("[MavLinkHeartbeatComponent::HandleConnectionTimeout] VehicleId %d connection to ArduPilot lost(Timeout)."), VehicleId);
		OnConnectionStatusChanged.Broadcast(false);
	}
}