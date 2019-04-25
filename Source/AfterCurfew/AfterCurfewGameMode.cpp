// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "AfterCurfewGameMode.h"
#include "AfterCurfewPawn.h"
#include "AfterCurfewPlayerController.h"

AAfterCurfewGameMode::AAfterCurfewGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = AAfterCurfewPlayerController::StaticClass();

	// set default pawn class to our character class
	DefaultPawnClass = AAfterCurfewPawn::StaticClass();
}

