// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PhysicsUnitsUtils.generated.h"

USTRUCT(BlueprintType)
struct FSitlImu
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "SITL Physics")
	FVector Gyro = FVector::ZeroVector; // roll, pitch, yaw (rad/s)

	UPROPERTY(BlueprintReadWrite, Category = "SITL Physics")
	FVector Accel = FVector::ZeroVector; // x, y, z (m/s^2)
};

USTRUCT(BlueprintType)
struct FSitlTelemetry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "SITL Physics")
	double Timestamp = 0.0;

	UPROPERTY(BlueprintReadWrite, Category = "SITL Physics")
	FSitlImu Imu;

	UPROPERTY(BlueprintReadWrite, Category = "SITL Physics")
	FVector Position = FVector::ZeroVector; // ned (m)

	UPROPERTY(BlueprintReadWrite, Category = "SITL Physics")
	FVector Velocity = FVector::ZeroVector; // ned (m/s)

	UPROPERTY(BlueprintReadWrite, Category = "SITL Physics")
	FVector Acceleration = FVector::ZeroVector; // ned (m/s^2)

	UPROPERTY(BlueprintReadWrite, Category = "SITL Physics")
	FVector Attitude = FVector::ZeroVector; // roll, pitch, yaw (rad)

	UPROPERTY(BlueprintReadWrite, Category = "SITL Physics")
	FVector AngularVelocity = FVector::ZeroVector; // roll, pitch, yaw (rad/s)
};

UCLASS()
class UAVSITLTRAINER_API UPhysicsUnitsUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "SITL Physics")
	static FSitlTelemetry ConvertUEToSITL(
		double		   Timestamp,
		const FVector& UEPosition,
		const FVector& UEVelocity,
		const FVector& UEAcceleration,
		const FQuat&   UE5Quaternion,
		const FVector& UEAngularVelocity); // cartesian to ned conversion

	static FString SerializeTelemetryToJson(const FSitlTelemetry& Telemetry);
};
