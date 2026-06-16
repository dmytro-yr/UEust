// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VehiclePhysicsComponent.generated.h"

class ULinkManagerSubsystem;
class UVehicleLink;
class UPrimitiveComponent;

// Layout config which matching bones to network
USTRUCT(BlueprintType)
struct FVehicleRotorConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SITL Config")
	FName RotorBoneName = NAME_None;

	// index in the motor throttles array, should be 0-7 for octocopter
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SITL Config")
	int32 RotorIndex = 0;

	// direction of the rotor, used for calculating torque direction. Should be either up or down vector.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SITL Config")
	FVector RotorDirection = FVector::DownVector;

	/*
	Max thrust per motor in newtons, used for converting throttle to force.
	Can be overridden by the global max motor thrust if set to 0 or negative.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SITL Config")
	float RotorMaxThrustNewtons = 25.0f;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent), BlueprintType)
class UAVSITLTRAINER_API UVehiclePhysicsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVehiclePhysicsComponent();
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void UpdateMotorThrottles(const float InThrottles[8]);

private:
	void ApplyMotorForces();

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
	// Active automated rotor configurations
	UPROPERTY(EditAnywhere, Category = "SITL Physics")
	TArray<FVehicleRotorConfig> ActiveRotorConfigs;
};