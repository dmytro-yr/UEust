// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LinkManagerSubsystem.h"
#include "VehiclePhysicsComponent.generated.h"

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
	// Sets default values for this actor's properties
	UVehiclePhysicsComponent();
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void UpdateMotorThrottles(const float InThrottles[8]);
	// TODO: Add API to accept automated Rotor configuration
	// void ConfigureRotors(const TArray<FVehicleRotorConfig>& RotorConfigs);

	UFUNCTION()
	void HandleIncomingThrottles(int32 InVehicleId, const TArray<float>& NormalizedThrottles);

private:
	void ApplyMotorForces();

private:
	UPROPERTY(EditAnywhere, Category = "SITL Network Config")
	int32 VehicleId = 0;

	UPROPERTY()
	ULinkManagerSubsystem* LinkManager = nullptr;

	float MotorThrottles[8] = { 0 };

	UPROPERTY()
	UPrimitiveComponent* VehicleMesh = nullptr;

	// Active automated rotor configurations
	UPROPERTY(EditAnywhere, Category = "SITL Physics")
	TArray<FVehicleRotorConfig> ActiveRotorConfigs;
};