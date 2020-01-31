// Originally based off https://github.com/ReKTeK/Defragr
// Original copyright:
// 
/* Copyright (C) Terence-Lee 'Zinglish' Davis
 * Written by Terence-Lee 'Zinglish' Davis <zinglish[at]gmail.com>
 */


#include "SourcePlayerMovement.h"
#include "SourcePlayer.h"

// @todo Add basic player replication, positional and look replication to near by players.
// @todo Move Everything related to Tracing/Sliding to Collision Component.													(Long term, low priority)
// @todo Make auto jump toggle																							    (Long term, low priority)
// @todo add crouching that moves eye point																					(Long term, low priority)
// @todo add crouching that changes player collision																		(Long term, low priority)
// @todo replace updated comp pos calls with GetComponentLocation
// @todo integrate more engine functions like SafeMoveUpdatedComponent, MoveUpdatedComponent, INetworkInterface
// @todo create parent with Trace/Slide funcs
// @todo match source gravity

// @todo New plan, next up -> Make new SourcePlayer that uses ACharacter as a base and ACharacterMovementComponent as a base. I don't mean inheritance I mean straight up ripping it.

USourcePlayerMovement::USourcePlayerMovement()  {
	PrimaryComponentTick.bCanEverTick = true;
	//PostPhysicsTickFunction.bCanEverTick = true;
	//PostPhysicsTickFunction.bStartWithTickEnabled = false;
	//PostPhysicsTickFunction.SetTickFunctionEnable(false);
	//PostPhysicsTickFunction.TickGroup = TG_PostPhysics;
}

void USourcePlayerMovement::BeginPlay() {
    Super::BeginPlay();
	USourcePlayerMovement::SetUpdatedPosition(UpdatedComponent->GetComponentLocation());
}

void USourcePlayerMovement::PostLoad() {
	Super::PostLoad();
	PlayerOwner = Cast<ASourceMovementPlayer>(PawnOwner);
}

void USourcePlayerMovement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction) {
	FVector InputMove = PawnOwner->ConsumeMovementInputVector();
  	
	//UpdateDuckJumpEyeOf fset();
	Duck(DeltaTime);

	QueueJump();  
	// Check ground
	TraceGround();
	if(bCanGroundMove)
		GroundMove(InputMove, DeltaTime);
	else
		AirMove(InputMove, DeltaTime);
	TraceGround();
	
	//if (GEngine)
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("Speed: %f"), Velocity.Size()));

	//MoveUpdatedComponent(GetUpdatedPosition(), FRotator::ZeroRotator, false);
	//Ensures rotation cannot be changed.
	UpdatedComponent->SetWorldLocationAndRotation(GetUpdatedPosition(), FRotator::ZeroRotator, false);
    
}

void USourcePlayerMovement::Duck(float DeltaTime) {
	//FVector NewExtent = PlayerOwner->GetCollisionBox()->GetScaledBoxExtent();
	//if (PlayerOwner->bDucking) {
	//	NewExtent.Z = PlayerOwner->CrouchCollisionBoxHeight;
	//	
	//} else {
	//	NewExtent.Z = PlayerOwner->CollisionBoxHeight;
	//}

	//if (NewExtent != PlayerOwner->GetCollisionBox()->GetScaledBoxExtent())
	//	PlayerOwner->GetCollisionBox()->SetBoxExtent(NewExtent);

}


FVector USourcePlayerMovement::GetWishVelocity(FVector InputMove) {
	FRotator ViewRotation = GetCameraRotation();
	FVector ForwardLook = FRotationMatrix(ViewRotation).GetScaledAxis(EAxis::X);
	FVector RightLook = FRotationMatrix(ViewRotation).GetScaledAxis(EAxis::Y);

	// Project moves down to flat plane.
	ForwardLook.Z = 0.f;
	RightLook.Z = 0.f;
	ForwardLook.Normalize();
	RightLook.Normalize();

	FVector WishVelocity = FVector::ZeroVector;
	WishVelocity = ForwardLook * InputMove.X + RightLook * InputMove.Y;
	WishVelocity.Z = 0.f;

	return WishVelocity;
}

void USourcePlayerMovement::AirMove(FVector InputMove, float DeltaTime) {
	FVector WishVelocity = GetWishVelocity(InputMove);

	float WishSpeed = WishVelocity.Size();
	FVector WishDirection = WishVelocity;
	WishDirection.Normalize();
	WishSpeed *= 320.0f;

	if ((WishSpeed != 0.0f) && (WishSpeed > AirMaxSpeed)) {
		WishSpeed = AirMaxSpeed;
	}

	ApplyAirAcceleration(WishDirection, WishSpeed, AirAcceleration, DeltaTime);

	// Apply gravity
	Velocity.Z -= MovementGravity * DeltaTime;

	StepSlideMove(true, DeltaTime);
}


void USourcePlayerMovement::GroundMove(FVector InputMove, float DeltaTime) {
	if(CheckJump())
	{
		AirMove(InputMove, DeltaTime);
		return;
	}

	ApplyFriction(DeltaTime);


	FVector WishVelocity = GetWishVelocity(InputMove);

	FVector WishDirection = WishVelocity;
	float WishSpeed = WishDirection.Size();
	WishDirection.Normalize();
	WishSpeed *= GroundMaxSpeed;
	if ((WishSpeed != 0.0f) && (WishSpeed > GroundMaxSpeed)) {
		WishSpeed = GroundMaxSpeed;
	}

	ApplyAcceleration(WishDirection, WishSpeed, GroundAcceleration, DeltaTime);

	float vel = Velocity.Size();

	ClipVelocity(Velocity, GroundTrace.ImpactNormal, Velocity, Overclip);

	Velocity.Normalize();
	Velocity *= vel;

	// Don't do anything if standing still
	if(Velocity.X == 0.f && Velocity.Y == 0.f)
		return;

	StepSlideMove(false, DeltaTime);
}

void USourcePlayerMovement::FlyMove(FVector InputMove, float DeltaTime) {
	float Speed = Velocity.Size();
	if (Speed < 1) {
		Velocity = FVector::ZeroVector;
	} else {
		// Apply friction.
		float Drop = 0;

		float LocalFriction = Friction * 1.5;	// extra friction
		float Control = Speed < GroundStopSpeed ? GroundStopSpeed : Speed;
		Drop += Control * LocalFriction * DeltaTime;

		// scale the velocity
		float NewSpeed = Speed - Drop;
		if (NewSpeed < 0)
			NewSpeed = 0;
		NewSpeed /= Speed;

		Velocity *= NewSpeed;
	}


	FRotator ViewRotation = GetCameraRotation();

	FVector ForwardLook = FRotationMatrix(ViewRotation).GetScaledAxis(EAxis::X);
	FVector RightLook = FRotationMatrix(ViewRotation).GetScaledAxis(EAxis::Y);

	FVector WishVelocity = FVector::ZeroVector;
	WishVelocity.X = ForwardLook.X * InputMove.X + RightLook.X * InputMove.Y;
	WishVelocity.Y = ForwardLook.Y * InputMove.X + RightLook.Y * InputMove.Y;
	WishVelocity.Z = ForwardLook.Z * InputMove.X + RightLook.Z * InputMove.Y;
	WishVelocity = WishVelocity.GetClampedToSize(-1, 1);

	FVector WishDirection = WishVelocity;
	float WishSpeed = WishDirection.Size();
	WishDirection.Normalize();
	WishSpeed *= 1000.f;

	// Apply Velocity changes.
	ApplyAcceleration(WishDirection, WishSpeed, GroundAcceleration, DeltaTime);

	// VectorMA	
	SetUpdatedPosition(GetUpdatedPosition() + (Velocity * DeltaTime));
}

void USourcePlayerMovement::ApplyAcceleration(FVector WishDirection, float WishSpeed, float DynamicAcceleration, float DeltaTime)
{
	float AddSpeed;
	float AccelerationSpeed;
	float CurrentSpeed;

	CurrentSpeed = FVector::DotProduct(WishDirection, Velocity);
	AddSpeed = WishSpeed - CurrentSpeed;
	if(AddSpeed <= 0.f)
		return;

	// Determine amount of Acceleration. (Consider adding surfrace friction here.)
	AccelerationSpeed = DynamicAcceleration * DeltaTime * WishSpeed;

	// Cap at AddSpeed
	if(AccelerationSpeed > AddSpeed)
		AccelerationSpeed = AddSpeed;

	Velocity += AccelerationSpeed * WishDirection;
}


void USourcePlayerMovement::ApplyAirAcceleration(FVector WishDirection, float WishSpeed, float Acceleration, float DeltaTime) {

	float CurrentSpeed = FVector::DotProduct(WishDirection, Velocity);
	float AddSpeed = WishSpeed - CurrentSpeed;
	// If we aren't adding anything, finished.
	if (AddSpeed <= 0)
		return;

	float AccelSpeed = Acceleration * WishSpeed * DeltaTime;
	// Cap it.
	if (AccelSpeed > AddSpeed)
		AccelSpeed = AddSpeed;
	
	Velocity += WishDirection * AccelSpeed;

}

void USourcePlayerMovement::ApplyFriction(float DeltaTime) {
	float Speed = Velocity.Size();
	float Drop = 0.0f;
	if(bIsGrounded)
	{
		float Control = Speed < GroundAcceleration ? GroundDampening : Speed;
		Drop = Control * Friction * DeltaTime;
	}

	// if(MovementType == EMovementType::Spectate)
		// Drop += speed * SpectatorFriction * Delta;

	float NewSpeed = Speed - Drop;
	if(NewSpeed < 0.f)
		NewSpeed = 0.f;
	if(Speed > 0.f)
		NewSpeed /= Speed;

	Velocity.X *= NewSpeed;
	Velocity.Y *= NewSpeed;
}

void USourcePlayerMovement::ClipVelocity(FVector In, FVector Normal, FVector& Out, float Overbounce) {
	float Backoff = FVector::DotProduct(In, Normal);
		if (Backoff < 0.f)
			Backoff *= Overbounce;
		else
			Backoff /= Overbounce;

	Out = In - Normal * Backoff;
}

bool USourcePlayerMovement::CheckJump() {
	if (bWillJump) {
		Velocity.Z = JumpForce;
		bWillJump = false;
		return true;
	}
	return false;
}

void USourcePlayerMovement::QueueJump() {
	if (bInJump && !bWillJump && bIsGrounded)
		bWillJump = true;
	if (!bInJump)
		bWillJump = false;
}

void USourcePlayerMovement::SetJumpInput(bool Input) {
	bInJump = Input;
}

void USourcePlayerMovement::SetUpdatedPosition(const FVector NewPosition) {
	UpdatedPosition = NewPosition;
}

FVector USourcePlayerMovement::GetUpdatedPosition() {
	return UpdatedPosition;
}

FRotator USourcePlayerMovement::GetCameraRotation() {
	return PawnOwner->GetActorRotation().GetInverse() + PawnOwner->GetViewRotation();
}

void USourcePlayerMovement::SetUpdatedComponent(USceneComponent* NewUpdatedComponent) {
	Super::SetUpdatedComponent(NewUpdatedComponent);
	PlayerOwner = Cast<ASourceMovementPlayer>(PawnOwner);
}

// Movement Code

bool USourcePlayerMovement::SlideMove(bool Gravity, float DeltaTime) {
	int32 NumPlanes;
	int32 NumBumps;
	FVector Planes[5];
	FVector EndVelocity;

	NumPlanes = 0;
	NumBumps = 4;

	if(Gravity)
	{
		EndVelocity = Velocity;
		EndVelocity.Z -= Gravity * DeltaTime;
		Velocity.Z = (Velocity.Z + EndVelocity.Z) * 0.5f;

		if (bIsGrounded)
			ClipVelocity(Velocity, GroundTrace.ImpactNormal, Velocity, Overclip);
	}

	// This is used later to resolve collisions
	float TimeLeft = DeltaTime;

	// Never turn against the ground plane
	if(bIsGrounded)
	{
		NumPlanes = 1;
		Planes[0] = GroundTrace.ImpactNormal;
	}

	// Never turn against the original velocity
	Planes[NumPlanes] = Velocity;
	Planes[NumPlanes].Normalize();
	NumPlanes++;

	int32 BumpCount = 0;
	for(; BumpCount < NumBumps; BumpCount++)
	{
		// Calculate the position we are trying to move to
		// VectorMA
		FVector End = GetUpdatedPosition() + (Velocity * TimeLeft);
		
		// See if we can get there
		FHitResult PartialTrace;
		Trace(PartialTrace, GetUpdatedPosition(), End);

		// If there was no hit then move the player the entire way and stop
		// this iteration, however if there was a hit then move the player
		// along the trace until the hit.
		SetUpdatedPosition(TraceEnd(PartialTrace));
		if(!PartialTrace.bBlockingHit)
			break;

		// TODO: Record the entity that was collided with

		TimeLeft -= TimeLeft * PartialTrace.Time;

		if(NumPlanes >= MaxClipPlanes)
		{
			Velocity = FVector::ZeroVector;
			return true;
		}

		// If this is the same plane we hit before, nudge the velocity
		// along it, which fixes some epsilon issues with non-axial planes.
		int32 i;
		for(i = 0; i < NumPlanes; i++)
		{
			float DotProd = FVector::DotProduct(PartialTrace.ImpactNormal, Planes[i]);
			if(DotProd > 0.99f)
			{
				Velocity += PartialTrace.ImpactNormal;
				break;
			}
		}
		if(i < NumPlanes)
			continue;
		Planes[NumPlanes] = PartialTrace.ImpactNormal;
		NumPlanes++;

		// We modify the velocity of the player so it parallels all the clip planes
		i = 0;
		for(i = 0; i < NumPlanes; i++)
		{
			FVector StartClipVelocity;
			FVector EndClipVelocity;

			// Find a plane that the player enters
			float Into = FVector::DotProduct(Velocity, Planes[i]);
			// The move doesn't interact with the plane
			if(Into >= 0.1f)
				continue;

			// TODO: Impact magnitude
			// FROM: bg_slidemove.c
			// 
			// if ( -into > pml.impactSpeed ) {
			// 	pml.impactSpeed = -into;
			// }

			// Slide along the plane
			ClipVelocity(Velocity, Planes[i], StartClipVelocity, Overclip);
			ClipVelocity(EndVelocity, Planes[i], EndClipVelocity, Overclip);

			// Check for a second plane that the new move enters
			for(int32 j = 0; j < NumPlanes; j++)
			{
				if(j == i)
					continue;
				// The move doesn't interact with the plane
				if(FVector::DotProduct(StartClipVelocity, Planes[j]) >= 0.1f)
					continue;

				// Slide along the plane
				ClipVelocity(StartClipVelocity, Planes[j], StartClipVelocity, Overclip);
				ClipVelocity(EndClipVelocity, Planes[j], EndClipVelocity, Overclip);

				// If it goes back into the first clip plane
				if(FVector::DotProduct(StartClipVelocity, Planes[j]) >= 0.1f)
					continue;

				// Slide the original velocity along the intersection
				// of the two planes
				FVector Dir;
				float D;

				Dir = FVector::CrossProduct(Planes[i], Planes[j]);
				Dir.Normalize();
				D = FVector::DotProduct(Dir, Velocity);
				StartClipVelocity = Dir * D;

				Dir = FVector::CrossProduct(Planes[i], Planes[j]);
				Dir.Normalize();
				D = FVector::DotProduct(Dir, EndVelocity);
				EndClipVelocity = Dir * D;

				// Check for third plane intersection
				for(int32 k = 0; k < NumPlanes; k++)
				{
					if (k == i || k == j)
						continue;
					// The move doesn't interact with the plane
					if (FVector::DotProduct(StartClipVelocity, Planes[k]) >= 0.1f)
						continue;

					// Stop dead
					Velocity = FVector::ZeroVector;
					//GEngine->AddOnScreenDebugMessage(20, 0.01f, FColor::Red, TEXT("Player is stuck"));

					return true;
				}
			}

			// Fixed all plane interactions, continue trying another
			// move.
			Velocity = StartClipVelocity;
			EndVelocity = EndClipVelocity;
			break;
		}
	}

	if(Gravity)
		Velocity = EndVelocity;

	return (BumpCount != 0);
}

void USourcePlayerMovement::StepSlideMove(bool Gravity, float DeltaTime)
{
	FVector StartO = GetUpdatedPosition();
	FVector StartV = Velocity;

	// Got where we wanted on the first go
	if(SlideMove(Gravity, DeltaTime) == 0)
		return;
	
	FVector Down = StartO;
	Down.Z -= MaxStepSize;
	
	FHitResult StepTrace;
	Trace(StepTrace, StartO, Down);
	FVector Up = FVector::ZeroVector;
	Up.Z = 1;
	
	// Never step up if the player still has up velocity
	if (Velocity.Z > 0.f &&
		(StepTrace.Time == 1.0f || FVector::DotProduct(StepTrace.ImpactNormal, Up) < 0.7f))
		return;

	FVector DownO = GetUpdatedPosition();
	FVector DownV = Velocity;

	Up = StartO;
	Up.Z += MaxStepSize;

	Trace(StepTrace, StartO, Up);
	if(StepTrace.bStartPenetrating)
		return; // Can't step up

	float StepSize = TraceEnd(StepTrace).Z - StartO.Z;
	SetUpdatedPosition(TraceEnd(StepTrace));
	Velocity = StartV;

	SlideMove(Gravity, DeltaTime);

	// Push down the final amount
	Down = GetUpdatedPosition();
	Down.Z -= StepSize;
	Trace(StepTrace, GetUpdatedPosition(), Down);

	SetUpdatedPosition(TraceEnd(StepTrace));
	if(StepTrace.Time < 1.0f) {
		ClipVelocity(Velocity, StepTrace.ImpactNormal, Velocity, Overclip);
	}
}

void USourcePlayerMovement::TraceGround() {

	bool Hit = Trace(GroundTrace, GetUpdatedPosition(), GetUpdatedPosition() - FVector(0.f, 0.f, GroundTraceDistance));
	// No hits then the player cannot ground move
	if(GroundTrace.Time == 1.0f) {
		bIsGrounded    = false;
		bCanGroundMove = false;
		return;
	}
	// Check if getting thrown off the ground
	if(Velocity.Z > 0 && FVector::DotProduct(Velocity, GroundTrace.ImpactNormal) > 10) {
		bIsGrounded = false;
		bCanGroundMove = false;
		return;
	}
	// Slopes that are too steep will not be considered walkable
	if(GroundTrace.ImpactNormal.Z < MinWalkNormal) {
		bIsGrounded = true;
		bCanGroundMove = false;
		return;
	}
	bIsGrounded = true;
	bCanGroundMove = true;
}

bool USourcePlayerMovement::Trace(FHitResult& Result, FVector Start, FVector End) {
	// Get the player's collider extents (note that in this case
	// the collider is required to be a box).
	FCollisionShape CollisionShape = PlayerOwner->GetCollisionBox()->GetCollisionShape();

	FCollisionQueryParams QueryParams(TEXT(""), false);
	FCollisionObjectQueryParams ObjectQueryParams(ECC_WorldStatic);

	bool Hit = GetWorld()->SweepSingleByObjectType(Result, Start, End, FQuat::Identity, ObjectQueryParams, CollisionShape, QueryParams);
	// bool Hit = GetWorld()->SweepSingle(Result, Start, End, FQuat::Identity, CollisionShape, QueryParams, ObjectQueryParams);
	return Hit;
}

FVector USourcePlayerMovement::TraceEnd(FHitResult& T) {
	// If the trace started in a collider then we calculate the escaped
	// vector and return it.
	if(T.bStartPenetrating) {
		FVector Loc = T.TraceStart + T.ImpactNormal * (T.PenetrationDepth) + (T.Normal * Underclip);
		//GEngine->AddOnScreenDebugMessage(50, 0.01f, FColor::Yellow, FString::Printf(TEXT("TraceEnd: Loc [X: %.6f, Y: %.6f, Z: %.6f]"), T.Normal.X, T.Normal.Y, T.Normal.Z));
		//GEngine->AddOnScreenDebugMessage(51, 0.01f, FColor::Yellow, FString::Printf(TEXT("TraceEnd: PenDepth [X: %.8f]"), T.PenetrationDepth));
		return Loc;
	}

	if(!T.bBlockingHit)
		return T.TraceEnd;
	else
		return T.TraceStart + T.Time * (T.TraceEnd - T.TraceStart) + (T.Normal * Underclip);
}