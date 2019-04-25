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
	MoveSpeed = 1000.0f;

	// Weapon
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
	PlayerInputComponent->BindAxis(MoveForwardBinding);
	PlayerInputComponent->BindAxis(MoveRightBinding);
	PlayerInputComponent->BindAxis(AimForwardBinding);
	PlayerInputComponent->BindAxis(AimRightBinding);
	PlayerInputComponent->BindAxis(LiftUpBinding);
	PlayerInputComponent->BindAction(FireBinding, IE_Pressed, this, &AAfterCurfewPawn::StartFiring);
	PlayerInputComponent->BindAction(FireBinding, IE_Released, this, &AAfterCurfewPawn::StopFiring);
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

	//TODO: movement and rotation needs acceleration / velocity, feeling of weight.
	//TODO: handle rotation collision (if necessary).
	//TODO: use virtual void PlayerController::AddPitchInput(float Val) and virtual void PlayerController::AddYawInput(float Val) for rotations?
	//TODO: turning to face the recticle should not be immediate, there should be turning speed.
	//TODO: Add minor ship rotations on heavy turns to improve the feeling of weight.

	APlayerController * Controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	FVector CurrentLocation = this->GetActorLocation();

	FHitResult TraceHitResult;
	Controller->GetHitResultUnderCursor(ECC_Camera, true, TraceHitResult);
	DrawDebugLine(GetWorld(), CurrentLocation, TraceHitResult.Location, FColor::Red, false);

	// Get the aiming direction for the player
	FVector AimLocation = TraceHitResult.Location;
	FVector AimDirection = AimLocation - CurrentLocation;
	AimDirection.Normalize();

	// Find the new facing rotation for the player
	FRotator PawnRotation = this->GetActorRotation();
	FRotator TargetRotation = AimDirection.Rotation();

	// Turn to face the target rotation by a turn speed amount
	float NewYaw = FMath::FixedTurn(PawnRotation.Yaw, TargetRotation.Yaw, 300.0f * DeltaSeconds);

	// The goal for this movement is slower start with some acceleration and then a very slight break at the end

	// This could also be done with Lerp, and would maybe be better.
	//float NewYaw = FMath::Lerp(PawnRotation.Yaw, TargetRotation.Yaw, 0.5f);
	/*if (!FMath::IsNearlyZero(NewYaw)) {
		DEBUGMESSAGE("YawDifference: %d", FColor::White, YawDifference);
	}*/

	const FRotator NewRotation = FRotator(PawnRotation.Pitch, NewYaw, PawnRotation.Roll);

	// Find XY movement directions
	const float ForwardValue = GetInputAxisValue(MoveForwardBinding);
	const float RightValue = GetInputAxisValue(MoveRightBinding);

	// Clamp max size so that (X=1, Y=1) doesn't cause faster movement in diagonal directions
	const FVector XYMoveDirection = FVector(ForwardValue, RightValue, 0).GetClampedToMaxSize(1.0f);

	// Calculate  movement
	const FVector XYMovement = XYMoveDirection * MoveSpeed * DeltaSeconds;

	// If non-zero size, move this actor
	if (XYMovement.SizeSquared() > 0.0f)
	{
		FHitResult Hit(1.f);

		RootComponent->MoveComponent(XYMovement, NewRotation, true, &Hit);

		if (Hit.IsValidBlockingHit())
		{
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
	const float UpValue = GetInputAxisValue(LiftUpBinding);
	const FVector ZMoveDirection = FVector(0, 0, UpValue).GetClampedToMaxSize(1.0f);
	const FVector ZMovement = ZMoveDirection * MoveSpeed * DeltaSeconds;

	if (ZMovement.SizeSquared() > 0.0f)
	{
		FHitResult Hit(1.f);

		FRotator CurrentRotation = this->GetActorRotation();

		RootComponent->MoveComponent(ZMovement, CurrentRotation, true, &Hit);

		if (Hit.IsValidBlockingHit())
		{
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

