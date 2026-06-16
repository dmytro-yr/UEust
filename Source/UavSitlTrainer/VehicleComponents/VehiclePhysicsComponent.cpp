// Fill out your copyright notice in the Description page of Project Settings.

#include "VehiclePhysicsComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "LinkManagerSubsystem.h"
#include "SitlNetwork/MavVehicleLink.h"
#include "SitlNetwork/MavLinkFactStore.h"

// Sets default values
UVehiclePhysicsComponent::UVehiclePhysicsComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UVehiclePhysicsComponent::BeginPlay()
{
	Super::BeginPlay();

	if (const auto* OwnerActor = GetOwner()) {
		if (auto* SkeletalMesh = OwnerActor->FindComponentByClass<USkeletalMeshComponent>()) {
			VehicleMesh = Cast<UPrimitiveComponent>(SkeletalMesh);
		}
	}
	if (!VehicleMesh) {
		UE_LOG(LogTemp, Warning, TEXT("[VehiclePhysicsComponent::BeginPlay] Failed to find skeletal mesh"));
	}

	if (const auto* World = GetWorld()) {
		if (const UGameInstance* GI = World->GetGameInstance()) {
			LinkManager = GI->GetSubsystem<ULinkManagerSubsystem>();
		}
	}

	if (!LinkManager) {
		return;
	}

	MavVehicleLink = LinkManager->GetMavVehicleLink(VehicleId);
	if (!MavVehicleLink) {
	}
}

void UVehiclePhysicsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!VehicleMesh || !LinkManager) {
		UE_LOG(LogTemp, Warning, TEXT("VehiclePhysicsComponent: Failed to get vehicle mesh or link manager. Skipping tick."));
		return;
	}
	ApplyMotorForces();
}

void UVehiclePhysicsComponent::ApplyMotorForces()
{
	if (!VehicleMesh) {
		return;
	}
	if (!VehicleMesh->IsSimulatingPhysics()) {
		return;
	}

	for (const FVehicleRotorConfig& CurrentRotor : ActiveRotorConfigs) {
		bool bIsValidConfig = CurrentRotor.RotorBoneName != NAME_None && CurrentRotor.RotorIndex >= 0 && CurrentRotor.RotorIndex < 8;
		if (!bIsValidConfig) {
			UE_LOG(LogTemp, Warning, TEXT("[UVehiclePhysicsComponent::ApplyMotorForces] Invalid rotor config for bone %s"), *CurrentRotor.RotorBoneName.ToString());
			continue;
		}

		float Throttle = 0.f;
		FName Key(*FString::Printf(TEXT("motor_%d"), CurrentRotor.RotorIndex));
		UE_LOG(LogTemp, Warning, TEXT("[UVehiclePhysicsComponent::ApplyMotorForces] We inside cycle apply motor forces"));

		if (MavVehicleLink->GetFactStore()->Actuators.GetValue(Key, Throttle)) {

			float CalculatedForce = Throttle * CurrentRotor.RotorMaxThrustNewtons * 100.0f; // Convert from N to Unreal units (cm*kg/s^2)

			FVector WorldThrustDirection = VehicleMesh->GetComponentTransform().TransformVectorNoScale(CurrentRotor.RotorDirection);
			FVector ForceToApply = WorldThrustDirection * CalculatedForce;
			UE_LOG(LogTemp, Warning, TEXT("[UVehiclePhysicsComponent::ApplyMotorForces] Apply motor forces. %s = %s"), *CurrentRotor.RotorBoneName.ToString(), *ForceToApply.ToString());
			VehicleMesh->AddForce(ForceToApply, CurrentRotor.RotorBoneName, true);
		}
	}
}
