// Originally based off https://github.com/ReKTeK/Defragr
// Original copyright:
// 
/* Copyright (C) Terence-Lee 'Zinglish' Davis
 * Written by Terence-Lee 'Zinglish' Davis <zinglish[at]gmail.com>
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/BoxComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "SourcePlayer.generated.h"

// TODO: Add Mouse Scalar to Match Source's / Quake's mouse input							(SHORT TERM HACK)
// TODO: Add Raw Mouse Input to the engine's best public capabilities. 						(LONG TERM)
// TODO: Add custom Mouse Handler to get 1:1 Source/Quake mouse handling. 					(LONG TERM)

class USourcePlayerMovement;

UCLASS(config = Game, BlueprintType, meta = (ShortTooltip = "Source Movement Player"))
class SOURCEMOVEMENT_API ASourceMovementPlayer : public APawn
{
	GENERATED_BODY()

public:
	ASourceMovementPlayer();

public:
	UCameraComponent* GetCamera();
	UBoxComponent* GetCollisionBox();

	UPROPERTY(BlueprintReadOnly)
	uint8 bDucked:1;
	UPROPERTY(BlueprintReadOnly)
	uint8 bDucking:1;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	
	UCameraComponent* CurrentCamera;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UBoxComponent* CollisionBox;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USourcePlayerMovement* PlayerMovement;

public:
	/* Health of the player */
	UPROPERTY(Category = "Gameplay", EditAnywhere, BlueprintReadWrite)
	int Health;

	/* Armour of the player */
	UPROPERTY(Category = "Gameplay", EditAnywhere, BlueprintReadWrite)
	int Armour;

	UPROPERTY(Category = "Crouch Movement (SourceMovement)", EditAnywhere, BlueprintReadWrite)
	float CrouchEyeHeight;

	UPROPERTY(Category = "Crouch Movement (SourceMovement)", EditAnywhere, BlueprintReadWrite)
	float CrouchCollisionBoxHeight;

	UPROPERTY(Category = "General Movement (SourceMovement)", EditAnywhere, BlueprintReadWrite)
	float CollisionBoxHeight;

protected:
	void JumpStart();
	void JumpEnd();
	void Forward(float AxisValue);
	void Right(float  AxisValue);
	
	void CrouchStart();
	void CrouchEnd();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
