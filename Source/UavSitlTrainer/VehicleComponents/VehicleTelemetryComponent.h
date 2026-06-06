// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LinkManagerSubsystem.h"
#include "VehicleTelemetryComponent.generated.h"

class UPrimitiveComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent), BlueprintType)
class UAVSITLTRAINER_API UVehicleTelemetryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	UVehicleTelemetryComponent();
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UFUNCTION()
	void HandleMavLinkParams(int32 InVehicleId, const FString& ParamName, float ParamValue);
	UFUNCTION()
	void HandleMavLinkAttitude(int32 InVehicleId, float Roll, float Pitch, float Yaw);

public:
	UPROPERTY(EditAnywhere, Category = "SITL Network Config")
	int32 VehicleId = 0;

private:
	UPROPERTY()
	ULinkManagerSubsystem* LinkManager = nullptr;

	UPROPERTY()
	UPrimitiveComponent* VehicleMesh = nullptr;

	double	ElapsedTime = 0.0;
	FVector PrevVelocity = FVector::ZeroVector;
};