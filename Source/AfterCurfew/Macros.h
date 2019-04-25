// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class AFTERCURFEW_API Macros
{
public:
	//TODO: ditch the template constructor / destructor
	Macros();
	~Macros();

	#define DEBUGMESSAGE(x, y, ...) if(GEngine){GEngine->AddOnScreenDebugMessage(-1, 15.0f, y, FString::Printf(TEXT(x), __VA_ARGS__));}

};
