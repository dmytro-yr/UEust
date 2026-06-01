// Fill out your copyright notice in the Description page of Project Settings.

#include "VehiclePhysicsComponent.h"
#include "Utils/PhysicsUnitsUtils.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

// Sets default values
UVehiclePhysicsComponent::UVehiclePhysicsComponent()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void UVehiclePhysicsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (auto world = GetWorld()) {
		if (auto gameInstance = world->GetGameInstance()) {
			LinkManager = gameInstance->GetSubsystem<ULinkManagerSubsystem>();
		}
	}

	UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(GetOwner()->GetComponentByClass(UPrimitiveComponent::StaticClass()));
	if (!RootPrimitive || !LinkManager) {
		UE_LOG(LogTemp, Warning, TEXT("VehiclePhysicsComponent: Failed to get root primitive component or link manager. Skipping tick."));
		return;
	}
	ElapsedTime += DeltaTime;

	FVector RawPos = RootPrimitive->GetComponentLocation();
	FVector RawVel = RootPrimitive->GetPhysicsLinearVelocity();

	FVector RawAngVelRad = RootPrimitive->GetPhysicsAngularVelocityInRadians();
	FQuat	RawQuat = RootPrimitive->GetComponentQuat();

	FVector RawAcc = (RawVel - PrevVelocity) / FMath::Max(DeltaTime, 0.0001f);
	PrevVelocity = RawVel;

	FSitlTelemetry Telemetry = UPhysicsUnitsUtils::ConvertUEToSITL(ElapsedTime, RawPos, RawVel, RawAcc, RawQuat, RawAngVelRad);
	FString		   OutboundJson = UPhysicsUnitsUtils::SerializeTelemetryToJson(Telemetry);
	if (LinkManager) {
		LinkManager->TestPushPhysicsString(VehicleId, OutboundJson);
		LinkManager->FlushInboundGameThreadQueues();
	}
} // Called when the game starts or when spawned
