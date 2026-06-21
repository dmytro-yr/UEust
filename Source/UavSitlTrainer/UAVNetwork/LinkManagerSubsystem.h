// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "NetworkTypes.h"
#include "LinkManagerSubsystem.generated.h"

class UVehicleLink;

UCLASS()
class UAVSITLTRAINER_API ULinkManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "SITL Network")
	bool ConnectVehicle(const FVehicleNetworkConfig& Config);

	UFUNCTION(BlueprintCallable, Category = "SITL Network")
	void DisconnectVehicle(int32 VehicleId);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SITL")
	bool IsVehicleConnected(int32 VehicleId) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SITL")
	UVehicleLink* GetVehicleLink(int32 VehicleId) const;

private:
	UPROPERTY()
	TMap<int32, TObjectPtr<UVehicleLink>> VehicleLinks;
};
