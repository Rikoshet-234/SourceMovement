// Originally based off https://github.com/ReKTeK/Defragr
// Original copyright:
// 
/* Copyright (C) Terence-Lee 'Zinglish' Davis
 * Written by Terence-Lee 'Zinglish' Davis <zinglish[at]gmail.com>
 */
#include "SourcePlayer.h"
#include "SourcePlayerMovement.h"
#include "Engine.h"

ASourceMovementPlayer::ASourceMovementPlayer() {
	CrouchEyeHeight = 53.34f; /* 28 Source Units */
	BaseEyeHeight = 122.f; /* 64 Source Units */
	
	CrouchCollisionBoxHeight = 68.58f;  /* 36 Source Units */
	CollisionBoxHeight = 137.16f; /* 72 Source Units */

   // Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    // Set this pawn to be controlled by the lowest-numbered player
    AutoPossessPlayer = EAutoReceiveInput::Player0;

    CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision Box"));
    CollisionBox->InitBoxExtent(FVector(60.95f /* 32 Source Units */, 60.95f /* 32 Source Units */, CollisionBoxHeight));
    
    // Create a dummy root component we can attach things to.  
	SetRootComponent(CollisionBox);

    CurrentCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person View"));
    CurrentCamera->SetupAttachment(RootComponent);
    CurrentCamera->SetRelativeLocation(FVector(0.f, 0.f, BaseEyeHeight)); // Assuming relative location starts from 0.
	// Enables Mouse Look through APawn's AController
	CurrentCamera->bUsePawnControlRotation = true;

	PlayerMovement = CreateDefaultSubobject<USourcePlayerMovement>(TEXT("SourceMovement Movement"));
	PlayerMovement->SetUpdatedComponent(CollisionBox);

}

// Called when the game starts or when spawned
void ASourceMovementPlayer::BeginPlay() {
	Super::BeginPlay();
	
}

// Called every frame
void ASourceMovementPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ASourceMovementPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Respond every frame to the values of our two movement axes, "MoveX" and "MoveY".
	InputComponent->BindAxis("MoveX", this, &ASourceMovementPlayer::Forward);
	InputComponent->BindAxis("MoveY", this, &ASourceMovementPlayer::Right);

	InputComponent->BindAxis("MouseX", this, &APawn::AddControllerYawInput);
	InputComponent->BindAxis("MouseY", this, &APawn::AddControllerPitchInput);

	InputComponent->BindAction("Jump", IE_Pressed, this, &ASourceMovementPlayer::JumpStart);
	InputComponent->BindAction("Jump", IE_Released, this, &ASourceMovementPlayer::JumpEnd);
	
	InputComponent->BindAction("Duck", IE_Pressed, this, &ASourceMovementPlayer::CrouchStart);
	InputComponent->BindAction("Duck", IE_Released, this, &ASourceMovementPlayer::CrouchEnd);
}

void ASourceMovementPlayer::CrouchStart() {
	bDucking = true;
}

void ASourceMovementPlayer::CrouchEnd() {
	bDucking = false;
}
//
//void ASourceMovementPlayer::setDucking(bool duck) {
//	bDucking = duck;
//}
//
//bool ASourceMovementPlayer::isDucking() {
//	return bDucking;
//}
//
//void ASourceMovementPlayer::setDucked(bool duck) {
//	bDucked = duck;
//}
//
//bool ASourceMovementPlayer::IsDucked() {
//	return bDucked;
//}

void ASourceMovementPlayer::JumpStart() {
	PlayerMovement->SetJumpInput(true);
}

void ASourceMovementPlayer::JumpEnd() {
	PlayerMovement->SetJumpInput(false);
}

void ASourceMovementPlayer::Forward(float AxisValue) {
	AddMovementInput(GetActorForwardVector() * AxisValue);
}

void ASourceMovementPlayer::Right(float AxisValue) {
	AddMovementInput(GetActorRightVector() * AxisValue);
}

UCameraComponent* ASourceMovementPlayer::GetCamera() {
	return CurrentCamera;
}

UBoxComponent* ASourceMovementPlayer::GetCollisionBox() {
	return CollisionBox;
}

