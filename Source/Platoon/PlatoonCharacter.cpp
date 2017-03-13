// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "Platoon.h"
#include "PlatoonCharacter.h"

APlatoonCharacter::APlatoonCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	AActor::SetActorHiddenInGame(true);
}