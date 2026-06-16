// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MavLinkHandlers/MavHandlerBase.h"
#include "NetworkTypes.h"
#include "MavLinkFactStore.h"

#include "MavLinkDispatcher.generated.h"

UCLASS()
class UAVSITLTRAINER_API UMavLinkDispatcher : public UObject
{

	GENERATED_BODY()

public:
	void Initialize(FNetworkChannels* InNetworkChannels, UMavLinkFactStore* FactStore);
	void RegisterHandler(uint32 MsgId, TUniquePtr<FMavHandlerBase> Handler);
	void ReceiveFacts(uint64 FrameCount);

private:
	UPROPERTY()
	TObjectPtr<UMavLinkFactStore>			  FactStore;
	FNetworkChannels*						  NetworkChannels = nullptr;
	TMap<uint32, TUniquePtr<FMavHandlerBase>> HandlerMap;
};
