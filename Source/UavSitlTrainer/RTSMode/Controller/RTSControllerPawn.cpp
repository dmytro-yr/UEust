// Fill out your copyright notice in the Description page of Project Settings.
#include "RTSControllerPawn.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "EnhancedInputComponent.h"

// Sets default values
ARTSControllerPawn::ARTSControllerPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	RootComponent = CapsuleComponent;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));

	SpringArm->SetupAttachment(RootComponent);
	Camera->SetupAttachment(SpringArm);
	Camera->SetProjectionMode(ECameraProjectionMode::Orthographic);

	FloatingPawnMovement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("FloatingPawnMovement"));
}

// Called when the game starts or when spawned
void ARTSControllerPawn::BeginPlay()
{
	Super::BeginPlay();
}

void ARTSControllerPawn::Move(const FInputActionValue& Value)
{
	const FVector2D MovementInput = Value.Get<FVector2D>();
	if (!Controller) {
		return;
	}
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(Forward, MovementInput.Y);
	AddMovementInput(Right, MovementInput.X);

} // Called every frame

void ARTSControllerPawn::Zoom(const FInputActionValue& Value)
{
	const float ZoomDirection = Value.Get<float>();
	if (!Controller) {
		return;
	}
	float DesiredOrthoWidth = Camera->OrthoWidth + ZoomDirection * CameraZoomSpeed;
	DesiredOrthoWidth = FMath::Clamp(DesiredOrthoWidth, MinCameraZoom, MaxCameraZoom);
	Camera->OrthoWidth = DesiredOrthoWidth;
}

void ARTSControllerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ARTSControllerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EnhancedInputComponent) {
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ARTSControllerPawn::Move);
		EnhancedInputComponent->BindAction(ZoomAction, ETriggerEvent::Triggered, this, &ARTSControllerPawn::Zoom);
	}
}