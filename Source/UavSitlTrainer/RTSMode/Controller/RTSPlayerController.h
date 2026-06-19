// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RTSPlayerController.generated.h"

class UInputMappingContext;

UCLASS()
class UAVSITLTRAINER_API ARTSPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	ARTSPlayerController();

	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAcces = "true"))
	UInputMappingContext* DefaultInputMappingContext;

protected:
	virtual void SetupInputComponent() override;
};
