// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "AfterCurfewPawn.h"
#include "AfterCurfewProjectile.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Engine/GameEngine.h"
#include "Engine/Public/DrawDebugHelpers.h"
#include "Macros.h"

const FName AAfterCurfewPawn::MoveForwardBinding("MoveForward");
const FName AAfterCurfewPawn::MoveRightBinding("MoveRight");
const FName AAfterCurfewPawn::AimForwardBinding("AimForward");
const FName AAfterCurfewPawn::AimRightBinding("AimRight");
const FName AAfterCurfewPawn::LiftUpBinding("LiftUp");
const FName AAfterCurfewPawn::ThrustBinding("Thrust");
const FName AAfterCurfewPawn::FireBinding("Fire");

AAfterCurfewPawn::AAfterCurfewPawn()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ShipMesh(TEXT("/Game/TwinStick/Meshes/TwinStickUFO.TwinStickUFO"));
	// Create the mesh component
	ShipMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShipMesh"));
	RootComponent = ShipMeshComponent;
	ShipMeshComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
	ShipMeshComponent->SetStaticMesh(ShipMesh.Object);

	// Cache our sound effect
	static ConstructorHelpers::FObjectFinder<USoundBase> FireAudio(TEXT("/Game/TwinStick/Audio/TwinStickFire.TwinStickFire"));
	FireSound = FireAudio.Object;

	// Create a camera boom...
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->bAbsoluteRotation = true; // Don't want arm to rotate when ship does
	CameraBoom->TargetArmLength = 1200.f;
	CameraBoom->RelativeRotation = FRotator(-80.f, 0.f, 0.f);
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level

	// Create a camera...
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	CameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	CameraComponent->bUsePawnControlRotation = false;	// Camera does not rotate relative to arm

	// Movement
	//MoveSpeed = 1000.0f;

	/*ThrustInterpSpeed = 2.f;
	ThrustMaxSpeed = 1000.f;
	ThrustMinSpeed = 0.f;
	CurrentThrustSpeed = 0.f;*/

	InterpSpeed = 2.f;
	MaxSpeed = 1000.f;
	MinSpeed = -1000.f;
	CurrForwardSpeed = 0.f;
	CurrRightSpeed = 0.f;

	YawInterpSpeed = 2.f;
	MaxYawSpeed = 600.f;
	MinYawSpeed = 0.f;
	CurrYawSpeed = 0.f;

	LiftInterpSpeed = 2.f;
	MaxLiftSpeed = 1000.f;
	MinLiftSpeed = -1000.f;
	CurrentLiftSpeed = 0.f;

	// Weapons
	GunOffset = FVector(90.f, 0.f, 0.f);
	FireRate = 0.1f;
	FireSpread = 2.5f;
	ProjectileScale = 1.f;
	ProjectileInitialSpeed = 3000.f;
	ProjectileMaxSpeed = 3000.f;
	bCanFire = true;
	bFire = false;
}

void AAfterCurfewPawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	// set up gameplay key bindings
	//PlayerInputComponent->BindAxis(MoveForwardBinding);
	//PlayerInputComponent->BindAxis(MoveRightBinding);
	//PlayerInputComponent->BindAxis(LiftUpBinding);
	PlayerInputComponent->BindAxis(MoveForwardBinding, this, &AAfterCurfewPawn::MoveForwardInput);
	PlayerInputComponent->BindAxis(MoveRightBinding, this, &AAfterCurfewPawn::MoveRightInput);
	//PlayerInputComponent->BindAxis(ThrustBinding, this, &AAfterCurfewPawn::ThrustInput);
	PlayerInputComponent->BindAxis(LiftUpBinding, this, &AAfterCurfewPawn::LiftUpInput);
	PlayerInputComponent->BindAxis(AimForwardBinding);
	PlayerInputComponent->BindAxis(AimRightBinding);
	PlayerInputComponent->BindAction(FireBinding, IE_Pressed, this, &AAfterCurfewPawn::StartFiring);
	PlayerInputComponent->BindAction(FireBinding, IE_Released, this, &AAfterCurfewPawn::StopFiring);
}

/*void AAfterCurfewPawn::ThrustInput(float Val)
{
	// Is there any input?
	bool bHasInput = !FMath::IsNearlyEqual(Val, 0.f);

	// Determing target speed based on input
	float TargetThrustSpeed = bHasInput ? Val * ThrustMaxSpeed : 0.f;

	// Calculate new speed
	float NewThrustSpeed = FMath::FInterpTo(CurrentThrustSpeed, TargetThrustSpeed, GetWorld()->GetDeltaSeconds(), ThrustInterpSpeed);

	// Clamp between MinSpeed and MaxSpeed
	CurrentThrustSpeed = FMath::Clamp(NewThrustSpeed, ThrustMinSpeed, ThrustMaxSpeed);

	//DEBUGMESSAGE("Thrust: Val: %f, Target: %f, Current: %f", FColor::Red, Val, TargetThrustSpeed, CurrentThrustSpeed);
}*/

void AAfterCurfewPawn::MoveForwardInput(float Val)
{
	// Is there any input?
	bool bHasInput = !FMath::IsNearlyEqual(Val, 0.f);

	// Determing target speed based on input
	float TargetForwardSpeed = bHasInput ? Val * MaxSpeed : 0.f;

	// Calculate new speed
	float NewForwardSpeed = FMath::FInterpTo(CurrForwardSpeed, TargetForwardSpeed, GetWorld()->GetDeltaSeconds(), InterpSpeed);

	// Clamp between MinSpeed and MaxSpeed
	CurrForwardSpeed = FMath::Clamp(NewForwardSpeed, MinSpeed, MaxSpeed);
}

void AAfterCurfewPawn::MoveRightInput(float Val)
{
	// Is there any input?
	bool bHasInput = !FMath::IsNearlyEqual(Val, 0.f);

	// Determing target speed based on input
	float TargetRightSpeed = bHasInput ? Val * MaxSpeed : 0.f;

	// Calculate new speed
	float NewRightSpeed = FMath::FInterpTo(CurrRightSpeed, TargetRightSpeed, GetWorld()->GetDeltaSeconds(), InterpSpeed);

	// Clamp between MinSpeed and MaxSpeed
	CurrRightSpeed = FMath::Clamp(NewRightSpeed, MinSpeed, MaxSpeed);
}

void AAfterCurfewPawn::LiftUpInput(float Val)
{	
	// Is there any input?
	bool bHasInput = !FMath::IsNearlyEqual(Val, 0.f);

	// Determing target speed based on input
	float TargetLiftSpeed = bHasInput ? Val * MaxLiftSpeed : 0.f;
	
	// Calculate new speed
	float NewLiftSpeed = FMath::FInterpTo(CurrentLiftSpeed, TargetLiftSpeed, GetWorld()->GetDeltaSeconds(), LiftInterpSpeed);

	// Clamp between MinSpeed and MaxSpeed
	CurrentLiftSpeed = FMath::Clamp(NewLiftSpeed, MinLiftSpeed, MaxLiftSpeed);
}

void AAfterCurfewPawn::StartFiring()
{
	bFire = 1;
}

void AAfterCurfewPawn::StopFiring()
{
	bFire = 0;
}

void AAfterCurfewPawn::Tick(float DeltaSeconds)
{
	//TODO: bounds objects created and placed via code?

	//TODO: add some recoil on firing shots?
	//TODO: support better controller version of aiming in addition to the mouse.
	//TODO: add a custom cursor sprite instead of the default crosshair.
	//TODO: Add minor ship rotations to Pitch / Roll on heavy turns to improve the feeling of weight.

	APlayerController * Controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	FVector CurrentLocation = this->GetActorLocation();

	FHitResult TraceHitResult;
	Controller->GetHitResultUnderCursor(ECC_Camera, true, TraceHitResult);
	DrawDebugLine(GetWorld(), CurrentLocation, TraceHitResult.Location, FColor::Red, false);

	// Get the aiming direction for the player
	FVector AimLocation = TraceHitResult.Location;
	FVector AimDirection = AimLocation - CurrentLocation;
	AimDirection.Normalize();

	// Get the player's current rotation
	FRotator CurrentRotation = this->GetActorRotation();

	// Determine target rotation based on input
	FRotator TargetRotation = AimDirection.Rotation();

	// Turn to face the target rotation by a turn speed amount

	//DEBUGMESSAGE("Current: %f, Target: %f", FColor::White, CurrentRotation.Yaw, TargetRotation.Yaw);

	/*bool bHasInput = !FMath::IsNearlyEqual(Val, 0.f);

	// Determing target speed based on input
	float TargetThrustSpeed = bHasInput ? Val * ThrustMaxSpeed : 0.f;

	// Calculate new speed
	float NewThrustSpeed = FMath::FInterpTo(CurrentThrustSpeed, TargetThrustSpeed, GetWorld()->GetDeltaSeconds(), ThrustInterpSpeed);

	// Clamp between MinSpeed and MaxSpeed
	CurrentThrustSpeed = FMath::Clamp(NewThrustSpeed, ThrustMinSpeed, ThrustMaxSpeed);*/

	bool bHasYawInput = !FMath::IsNearlyEqual(CurrentRotation.Yaw, TargetRotation.Yaw, 0.0001f);

	float YawTargetSpeed = bHasYawInput ? MaxYawSpeed : 0.f;

	float YawNewSpeed = FMath::FInterpTo(CurrYawSpeed, YawTargetSpeed, GetWorld()->GetDeltaSeconds(), YawInterpSpeed);

	CurrYawSpeed = FMath::Clamp(YawNewSpeed, MinYawSpeed, MaxYawSpeed);

	//DEBUGMESSAGE("CurrYawSpeed: %f", FColor::White, CurrYawSpeed);

	float NewYaw = FMath::FixedTurn(CurrentRotation.Yaw, TargetRotation.Yaw, CurrYawSpeed * DeltaSeconds);
	//float NewYaw = FMath::FInterpTo(CurrentRotation.Yaw, TargetRotation.Yaw, GetWorld()->GetDeltaSeconds(), 4.f);

	const FRotator NewRotation = FRotator(CurrentRotation.Pitch, NewYaw, CurrentRotation.Roll);

	// Calculate change in rotation this frame
	/*FRotator DeltaRotation(0, 0, 0);
	DeltaRotation.Pitch = 0.f;//CurrentPitchSpeed * DeltaSeconds;
	DeltaRotation.Yaw = CurrentYawSpeed * DeltaSeconds;
	DeltaRotation.Roll = 0.f;//CurrentRollSpeed * DeltaSeconds;*/

	// Rotate plane
	//AddActorLocalRotation(DeltaRotation);

	//TODO: This has a problem where holding input buttons on more than 1 axis produces faster acceleration, how to fix?
	const FVector XYMovement = FVector(CurrForwardSpeed * DeltaSeconds, CurrRightSpeed * DeltaSeconds, 0.f).GetClampedToMaxSize(MaxSpeed * DeltaSeconds);

	// If non-zero size, move this actor
	if (XYMovement.SizeSquared() > 0.0f)
	{
		FHitResult Hit(1.f);

		RootComponent->MoveComponent(XYMovement, NewRotation, true, &Hit);

		if (Hit.IsValidBlockingHit())
		{
			//DEBUGMESSAGE("WE HIT A WALL AT %f", FColor::White, UGameplayStatics::GetRealTimeSeconds(GetWorld()));

			const FVector Normal2D = Hit.Normal.GetSafeNormal2D();
			const FVector Deflection = FVector::VectorPlaneProject(XYMovement, Normal2D) * (1.f - Hit.Time);
			RootComponent->MoveComponent(Deflection, NewRotation, true);
		}
	}
	// If no XY movement, still turn to face the aim direction
	else
	{
		RootComponent->SetWorldRotation(NewRotation);
	}

	// Handle lift movement
	/*const float UpValue = GetInputAxisValue(LiftUpBinding);
	const FVector ZMoveDirection = FVector(0, 0, UpValue).GetClampedToMaxSize(1.0f);
	const FVector ZMovement = ZMoveDirection * 1000.f * DeltaSeconds;*/

	const FVector ZMoveDirection = FVector::UpVector;
	const FVector ZMovement = ZMoveDirection * CurrentLiftSpeed * DeltaSeconds;

	//DEBUGMESSAGE("CurrentLiftSpeed: %f", FColor::Red, CurrentLiftSpeed);

	DEBUGMESSAGE("Location Z: %f", FColor::White, CurrentLocation.Z);

	if (!ZMovement.IsNearlyZero(0.1f))//.SizeSquared() > 0.0f)// && !(CurrentLocation.Z + ZMovement.Z <= 207.f))
	{
		FHitResult Hit(1.f);

		FRotator CurrentRotation = this->GetActorRotation();

		RootComponent->MoveComponent(ZMovement, CurrentRotation, true, &Hit);

		if (Hit.IsValidBlockingHit())
		{
			//DEBUGMESSAGE("WE HIT A FLOOR/CEILING AT %f", FColor::White, UGameplayStatics::GetRealTimeSeconds(GetWorld()));

			const FVector Normal2D = Hit.Normal.GetSafeNormal2D();
			const FVector Deflection = FVector::VectorPlaneProject(ZMovement, Normal2D) * (1.f - Hit.Time);
			RootComponent->MoveComponent(Deflection, CurrentRotation, true);
		}
	}

	// Create fire direction vector
	//const FVector FireDirection = FVector(AimDirection.X, AimDirection.Y, 0.f);

	// Fire vector is the forward vector of the ship
	AimDirection = this->GetActorForwardVector();
	const FVector FireDirection = FVector(AimDirection.X, AimDirection.Y, 0.f);

	// Try and fire a shot if fire button is being held down
	if (bFire == true)
	{
		FireShot(FireDirection);
	}
	Super::Tick(DeltaSeconds);
}

void AAfterCurfewPawn::FireShot(FVector FireDirection)
{
	// If it's ok to fire again
	if (bCanFire == true)
	{
		// If we are pressing fire stick in a direction
		if (FireDirection.SizeSquared() > 0.0f)
		{
			// Add slight variance for better looking bullet spread.
			const FRotator FireRotation = FireDirection.Rotation().Add(0, FMath::FRandRange(-FireSpread, FireSpread), 0);

			// Spawn projectile at an offset from this pawn
			const FVector SpawnLocation = GetActorLocation() + FireRotation.RotateVector(GunOffset);

			UWorld* const World = GetWorld();
			if (World != NULL)
			{
				// spawn the projectile
				//World->SpawnActor<AAfterCurfewProjectile>(SpawnLocation, FireRotation);

				FVector Scale = FVector(1.0f);
				const FTransform SpawnTransform = FTransform(FireRotation, SpawnLocation, Scale * ProjectileScale);
				AAfterCurfewProjectile* NewProjectile = World->SpawnActorDeferred<AAfterCurfewProjectile>(AAfterCurfewProjectile::StaticClass(), SpawnTransform);
				NewProjectile->Initialize(ProjectileInitialSpeed, ProjectileMaxSpeed);
				NewProjectile->FinishSpawning(SpawnTransform);
			}

			bCanFire = false;
			World->GetTimerManager().SetTimer(TimerHandle_ShotTimerExpired, this, &AAfterCurfewPawn::ShotTimerExpired, FireRate);

			// try and play the sound if specified
			if (FireSound != nullptr)
			{
				UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
			}

			bCanFire = false;
		}
	}
}

void AAfterCurfewPawn::ShotTimerExpired()
{
	bCanFire = true;
}

