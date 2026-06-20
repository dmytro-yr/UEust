// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "VehiclePawn.generated.h"

class UVehicleLink;

UCLASS()
class UAVSITLTRAINER_API AVehiclePawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AVehiclePawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	const TObjectPtr<UVehicleLink>& GetVehicleLinkPtr() const { return VehicleLinkPtr; };
	int32							GetVehicleId() const { return VehicleId; };

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle", meta = (ExposeOnSpawn = "true"))
	int32 VehicleId = INDEX_NONE;

private:
	UPROPERTY(Transient)
	TObjectPtr<UVehicleLink> VehicleLinkPtr = nullptr;
};
