// Originally based off https://github.com/ReKTeK/Defragr
// Original copyright:
// 
/* Copyright (C) Terence-Lee 'Zinglish' Davis
 * Written by Terence-Lee 'Zinglish' Davis <zinglish[at]gmail.com>
 */
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "SourcePlayer.h"
#include "SourcePlayerMovement.generated.h"


UCLASS()
class SOURCEMOVEMENT_API USourcePlayerMovement : public UPawnMovementComponent
{
	GENERATED_BODY()

public:
	USourcePlayerMovement();
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	virtual void SetUpdatedComponent(USceneComponent* NewUpdatedComponent) override;
	virtual void PostLoad() override;

protected:
	/** Character movement component belongs to */
	UPROPERTY(Transient, DuplicateTransient)
	ASourceMovementPlayer* PlayerOwner;

protected:
	/** Is the player touching the ground. This includes steep slopes and such **/
	uint8 bIsGrounded:1;

	/** Determines whether the player can run or not */
	uint8 bCanGroundMove:1;

	/** Jump Input **/
	uint8 bInJump:1;

	/** Determines if player will jump next tick. **/
	uint8 bWillJump:1;

	/** Ground Trace **/
	FHitResult GroundTrace;

	/** Position to be set next tick. **/
	FVector UpdatedPosition;

public:
	void SetUpdatedPosition(const FVector NewPosition);
	void SetJumpInput(bool Input);

protected:
	/* Returns Camera rotation, including actor rotation offset.*/
	FRotator GetCameraRotation();
	FVector GetWishVelocity(FVector InputMove);
	FVector GetUpdatedPosition();
	
	void Duck(float DeltaTime);
	void AirMove(FVector InputMove, float DeltaTime);
	void GroundMove(FVector InputMove, float DeltaTime);
	void FlyMove(FVector InputMove, float DeltaTime);
	void ApplyFriction(float DeltaTime);
	void ApplyAirAcceleration(FVector WishDirection, float WishSpeed, float AirAcceleration, float DeltaTime);
	void ApplyAcceleration(FVector WishDirection, float WishSpeed, float DynamicAcceleration, float DeltaTime);
	void ClipVelocity(FVector In, FVector Normal, FVector& Out, float Overbounce);

	void QueueJump();
	bool CheckJump();

	bool Trace(FHitResult& Result, FVector Start, FVector End);
     /** Auto traces ground */
	void TraceGround();
	/** Gets the trace's ending. If there's a hit, it will return the location of the hit, if there
	is no hit then the trace's ending will return. If the trace started in a collider then the
	resolution vector will be returned */
	FVector TraceEnd(FHitResult& T);
	/** Attempts to solve a collision by sliding the player and clipping velocity */
	bool SlideMove(bool Gravity, float DeltaTime);

	/** Same as SlideMove except it takes into account slopes and stairs */
	void StepSlideMove(bool Gravity, float DeltaTime);

public:
	UPROPERTY(Category = "General Movement (Source)", BlueprintReadOnly)
	TEnumAsByte<enum EMovementMode> MovementMode;

	/** Custom gravity scale. Gravity is multiplied by this amount for the character */
	UPROPERTY(Category = "General Movement (Source)", EditAnywhere, BlueprintReadWrite)
	float MovementGravity = 1600.f;

	/** Amount of jumping force the player has, does not represent absolute jump height */
	UPROPERTY(Category = "General Movement (Source)", EditAnywhere, BlueprintReadWrite)
	float JumpForce = 420.f;

	/** Ground friction */
	UPROPERTY(Category = "General Movement (Source)", EditAnywhere, BlueprintReadWrite)
	float Friction = 6.f;

	/** The rate at which the player stops when on the ground */
	UPROPERTY(Category = "Ground Movement (Source)", EditAnywhere, BlueprintReadWrite)
	float GroundStopSpeed = 200.f;

	/** The maximum speed the player can travel while on the ground */
	UPROPERTY(Category = "Ground Movement (Source)", EditAnywhere, BlueprintReadWrite)
	float GroundMaxSpeed = 640.f;

	/** The rate at which the player gains speed while on the ground */
	UPROPERTY(Category = "Ground Movement (Source)", EditAnywhere, BlueprintReadWrite)
	float GroundAcceleration = 20.0f;

	/** The rate at which the player loses speed while on the ground */
	UPROPERTY(Category = "Ground Movement (Source)", EditAnywhere, BlueprintReadWrite)
	float GroundDampening = 20.f;

	/** The rate at which the player gains speed while on the ground */
	UPROPERTY(Category = "Air Movement (Source)", EditAnywhere, BlueprintReadWrite)
	float AirAcceleration = 20.0f;

	/** The rate at which the player gains speed while on the ground */
	UPROPERTY(Category = "Air Movement (Source)", EditAnywhere, BlueprintReadWrite)
	float AirMaxSpeed = 320.f;

	/** The amount the collision box has to travel before detecting the ground */
	UPROPERTY(Category = "General Collision (Source)", EditAnywhere, BlueprintReadWrite)
	float GroundTraceDistance = 0.25f;

	UPROPERTY(Category = "General Collision (Source)", EditAnywhere, BlueprintReadWrite)
	float MinWalkNormal = 0.7f;

	UPROPERTY(Category = "General Collision (Source)", EditAnywhere, BlueprintReadWrite)
	float Overclip = 1.001f;

	// The amount of 'skin' the player has
	UPROPERTY(Category = "General Collision (Source)", EditAnywhere, BlueprintReadWrite)
	float Underclip = 0.1f;

	UPROPERTY(Category = "General Collision (Source)", EditAnywhere, BlueprintReadWrite)
	int MaxClipPlanes = 5;

	UPROPERTY(Category = "General Collision (Source)", EditAnywhere, BlueprintReadWrite)
	int MaxStepSize = 18;
};


