// Fill out your copyright notice in the Description page of Project Settings.

#include "VehicleTelemetryComponent.h"
#include "Utils/PhysicsUnitsUtils.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

// Sets default values
UVehicleTelemetryComponent::UVehicleTelemetryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void UVehicleTelemetryComponent::BeginPlay()
{
	Super::BeginPlay();
	if (auto OwnerActor = GetOwner()) {
		if (auto SkeletalMesh = OwnerActor->FindComponentByClass<USkeletalMeshComponent>()) {
			VehicleMesh = Cast<UPrimitiveComponent>(SkeletalMesh);
		}
	}
	if (!VehicleMesh) {
		UE_LOG(LogTemp, Warning, TEXT("VehicleTelemetryComponent: Failed to find skeletal mesh"));
	}
	// Wait for AP to send FRAME_CLASS and FRAME_TYPE via MAVLink before enabling physics simulation to prevent potential issues with incorrect frame assumptions in AP
	VehicleMesh->SetSimulatePhysics(false);

	if (auto world = GetWorld()) {
		if (auto gameInstance = world->GetGameInstance()) {
			LinkManager = gameInstance->GetSubsystem<ULinkManagerSubsystem>();
		}
	}
	if (!LinkManager) {
		UE_LOG(LogTemp, Warning, TEXT("VehiclePhysicsComponent: Failed to get LinkManagerSubsystem"));
		return;
	}

	if (LinkManager) {
		LinkManager->OnMavLinkParamReceived.AddDynamic(this, &UVehicleTelemetryComponent::HandleMavLinkParams);
		LinkManager->OnMavLinkAttitudeReceived.AddDynamic(this, &UVehicleTelemetryComponent::HandleMavLinkAttitude);
	}
}

void UVehicleTelemetryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (auto world = GetWorld()) {
		if (auto gameInstance = world->GetGameInstance()) {
			LinkManager = gameInstance->GetSubsystem<ULinkManagerSubsystem>();
		}
	}

	if (!VehicleMesh || !LinkManager) {
		UE_LOG(LogTemp, Warning, TEXT("VehicleTelemetryComponent: Failed to get vehicle mesh or link manager. Skipping tick."));
		return;
	}
	ElapsedTime += DeltaTime;

	FVector RawPos = VehicleMesh->GetComponentLocation();
	FVector RawVel = VehicleMesh->GetPhysicsLinearVelocity();

	FVector RawAngVelRad = VehicleMesh->GetPhysicsAngularVelocityInRadians();
	FQuat	RawQuat = VehicleMesh->GetComponentQuat();

	FVector RawAcc = (RawVel - PrevVelocity) / FMath::Max(DeltaTime, 0.0001f);
	PrevVelocity = RawVel;

	FSitlTelemetry Telemetry = UPhysicsUnitsUtils::ConvertUEToSITL(ElapsedTime, RawPos, RawVel, RawAcc, RawQuat, RawAngVelRad);
	FString		   OutboundJson = UPhysicsUnitsUtils::SerializeTelemetryToJson(Telemetry);
	// UE_LOG(LogTemp, Log, TEXT("[VehicleTelemetryComponent] Vehicle %d sending telemetry: %s"), VehicleId, *OutboundJson);
	LinkManager->PushPhysicsJson(VehicleId, OutboundJson);
}

void UVehicleTelemetryComponent::HandleMavLinkParams(int32 InVehicleId, const FString& ParamName, float ParamValue)
{
	if (VehicleId != InVehicleId) {
		return;
	}
	if (ParamName.Equals(TEXT("FRAME_CLASS"))) {
		UE_LOG(LogTemp, Log, TEXT("[VehicleTelemetryComponent]Vehicle %d received MAVLink param: %s = %f"), VehicleId, *ParamName, ParamValue);
		VehicleMesh->SetSimulatePhysics(true);
	}
}

void UVehicleTelemetryComponent::HandleMavLinkAttitude(int32 InVehicleId, float Roll, float Pitch, float Yaw)
{
	if (VehicleId != InVehicleId) {
		return;
	}
	float	 ArduPilotRoll = FMath::RadiansToDegrees(Roll);
	float	 ArduPilotPitch = FMath::RadiansToDegrees(Pitch);
	float	 ArduPilotYaw = FMath::RadiansToDegrees(Yaw);
	FRotator TrueRotation = VehicleMesh->GetComponentRotation();
	UE_LOG(LogTemp, Log, TEXT("[VehicleTelemetryComponent] Vehicle %d received MAVLink attitude: Roll=%f, Pitch=%f, Yaw=%f. Current Actor Rotation: Roll=%f, Pitch=%f, Yaw=%f"),
		VehicleId, ArduPilotRoll, ArduPilotPitch, ArduPilotYaw, TrueRotation.Roll, TrueRotation.Pitch, TrueRotation.Yaw);
}