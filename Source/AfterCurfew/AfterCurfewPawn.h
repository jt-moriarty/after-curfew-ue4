// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AfterCurfewPawn.generated.h"

UCLASS(Blueprintable)
class AAfterCurfewPawn : public APawn
{
	GENERATED_BODY()

	/* The mesh component */
	UPROPERTY(Category = Mesh, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* ShipMeshComponent;

	/** The camera */
	UPROPERTY(Category = Camera, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* CameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(Category = Camera, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

public:
	AAfterCurfewPawn();

	/** Offset from the ships location to spawn projectiles */
	UPROPERTY(Category = "Gameplay\|Weapons", EditAnywhere, BlueprintReadWrite)
	FVector GunOffset;

	/* How fast the weapon will fire */
	UPROPERTY(Category = "Gameplay\|Weapons", EditAnywhere, BlueprintReadWrite)
	float FireRate;

	/* The maximum spread of the bullets */
	UPROPERTY(Category = "Gameplay\|Weapons", EditAnywhere, BlueprintReadWrite)
	float FireSpread;

	/* The scale of the bullets */
	UPROPERTY(Category = "Gameplay\|Weapons", EditAnywhere, BlueprintReadWrite)
	float ProjectileScale;

	/* The initial speed of the bullets */
	UPROPERTY(Category = "Gameplay\|Weapons", EditAnywhere, BlueprintReadWrite)
	float ProjectileInitialSpeed;

	/* The max speed of the bullets */
	UPROPERTY(Category = "Gameplay\|Weapons", EditAnywhere, BlueprintReadWrite)
	float ProjectileMaxSpeed;

	/* The speed our ship moves around the level */
	//UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
	//float MoveSpeed;

	/** Sound to play each time we fire */
	UPROPERTY(Category = Audio, EditAnywhere, BlueprintReadWrite)
	class USoundBase* FireSound;

	// Begin Actor Interface
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	// End Actor Interface

	/* Fire a shot in the specified direction */
	void FireShot(FVector FireDirection);

	/* Handler for the fire timer expiration */
	void ShotTimerExpired();

	void StartFiring();

	void StopFiring();

	// Static names for axis bindings
	static const FName MoveForwardBinding;
	static const FName MoveRightBinding;
	static const FName AimForwardBinding;
	static const FName AimRightBinding;
	static const FName LiftUpBinding;
	static const FName ThrustBinding;
	static const FName FireBinding;

protected:
	/** Bound to the thrust axis */
	void LiftUpInput(float Val);

	/** Bound to the vertical axis */
	void MoveForwardInput(float Val);

	/** Bound to the horizontal axis */
	void MoveRightInput(float Val);

	//void ThrustInput(float Val);

private:

	/* Flag to control firing  */
	uint32 bCanFire : 1;

	/* Flag to control firing  */
	uint32 bFire : 1;

	/** Handle for efficient management of ShotTimerExpired timer */
	FTimerHandle TimerHandle_ShotTimerExpired;

	/*
	UPROPERTY(Category = Gameplay, EditAnywhere)
	float ThrustInterpSpeed;

	UPROPERTY(Category = Gameplay, EditAnywhere)
	float ThrustMaxSpeed;

	UPROPERTY(Category = Gameplay, EditAnywhere)
	float ThrustMinSpeed;

	float CurrentThrustSpeed;
	*/

	/** How quickly speed changes */
	UPROPERTY(Category = "Gameplay\|Movement\|Planar", EditAnywhere)
	float InterpSpeed;

	/** Max speed */
	UPROPERTY(Category = "Gameplay\|Movement\|Planar", EditAnywhere)
	float MaxSpeed;

	/** Min speed */
	UPROPERTY(Category = "Gameplay\|Movement\|Planar", EditAnywhere)
	float MinSpeed;

	/** Current forward speed */
	float CurrForwardSpeed;

	/** Current forward speed */
	float CurrRightSpeed;


	/** How quickly speed changes */
	UPROPERTY(Category = "Gameplay\|Movement\|Turning", EditAnywhere)
	float YawInterpSpeed;

	/** Max speed */
	UPROPERTY(Category = "Gameplay\|Movement\|Turning", EditAnywhere)
	float MaxYawSpeed;

	/** Min speed */
	UPROPERTY(Category = "Gameplay\|Movement\|Turning", EditAnywhere)
	float MinYawSpeed;

	/** Current turning speed */
	float CurrYawSpeed;


	/** How quickly lift speed changes */
	UPROPERTY(Category = "Gameplay\|Movement\|Lift", EditAnywhere)
	float LiftInterpSpeed;

	/** Max lift speed */
	UPROPERTY(Category = "Gameplay\|Movement\|Lift", EditAnywhere)
	float MaxLiftSpeed;

	/** Min lift speed */
	UPROPERTY(Category = "Gameplay\|Movement\|Lift", EditAnywhere)
	float MinLiftSpeed;

	/** Current lift speed */
	float CurrentLiftSpeed;

public:
	/** Returns ShipMeshComponent subobject **/
	FORCEINLINE class UStaticMeshComponent* GetShipMeshComponent() const { return ShipMeshComponent; }
	/** Returns CameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetCameraComponent() const { return CameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
};