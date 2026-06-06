// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SitlNetwork/MavLinkIncludes.h"
#include "MavLinkHeartbeatComponent.generated.h"

class ULinkManagerSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMavLinkConnectionStatusChanged, bool, bIsConnected);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UAVSITLTRAINER_API UMavLinkHeartbeatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UMavLinkHeartbeatComponent();
	void HandleIncomingHeartbeat(int32 InVehicleId, const mavlink_heartbeat_t& DecodedHeartbeat);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void SendHeartbeat();

	void HandleConnectionTimeout();

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SITL Network Config")
	int32 VehicleId = 0;

	UPROPERTY(BlueprintAssignable, Category = "SITL Network Config")
	FOnMavLinkConnectionStatusChanged OnConnectionStatusChanged;

	UPROPERTY(BlueprintReadOnly, Category = "SITL Network Config")
	bool bIsConnected = false;

private:
	FTimerHandle HeartbeatSendTimer;
	FTimerHandle TimeoutTimer;
	const float	 HeartbeatInterval = 1.0f;
	const float	 ConnectionTimeout = 3.0f;

	UPROPERTY(Transient)
	TObjectPtr<ULinkManagerSubsystem> CachedLinkManager;
};