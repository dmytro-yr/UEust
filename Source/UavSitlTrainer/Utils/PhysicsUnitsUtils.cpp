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

	// =========================================================================
	// 1. WORLD SPACE TRANSFORMS (Unreal Left-Handed -> NED Right-Handed)
	// =========================================================================
	// Unreal: +X=North, +Y=East, +Z=Up (cm)
	// NED:    +X=North, +Y=East, +Z=Down (m)
	OutTelemetry.Position = FVector(UEPosition.X * 0.01f, UEPosition.Y * 0.01f, -UEPosition.Z * 0.01f);
	OutTelemetry.Velocity = FVector(UEVelocity.X * 0.01f, UEVelocity.Y * 0.01f, -UEVelocity.Z * 0.01f);
	OutTelemetry.Acceleration = FVector(UEAcceleration.X * 0.01f, UEAcceleration.Y * 0.01f, -UEAcceleration.Z * 0.01f);

	// =========================================================================
	// 2. VEHICLE ATTITUDE (Unreal Deg -> NED Radians Aerospace Standard)
	// =========================================================================
	FRotator UERotator = UE5Quaternion.Rotator();
	OutTelemetry.Attitude.X = FMath::DegreesToRadians(UERotator.Roll);	 // Roll (Same)
	OutTelemetry.Attitude.Y = FMath::DegreesToRadians(-UERotator.Pitch); // Pitch (Inverted)
	OutTelemetry.Attitude.Z = FMath::DegreesToRadians(-UERotator.Yaw);	 // Yaw (Inverted)

	// =========================================================================
	// 3. GYRO SENSOR (Left-Handed Global -> Right-Handed Local Body Frame)
	// =========================================================================
	// Un-rotate the global tracking velocity into the vehicle's relative local framework
	FVector LocalAngVelRad = UE5Quaternion.UnrotateVector(UEAngularVelocity);

	// Map to aerospace body rates: p (roll rate), q (pitch rate), r (yaw rate)
	OutTelemetry.Imu.Gyro.X = LocalAngVelRad.X;	 // p (Roll Rate)
	OutTelemetry.Imu.Gyro.Y = -LocalAngVelRad.Y; // q (Pitch Rate - Inverted for Right-Handed Body)
	OutTelemetry.Imu.Gyro.Z = -LocalAngVelRad.Z; // r (Yaw Rate - Inverted for Right-Handed Body)

	// =========================================================================
	// 4. ACCELEROMETER SENSOR (Proper Acceleration with Gravity Compensation)
	// =========================================================================
	// Kinematic acceleration converted to meters
	FVector NedWorldAcc = OutTelemetry.Acceleration;

	// FIX: Inject Earth Gravity. Accelerometers read upwards acceleration relative to freefall.
	// When at rest, an IMU experiences 1G (+9.80665 m/s^2) pulling UP away from the core of the earth.
	// In NED coordinates, "Up" is the NEGATIVE Z direction.
	FVector NedWorldAccWithGravity = NedWorldAcc + FVector(0.0f, 0.0f, -9.80665f);

	// Convert our specific vehicle rotation context cleanly into the destination coordinate space
	FQuat NedOrientation = FQuat::MakeFromRotator(FRotator(-UERotator.Pitch, -UERotator.Yaw, UERotator.Roll));

	// Un-rotate the combined world vector into the aircraft body frame
	FVector LocalAccBody = NedOrientation.UnrotateVector(NedWorldAccWithGravity);

	// ArduPilot's JSON model expects "accel_body" to explicitly include gravity offsets
	OutTelemetry.Imu.Accel.X = LocalAccBody.X;
	OutTelemetry.Imu.Accel.Y = LocalAccBody.Y;
	OutTelemetry.Imu.Accel.Z = -LocalAccBody.Z; // Match right-handed sensor direction layout

	// General backward compatibility link
	OutTelemetry.AngularVelocity = OutTelemetry.Imu.Gyro;

	return OutTelemetry;
}

FString UPhysicsUnitsUtils::SerializeTelemetryToJson(const FSitlTelemetry& Telemetry)
{
	FString JsonString;
	JsonString.Reserve(256); // Smaller footprint now that it's clean

	// Mandatory Field 1: Absolute Physics Timestamp
	JsonString.Appendf(TEXT("{\"timestamp\":%.2f,"), Telemetry.Timestamp);

	// Mandatory Field 2: Nested IMU containing gyro and accel_body
	JsonString.Appendf(TEXT("\"imu\":{\"gyro\":[%.3f,%.3f,%.3f],\"accel_body\":[%.3f,%.3f,%.3f]},"),
		Telemetry.Imu.Gyro.X, Telemetry.Imu.Gyro.Y, Telemetry.Imu.Gyro.Z,
		Telemetry.Imu.Accel.X, Telemetry.Imu.Accel.Y, Telemetry.Imu.Accel.Z);

	// Mandatory Field 3: Global World Position (NED)
	JsonString.Appendf(TEXT("\"position\":[%.3f,%.3f,%.3f],"), Telemetry.Position.X, Telemetry.Position.Y, Telemetry.Position.Z);

	// Mandatory Field 5: Vehicle Orientation Attitude (Roll, Pitch, Yaw in Radians)
	JsonString.Appendf(TEXT("\"attitude\":[%.3f,%.3f,%.3f],"), Telemetry.Attitude.X, Telemetry.Attitude.Y, Telemetry.Attitude.Z);

	// Mandatory Field 4: Global World Velocity (NED)
	JsonString.Appendf(TEXT("\"velocity\":[%.3f,%.3f,%.3f]}"), Telemetry.Velocity.X, Telemetry.Velocity.Y, Telemetry.Velocity.Z);

	return JsonString;
}
