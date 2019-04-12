// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "AfterCurfewGameMode.h"
#include "AfterCurfewPawn.h"

AAfterCurfewGameMode::AAfterCurfewGameMode()
{
	// set default pawn class to our character class
	DefaultPawnClass = AAfterCurfewPawn::StaticClass();
}

