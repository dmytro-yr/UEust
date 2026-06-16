// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SitlNetwork/MavLinkIncludes.h"
#include "MavLinkHeartbeatComponent.generated.h"

class UMavVehicleLink;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMavLinkConnectionStatusChanged, bool, bIsConnected);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UAVSITLTRAINER_API UMavLinkHeartbeatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMavLinkHeartbeatComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SITL Network Config")
	int32 VehicleId = 1;

	UPROPERTY(BlueprintAssignable, Category = "SITL Network Config")
	FOnMavLinkConnectionStatusChanged OnConnectionStatusChanged;

	UPROPERTY(BlueprintReadOnly, Category = "SITL Network Config")
	bool bIsConnected = false;

private:
	void HandleConnectionTimeout();
	void SendHeartbeat();
	void OnStatusFactChanged(FName Key, float Value);

private:
	UPROPERTY(Transient)
	TObjectPtr<UMavVehicleLink> MavVehicleLink;

	FTimerHandle HeartbeatTimer;
	FTimerHandle TimeoutTimer;
	const float	 HeartbeatInterval = 1.0f;
	const float	 ConnectionTimeout = 3.0f;
};