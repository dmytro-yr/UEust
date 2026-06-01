// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LinkManagerSubsystem.h"
#include "VehiclePhysicsComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent), BlueprintType)
class UAVSITLTRAINER_API UVehiclePhysicsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	UVehiclePhysicsComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, Category = "SITL Network Config")
	int32 VehicleId = 0;

private:
	UPROPERTY()
	ULinkManagerSubsystem* LinkManager = nullptr;

	double	ElapsedTime = 0.0;
	FVector PrevVelocity = FVector::ZeroVector;
};