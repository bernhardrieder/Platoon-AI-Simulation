// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "Location.h"
#include "SteeredFormationManager.h"
#include "FormationPatternComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PLATOON_API UFormationPatternComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UFormationPatternComponent();

	Kinematic GetCenterOfFormation(const TArray<FSlotAssignment>& slotAssignments) const;
	Kinematic GetRelativeSlotKinematic(const int& slotNumber) const;
	FVector GetOverallSlotWorldLocation(const int& slotNumber) const;
	bool SupportSlots(const int& slotCount) const;
	int SupportedSlots() const;

	UPROPERTY(EditAnywhere)
	TArray<FTransform> FormationSlots;
};
