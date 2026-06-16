// Fill out your copyright notice in the Description page of Project Settings.

#include "MavLinkFactStore.h"

void FMavLinkFact::SetValue(float NewValue, uint64 Frame)
{ // value should changed with precision 0.00001
	if (FMath::IsNearlyEqual(Value, NewValue, 1e-5f)) {
		return;
	}
	Value = NewValue;
	bIsStale = true;
	Timestamp = Frame;
	OnChanged.Broadcast(NewValue);
}

void FMavLinkFact::SetValueSilent(float NewValue, uint64 Frame)
{
	if (FMath::IsNearlyEqual(Value, NewValue, 1e-5f)) {
		return;
	}
	Value = NewValue;
	bIsStale = true;
	Timestamp = Frame;
}

FMavLinkFact& FMavLinkFactCategory::GetOrCreate(FName Key)
{
	FMavLinkFact* Fact = Facts.Find(Key);
	if (Fact) {
		return *Fact;
	}

	FMavLinkFact& NewFact = Facts.Add(Key);
	NewFact.Name = Key;
	return NewFact;
}

bool FMavLinkFactCategory::GetValue(FName Key, float& OutValue) const
{
	if (const FMavLinkFact* Fact = Facts.Find(Key)) {
		OutValue = Fact->Value;
		return true;
	}

	return false;
}

void FMavLinkFactCategory::RefreshStaleFacts()
{
	for (auto& [Key, Fact] : Facts) {
		if (Fact.bIsStale) {
			Fact.OnChanged.Broadcast(Fact.Value);
			OnAnyChanged.Broadcast(Key, Fact.Value);
			Fact.bIsStale = false;
		}
	}
}

void FMavLinkFactCategory::SubscribeToChange(FName Key, TFunction<void(float)> Callback)
{
	GetOrCreate(Key).OnChanged.AddLambda(MoveTemp(Callback));
}

void UMavLinkFactStore::RefreshAllStaleFacts()
{
	Attitude.RefreshStaleFacts();
	Actuators.RefreshStaleFacts();
	Position.RefreshStaleFacts();
	Params.RefreshStaleFacts();
	Status.RefreshStaleFacts();
	Battery.RefreshStaleFacts();
	CmdAck.RefreshStaleFacts();
}

float UMavLinkFactStore::GetAttitude(FName Key) const
{
	return 0.f;
}

float UMavLinkFactStore::GetParam(FName ParamName) const
{
	return 0.f;
}

float UMavLinkFactStore::GetMotorThrottle(int32 MotorIndex) const
{
	return 0.f;
}