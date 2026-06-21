// Fill out your copyright notice in the Description page of Project Settings.

#include "UnifiedFlightController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"

AUnifiedFlightController::AUnifiedFlightController()
{
	bShowMouseCursor = true;
}

void AUnifiedFlightController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (!DefaultInputMappingContext) {
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());

	if (Subsystem) {
		Subsystem->AddMappingContext(DefaultInputMappingContext, 0);
		UE_LOG(LogTemp, Display, TEXT("[AUnifiedFlightController::SetupInputComponent] Input mapping context added"));
	}

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent)) {
		EnhancedInputComponent->BindAction(SelectAction, ETriggerEvent::Completed, this, &AUnifiedFlightController::Select);
	}
}

void AUnifiedFlightController::Select(const FInputActionValue& Value)
{

	FHitResult HitResult;
	GetHitResultUnderCursor(ECollisionChannel::ECC_Camera, false, HitResult);

	AActor* SelectedVehicle = HitResult.GetActor();

	if (SelectedVehicle) {
		UE_LOG(LogTemp, Display, TEXT("[AUnifiedFlightController::SelectAction] You select Vehicle %s"), *SelectedVehicle->GetName());
	}
}
