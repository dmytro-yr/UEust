// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ReceiveHandlers/HandlerBase.h"
#include "NetworkTypes.h"
#include "MavLinkFactStore.h"

#include "InboundDispatcher.generated.h"

UCLASS()
class UAVSITLTRAINER_API UInboundDispatcher : public UObject
{

	GENERATED_BODY()

public:
	void Initialize(FNetworkChannels* InNetworkChannels, UMavLinkFactStore* FactStore);
	void RegisterMavHandler(uint32 MsgId, TUniquePtr<FMavHandlerBase> Handler);
	void RegisterRawFrameHandler(uint32 MsgId, TUniquePtr<FRawFrameHandlerBase> Handler);
	void ReceiveFacts(uint64 FrameCount);

private:
	UPROPERTY()
	TObjectPtr<UMavLinkFactStore>				   FactStore;
	FNetworkChannels*							   NetworkChannels = nullptr;
	TMap<uint32, TUniquePtr<FMavHandlerBase>>	   MavHandlerMap;
	TMap<uint32, TUniquePtr<FRawFrameHandlerBase>> RawFrameHandlerMap;
};
