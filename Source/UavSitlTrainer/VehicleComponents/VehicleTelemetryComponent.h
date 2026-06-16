// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VehicleTelemetryComponent.generated.h"

class ULinkManagerSubsystem;
class UVehicleLink;
class UPrimitiveComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent), BlueprintType)
class UAVSITLTRAINER_API UVehicleTelemetryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVehicleTelemetryComponent();
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SITL Network Config")
	int32 VehicleId = 1;

private:
	UPROPERTY()
	TObjectPtr<ULinkManagerSubsystem> LinkManager = nullptr;

	UPROPERTY()
	TObjectPtr<UVehicleLink> VehicleLink = nullptr;

	UPROPERTY()
	TObjectPtr<UPrimitiveComponent> VehicleMesh = nullptr;

	double	ElapsedTime = 0.0;
	FVector PrevVelocity = FVector::ZeroVector;
};