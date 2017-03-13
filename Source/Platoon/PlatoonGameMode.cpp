// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "Platoon.h"
#include "PlatoonGameMode.h"
#include "PlatoonCharacter.h"

APlatoonGameMode::APlatoonGameMode()
{
	 //set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/Empty_Character_BP"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}