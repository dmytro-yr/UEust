// Fill out your copyright notice in the Description page of Project Settings.

#include "Vehicle/VehiclePawn.h"
#include "LinkManagerSubsystem.h"
#include "UAVNetwork/VehicleLink.h"

// Sets default values
AVehiclePawn::AVehiclePawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AVehiclePawn::BeginPlay()
{
	const auto* World = GetWorld();

	if (!World) {
		return;
	}

	if (const UGameInstance* GI = World->GetGameInstance()) {
		auto LinkManager = GI->GetSubsystem<ULinkManagerSubsystem>();
		if (!LinkManager) {
			UE_LOG(LogTemp, Warning, TEXT("[AVehiclePawn::BeginPlay] Failed to get LinkManagerSubsystem"));
			return;
		}

		VehicleLinkPtr = LinkManager->GetVehicleLink(VehicleId);
	}

	if (!VehicleLinkPtr) {
		UE_LOG(LogTemp, Error, TEXT("[AVehiclePawn::BeginPlay] No VehicleLink for id %d"), VehicleId)
	}

	Super::BeginPlay();
}

// Called every frame
void AVehiclePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AVehiclePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}
