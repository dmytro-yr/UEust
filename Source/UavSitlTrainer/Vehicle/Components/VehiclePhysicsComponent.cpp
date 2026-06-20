// Fill out your copyright notice in the Description page of Project Settings.

#include "VehiclePhysicsComponent.h"
#include "Engine/World.h"
#include "Vehicle/VehiclePawn.h"
#include "UAVNetwork/VehicleLink.h"
#include "UAVNetwork/MavLinkFactStore.h"

// Sets default values
UVehiclePhysicsComponent::UVehiclePhysicsComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UVehiclePhysicsComponent::BeginPlay()
{
	Super::BeginPlay();
	const auto* VehiclePawn = Cast<AVehiclePawn>(GetOwner());
	if (!VehiclePawn) {
		UE_LOG(LogTemp, Error, TEXT("[UVehiclePhysicsComponent::BeginPlay] No VehiclePawn."));
		return;
	}

	VehicleLinkPtr = VehiclePawn->GetVehicleLinkPtr();
	int32 VehicleId = VehiclePawn->GetVehicleId();
	if (!VehicleLinkPtr) {
		UE_LOG(LogTemp, Error, TEXT("[UVehiclePhysicsComponent::BeginPlay] No VehicleLink for id %d"), VehicleId);
	}

	if (auto* SkeletalMesh = VehiclePawn->FindComponentByClass<USkeletalMeshComponent>()) {
		VehicleMeshPtr = Cast<UPrimitiveComponent>(SkeletalMesh);
	}

	if (!VehicleMeshPtr) {
		UE_LOG(LogTemp, Warning, TEXT("[UVehiclePhysicsComponent::BeginPlay] Failed to find skeletal mesh"));
	}
}

void UVehiclePhysicsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!VehicleMeshPtr || !VehicleLinkPtr) {
		UE_LOG(LogTemp, Warning, TEXT("[UVehiclePhysicsComponent::TickComponent] Failed to get vehicle mesh or link manager. Skipping tick."));
		return;
	}
	ApplyMotorForces();
}

void UVehiclePhysicsComponent::ApplyMotorForces()
{
	if (!VehicleMeshPtr) {
		return;
	}
	if (!VehicleMeshPtr->IsSimulatingPhysics()) {
		return;
	}

	for (const FVehicleRotorConfig& CurrentRotor : ActiveRotorConfigs) {
		bool bIsValidConfig = CurrentRotor.RotorBoneName != NAME_None && CurrentRotor.RotorIndex >= 0 && CurrentRotor.RotorIndex < 8;
		if (!bIsValidConfig) {
			UE_LOG(LogTemp, Warning,
				TEXT("[UVehiclePhysicsComponent::ApplyMotorForces] Invalid rotor config for bone %s"), *CurrentRotor.RotorBoneName.ToString());
			continue;
		}

		float Throttle = 0.f;
		FName Key(*FString::Printf(TEXT("motor_%d"), CurrentRotor.RotorIndex));
		UE_LOG(LogTemp, Warning, TEXT("[UVehiclePhysicsComponent::ApplyMotorForces] We inside cycle apply motor forces"));

		if (VehicleLinkPtr->GetFactStore()->Actuators.GetValue(Key, Throttle)) {

			float CalculatedForce = Throttle * CurrentRotor.RotorMaxThrustNewtons * 100.0f; // Convert from N to Unreal units (cm*kg/s^2)

			FVector WorldThrustDirection = VehicleMeshPtr->GetComponentTransform().TransformVectorNoScale(CurrentRotor.RotorDirection);
			FVector ForceToApply = WorldThrustDirection * CalculatedForce;
			UE_LOG(LogTemp, Warning,
				TEXT("[UVehiclePhysicsComponent::ApplyMotorForces] Apply motor forces. %s = %s"), *CurrentRotor.RotorBoneName.ToString(), *ForceToApply.ToString());
			VehicleMeshPtr->AddForce(ForceToApply, CurrentRotor.RotorBoneName, true);
		}
	}
}
