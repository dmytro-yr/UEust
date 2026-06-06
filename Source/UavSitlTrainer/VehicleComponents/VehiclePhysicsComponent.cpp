// Fill out your copyright notice in the Description page of Project Settings.

#include "VehiclePhysicsComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

// Sets default values
UVehiclePhysicsComponent::UVehiclePhysicsComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UVehiclePhysicsComponent::BeginPlay()
{
	Super::BeginPlay();
	if (auto OwnerActor = GetOwner()) {
		if (auto SkeletalMesh = OwnerActor->FindComponentByClass<USkeletalMeshComponent>()) {
			VehicleMesh = Cast<UPrimitiveComponent>(SkeletalMesh);
		}
	}
	if (!VehicleMesh) {
		UE_LOG(LogTemp, Warning, TEXT("VehiclePhysicsComponent: Failed to find skeletal mesh"));
		return;
	}

	if (auto world = GetWorld()) {
		if (auto gameInstance = world->GetGameInstance()) {
			LinkManager = gameInstance->GetSubsystem<ULinkManagerSubsystem>();
		}
	}
	if (!LinkManager) {
		UE_LOG(LogTemp, Warning, TEXT("VehiclePhysicsComponent: Failed to get LinkManagerSubsystem"));
		return;
	}
	LinkManager->OnPhysicsDataReceived.AddDynamic(this, &UVehiclePhysicsComponent::HandleIncomingThrottles);
}

void UVehiclePhysicsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!VehicleMesh || !LinkManager) {
		UE_LOG(LogTemp, Warning, TEXT("VehiclePhysicsComponent: Failed to get vehicle mesh or link manager. Skipping tick."));
		return;
	}
	// may be call from LinkManager
	LinkManager->FlushInboundGameThreadQueues();
	LinkManager->FlushInboundMavlinkQueues();
	ApplyMotorForces();
}

void UVehiclePhysicsComponent::UpdateMotorThrottles(const float InThrottles[8])
{
	for (int i = 0; i < 8; ++i) {
		MotorThrottles[i] = InThrottles[i];
	}
}

void UVehiclePhysicsComponent::HandleIncomingThrottles(int32 InVehicleId, const TArray<float>& NormalizedThrottles)
{
	if (InVehicleId != VehicleId) {
		return;
	}
	if (NormalizedThrottles.Num() != 8) {
		UE_LOG(LogTemp, Warning, TEXT("Received throttle data with incorrect number of elements: %d"), NormalizedThrottles.Num());
		return;
	}

	int32 RotorCount = FMath::Min(NormalizedThrottles.Num(), 8);
	for (int i = 0; i < RotorCount; ++i) {
		MotorThrottles[i] = NormalizedThrottles[i];
	}
}

void UVehiclePhysicsComponent::ApplyMotorForces()
{
	if (!VehicleMesh) {
		UE_LOG(LogTemp, Warning, TEXT("VehiclePhysicsComponent: No vehicle mesh found. Cannot apply motor forces."));
		return;
	}
	if (!VehicleMesh->IsSimulatingPhysics()) {
		UE_LOG(LogTemp, Warning, TEXT("VehiclePhysicsComponent: Vehicle mesh is not simulating physics. Cannot apply motor forces."));
		return;
	}

	for (const FVehicleRotorConfig& RotorConfig : ActiveRotorConfigs) {
		bool bIsValidConfig = RotorConfig.RotorBoneName != NAME_None && RotorConfig.RotorIndex >= 0 && RotorConfig.RotorIndex < 8;
		if (!bIsValidConfig) {
			UE_LOG(LogTemp, Warning, TEXT("Invalid rotor config for bone %s"), *RotorConfig.RotorBoneName.ToString());
			continue;
		}
		float CurrentThrottle = MotorThrottles[RotorConfig.RotorIndex];
		float CalculatedForce = CurrentThrottle * RotorConfig.RotorMaxThrustNewtons * 1000.0f; // Convert from N to Unreal units (cm*kg/s^2)

		FVector WorldThrustDirection = VehicleMesh->GetComponentTransform().TransformVectorNoScale(RotorConfig.RotorDirection);
		FVector ForceToApply = WorldThrustDirection * CalculatedForce;

		VehicleMesh->AddForce(ForceToApply, RotorConfig.RotorBoneName, true);
	}
}
