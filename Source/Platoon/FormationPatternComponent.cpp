// Fill out your copyright notice in the Description page of Project Settings.

#include "Platoon.h"
#include "FormationPatternComponent.h"

UFormationPatternComponent::UFormationPatternComponent()
{
	bWantsBeginPlay = false;
	PrimaryComponentTick.bCanEverTick = false;
}

Kinematic UFormationPatternComponent::GetCenterOfFormation(const TArray<FSlotAssignment>& slotAssignments) const
{	
	Kinematic center;

	for (auto slot : slotAssignments)
	{
		Kinematic location = GetRelativeSlotKinematic(slot.SlotNumber);
		center.Position += location.Position;
		center.Orientation += location.Orientation;
	}

	center.Position /= slotAssignments.Num();
	center.Orientation /= slotAssignments.Num();

	return center;
}

Kinematic UFormationPatternComponent::GetRelativeSlotKinematic(const int& slotIndex) const
{
	Kinematic result;
	const FTransform& slot = FormationSlots[slotIndex];

	result.Position = slot.GetLocation();
	result.Orientation = FMath::DegreesToRadians(slot.GetRotation().Euler().Z);

	return result;
}

FVector UFormationPatternComponent::GetOverallSlotWorldLocation(const int& slotNumber) const
{
	if (FormationSlots.Num() <= slotNumber) return FVector::ZeroVector;

	FVector centerOfFormation;
	for (auto slot : FormationSlots)
	{
		centerOfFormation += slot.GetLocation();
	}
	centerOfFormation /= FormationSlots.Num();

	const FTransform& slot = FormationSlots[slotNumber];
	FVector newSlotLocation = slot.GetLocation() - centerOfFormation;
	FVector location = GetOwner()->GetActorLocation() + GetOwner()->GetActorRotation().RotateVector(newSlotLocation);
	location.Z = 0;
	return location;
}

bool UFormationPatternComponent::SupportSlots(const int& slotCount) const
{
	return FormationSlots.Num() >= slotCount;
}

int UFormationPatternComponent::SupportedSlots() const
{
	return FormationSlots.Num();
}