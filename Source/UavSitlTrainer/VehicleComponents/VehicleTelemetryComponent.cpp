// Fill out your copyright notice in the Description page of Project Settings.

#include "VehicleTelemetryComponent.h"
#include "Utils/PhysicsUnitsUtils.h"
#include "Engine/World.h"
#include "LinkManagerSubsystem.h"
#include "GameFramework/Actor.h"
#include "UAVNetwork/VehicleLink.h"
#include "UAVNetwork/OutboundDispatcher.h"

// Sets default values
UVehicleTelemetryComponent::UVehicleTelemetryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void UVehicleTelemetryComponent::BeginPlay()
{
	Super::BeginPlay();
	if (const auto* OwnerActor = GetOwner()) {
		if (auto* SkeletalMesh = OwnerActor->FindComponentByClass<USkeletalMeshComponent>()) {
			VehicleMesh = Cast<UPrimitiveComponent>(SkeletalMesh);
		}
	}

	if (!VehicleMesh) {
		UE_LOG(LogTemp, Warning, TEXT("[VehicleTelemetryComponent::BeginPlay] Failed to find skeletal mesh"));
	}
	// Wait for AP to send FRAME_CLASS and FRAME_TYPE via MAVLink before enabling physics simulation to prevent potential issues with incorrect frame assumptions in AP
	VehicleMesh->SetSimulatePhysics(false);

	if (const auto* World = GetWorld()) {
		if (const UGameInstance* GI = World->GetGameInstance()) {
			LinkManager = GI->GetSubsystem<ULinkManagerSubsystem>();
		}
	}

	if (!LinkManager) {
		UE_LOG(LogTemp, Warning, TEXT("[VehicleTelemetryComponent::BeginPlay] Failed to get LinkManagerSubsystem"));
		return;
	}

	VehicleLink = LinkManager->GetVehicleLink(VehicleId);
	if (!VehicleLink) {
		UE_LOG(LogTemp, Error, TEXT("[VehicleTelemetryComponent::BeginPlay] No VehicleLink for id %d"), VehicleId);
	}
}

void UVehicleTelemetryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ElapsedTime += DeltaTime;

	FVector RawPos = VehicleMesh->GetComponentLocation();
	FVector RawVel = VehicleMesh->GetPhysicsLinearVelocity();

	FVector RawAngVelRad = VehicleMesh->GetPhysicsAngularVelocityInRadians();
	FQuat	RawQuat = VehicleMesh->GetComponentQuat();

	FVector RawAcc = (RawVel - PrevVelocity) / FMath::Max(DeltaTime, 0.0001f);
	PrevVelocity = RawVel;

	FSitlTelemetry Telemetry = UPhysicsUnitsUtils::ConvertUEToSITL(ElapsedTime, RawPos, RawVel, RawAcc, RawQuat, RawAngVelRad);
	FString		   OutboundJson = UPhysicsUnitsUtils::SerializeTelemetryToJson(Telemetry);
	if (VehicleLink && VehicleLink->GetOutboundDispatcher()) {
		VehicleLink->GetOutboundDispatcher()->Enqueue(OutboundJson);
	}
}