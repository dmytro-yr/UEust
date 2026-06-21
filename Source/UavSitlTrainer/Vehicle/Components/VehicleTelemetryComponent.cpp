// Fill out your copyright notice in the Description page of Project Settings.

#include "VehicleTelemetryComponent.h"
#include "Utils/PhysicsUnitsUtils.h"
#include "Engine/World.h"
#include "Vehicle/VehiclePawn.h"
#include "UAVNetwork/VehicleLink.h"
#include "UAVNetwork/Dispatchers/OutboundDispatcher.h"

// Sets default values
UVehicleTelemetryComponent::UVehicleTelemetryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void UVehicleTelemetryComponent::BeginPlay()
{
	Super::BeginPlay();
	const auto* VehiclePawn = Cast<AVehiclePawn>(GetOwner());
	if (!VehiclePawn) {
		UE_LOG(LogTemp, Error, TEXT("[VehicleTelemetryComponent::BeginPlay] No VehiclePawn."));
		return;
	}

	VehicleLinkPtr = VehiclePawn->GetVehicleLinkPtr();
	int32 VehicleId = VehiclePawn->GetVehicleId();
	if (!VehicleLinkPtr) {
		UE_LOG(LogTemp, Error, TEXT("[VehicleTelemetryComponent::BeginPlay] No VehicleLink for id %d"), VehicleId);
		return;
	}

	if (auto* SkeletalMesh = VehiclePawn->FindComponentByClass<USkeletalMeshComponent>()) {
		VehicleMeshPtr = Cast<UPrimitiveComponent>(SkeletalMesh);
	}

	if (!VehicleMeshPtr) {
		UE_LOG(LogTemp, Warning, TEXT("[VehicleTelemetryComponent::BeginPlay] Failed to find skeletal mesh"));
		return;
	}
	// Wait for AP to send FRAME_CLASS and FRAME_TYPE via MAVLink before enabling physics simulation to prevent potential issues with incorrect frame assumptions in AP
	VehicleMeshPtr->SetSimulatePhysics(false);
}

void UVehicleTelemetryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ElapsedTime += DeltaTime;

	FVector RawPos = VehicleMeshPtr->GetComponentLocation();
	FVector RawVel = VehicleMeshPtr->GetPhysicsLinearVelocity();

	FVector RawAngVelRad = VehicleMeshPtr->GetPhysicsAngularVelocityInRadians();
	FQuat	RawQuat = VehicleMeshPtr->GetComponentQuat();

	FVector RawAcc = (RawVel - PrevVelocity) / FMath::Max(DeltaTime, 0.0001f);
	PrevVelocity = RawVel;

	FSitlTelemetry Telemetry = UPhysicsUnitsUtils::ConvertUEToSITL(ElapsedTime, RawPos, RawVel, RawAcc, RawQuat, RawAngVelRad);
	FString		   OutboundJson = UPhysicsUnitsUtils::SerializeTelemetryToJson(Telemetry);
	if (VehicleLinkPtr && VehicleLinkPtr->GetOutboundDispatcher()) {
		VehicleLinkPtr->GetOutboundDispatcher()->Enqueue(OutboundJson);
	}
}