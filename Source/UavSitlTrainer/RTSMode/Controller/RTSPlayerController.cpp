// Fill out your copyright notice in the Description page of Project Settings.

#include "RTSPlayerController.h"
#include "RTSMode/Controller/RTSPlayerController.h"
#include "InputMappingContext.h"
#include "EnhancedInputSubsystems.h"

ARTSPlayerController::ARTSPlayerController()
{
	bShowMouseCursor = true;
}

void ARTSPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (!DefaultInputMappingContext) {
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());

	if (Subsystem) {
		Subsystem->AddMappingContext(DefaultInputMappingContext, 0);
		UE_LOG(LogTemp, Display, TEXT("[ARTSPlayerController::SetupInputComponent] Input mapping context added"));
	}
}
