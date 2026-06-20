// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VehicleTelemetryComponent.generated.h"

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

private:
	UPROPERTY()
	TObjectPtr<UVehicleLink> VehicleLinkPtr = nullptr;

	UPROPERTY()
	TObjectPtr<UPrimitiveComponent> VehicleMeshPtr = nullptr;

	double	ElapsedTime = 0.0;
	FVector PrevVelocity = FVector::ZeroVector;
};