// Fill out your copyright notice in the Description page of Project Settings.

#include "MavLinkHeartbeatComponent.h"
#include "LinkManagerSubsystem.h"

// Sets default values for this component's properties
UMavLinkHeartbeatComponent::UMavLinkHeartbeatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// Called when the game starts
void UMavLinkHeartbeatComponent::BeginPlay()
{
	Super::BeginPlay();
	auto World = GetWorld();
	if (World) {
		if (auto gameInstance = World->GetGameInstance()) {
			CachedLinkManager = gameInstance->GetSubsystem<ULinkManagerSubsystem>();
		}
	}

	if (CachedLinkManager) {
		CachedLinkManager->OnMavLinkHeartbeatReceived.AddUObject(this, &UMavLinkHeartbeatComponent::HandleIncomingHeartbeat);
	}
	World->GetTimerManager().SetTimer(
		HeartbeatSendTimer,
		this,
		&UMavLinkHeartbeatComponent::SendHeartbeat,
		HeartbeatInterval,
		true);
}

void UMavLinkHeartbeatComponent::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	if (auto World = GetWorld()) {
		World->GetTimerManager().ClearTimer(HeartbeatSendTimer);
		World->GetTimerManager().ClearTimer(TimeoutTimer);
		if (CachedLinkManager) {
			CachedLinkManager->OnMavLinkHeartbeatReceived.RemoveAll(this);
		}
	}
	Super::EndPlay(EndPlayReason);
}

void UMavLinkHeartbeatComponent::SendHeartbeat()
{
	if (CachedLinkManager) {
		mavlink_message_t	Msg;
		mavlink_heartbeat_t Data;
		FMemory::Memzero(&Data, sizeof(mavlink_heartbeat_t));

		Data.type = MAV_TYPE_QUADROTOR;
		Data.autopilot = MAV_AUTOPILOT_INVALID;
		Data.base_mode = MAV_MODE_FLAG_HIL_ENABLED;
		Data.system_status = MAV_STATE_ACTIVE;

		mavlink_msg_heartbeat_encode(GCS_SYSTEM_ID, MAV_COMP_ID_AUTOPILOT1, &Msg, &Data);
		CachedLinkManager->SendMavLinkMessage(VehicleId, Msg);
	}
}

void UMavLinkHeartbeatComponent::HandleIncomingHeartbeat(int32 InVehicleId, const mavlink_heartbeat_t& DecodedHeartbeat)
{
	if (InVehicleId != VehicleId) {
		return;
	}

	// TODO think how to optimize
	if (auto World = GetWorld()) {
		World->GetTimerManager().SetTimer(
			TimeoutTimer,
			this,
			&UMavLinkHeartbeatComponent::HandleConnectionTimeout,
			ConnectionTimeout,
			false);
	}

	if (!bIsConnected) {
		bIsConnected = true;
		UE_LOG(LogTemp, Log, TEXT("[MavLinkHeartbeatComponent::HandleIncomingHeartbeat] VehicleId %d connection to ArduPilot Established!"), VehicleId);
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
