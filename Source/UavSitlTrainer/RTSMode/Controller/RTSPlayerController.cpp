// Fill out your copyright notice in the Description page of Project Settings.

#include "RTSPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"

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

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent)) {
		EnhancedInputComponent->BindAction(SelectAction, ETriggerEvent::Completed, this, &ARTSPlayerController::Select);
	}
}

void ARTSPlayerController::Select(const FInputActionValue& Value)
{

	FHitResult HitResult;
	GetHitResultUnderCursor(ECollisionChannel::ECC_Camera, false, HitResult);

	AActor* SelectedVehicle = HitResult.GetActor();

	if (SelectedVehicle) {
		UE_LOG(LogTemp, Display, TEXT("[ARTSPlayerController::SelectAction] You select Vehicle %s"), *SelectedVehicle->GetName());
	}
}
