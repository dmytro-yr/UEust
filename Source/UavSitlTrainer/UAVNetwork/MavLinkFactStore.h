// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MavLinkFactStore.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnChanged, float);

struct FMavLinkFact
{
	FName  Name;
	float  Value = 0.f;
	bool   bIsStale = false;
	uint64 Timestamp = false;

	FOnChanged OnChanged;

	void SetValue(float NewValue, uint64 Frame);
	void SetValueSilent(float NewValue, uint64 Frame);
};

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAnyChanged, FName, float);

struct FMavLinkFactCategory
{
	TMap<FName, FMavLinkFact> Facts;

	FOnAnyChanged OnAnyChanged;

	FMavLinkFact& GetOrCreate(FName Key);
	bool		  GetValue(FName Key, float& OutValue) const;
	void		  RefreshStaleFacts();
	void		  SubscribeToChange(FName Key, TFunction<void(float)> Callback);
};

UCLASS()
class UAVSITLTRAINER_API UMavLinkFactStore : public UObject
{
	GENERATED_BODY()
public:
	// Groups per Mavlink message category
	FMavLinkFactCategory Attitude;
	FMavLinkFactCategory Actuators;
	FMavLinkFactCategory Position;
	FMavLinkFactCategory Params;
	FMavLinkFactCategory Status;
	FMavLinkFactCategory Battery;
	FMavLinkFactCategory CmdAck;

	void RefreshAllStaleFacts();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SITL Facts")
	float GetAttitude(FName Key) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SITL Facts")
	float GetParam(FName ParamName) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SITL Facts")
	float GetMotorThrottle(int32 MotorIndex) const;
};
