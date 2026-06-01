// Fill out your copyright notice in the Description page of Project Settings.

#include "Utils/PhysicsUnitsUtils.h"
#include "PhysicsUnitsUtils.h"

FSitlTelemetry UPhysicsUnitsUtils::ConvertUEToSITL(
	double		   Timestamp,
	const FVector& UEPosition,
	const FVector& UEVelocity,
	const FVector& UEAcceleration,
	const FQuat&   UE5Quaternion,
	const FVector& UEAngularVelocity)
{
	FSitlTelemetry OutTelemetry;
	OutTelemetry.Timestamp = Timestamp;
	// UE5 to NED conversion with unit adjustments
	// position (cm->m)
	OutTelemetry.Position = UEPosition * 0.01f;
	OutTelemetry.Position.Z = -OutTelemetry.Position.Z; // invert z for ned

	// velocity (cm/s -> m/s)(-z)
	OutTelemetry.Velocity = UEVelocity * 0.01f;
	OutTelemetry.Velocity.Z = -OutTelemetry.Velocity.Z; // invert z for ned

	// acceleration (cm/s^2 -> m/s^2)(-z)
	OutTelemetry.Acceleration = UEAcceleration * 0.01f;
	OutTelemetry.Acceleration.Z = -OutTelemetry.Acceleration.Z; // invert z for ned

	FQuat	 NedQuat = FQuat(UE5Quaternion.X, UE5Quaternion.Y, -UE5Quaternion.Z, -UE5Quaternion.W); // convert to ned by inverting z and w
	FRotator NedRotator = NedQuat.Rotator();

	// attitude (roll, pitch, yaw) (deg -> rad)(-pitch, -yaw)
	OutTelemetry.Attitude.X = FMath::DegreesToRadians(NedRotator.Roll);	 // roll
	OutTelemetry.Attitude.Y = FMath::DegreesToRadians(NedRotator.Pitch); // pitch
	OutTelemetry.Attitude.Z = FMath::DegreesToRadians(NedRotator.Yaw);	 // yaw

	// angular velocity (roll, pitch, yaw) (deg/s -> rad/s)(-pitch, -yaw)
	OutTelemetry.AngularVelocity.X = FMath::DegreesToRadians(-UEAngularVelocity.X); // roll
	OutTelemetry.AngularVelocity.Y = FMath::DegreesToRadians(-UEAngularVelocity.Y); // pitch
	OutTelemetry.AngularVelocity.Z = FMath::DegreesToRadians(UEAngularVelocity.Z);	// yaw

	return OutTelemetry;
}

FString UPhysicsUnitsUtils::SerializeTelemetryToJson(const FSitlTelemetry& Telemetry)
{
	FString JsonString;
	JsonString.Reserve(512);
	JsonString.Appendf(TEXT("{\"timestamp\":%.4f,"), Telemetry.Timestamp);
	JsonString.Appendf(TEXT("\"position\":[%.4f,%.4f,%.4f],"), Telemetry.Position.X, Telemetry.Position.Y, Telemetry.Position.Z);
	JsonString.Appendf(TEXT("\"velocity\":[%.4f,%.4f,%.4f],"), Telemetry.Velocity.X, Telemetry.Velocity.Y, Telemetry.Velocity.Z);
	JsonString.Appendf(TEXT("\"acceleration\":[%.4f,%.4f,%.4f],"), Telemetry.Acceleration.X, Telemetry.Acceleration.Y, Telemetry.Acceleration.Z);
	JsonString.Appendf(TEXT("\"attitude\":[%.4f,%.4f,%.4f],"), Telemetry.Attitude.X, Telemetry.Attitude.Y, Telemetry.Attitude.Z);
	JsonString.Appendf(TEXT("\"angular_velocity\":[%.4f,%.4f,%.4f]}"), Telemetry.AngularVelocity.X, Telemetry.AngularVelocity.Y, Telemetry.AngularVelocity.Z);

	return JsonString;
}
