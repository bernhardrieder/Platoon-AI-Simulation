// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SteeringActor.h"
#include "SteeredFormationManager.generated.h"

USTRUCT()
struct FSlotAssignment
{
	GENERATED_BODY()

	UPROPERTY()
	ASteeringActor* Character;

	UPROPERTY()
	int SlotNumber;

	FSlotAssignment() : FSlotAssignment(nullptr) {}
	FSlotAssignment(ASteeringActor* character) : Character(character), SlotNumber(0) {}
};

UCLASS()
class PLATOON_API ASteeredFormationManager : public ASteeringActor
{
	GENERATED_BODY()

	ASteeredFormationManager();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category = "Steering")
		TSubclassOf<class ASteeringActor> SteeringActorClass;

	UPROPERTY(VisibleInstanceOnly, Category = "Steering")
		class UFormationPatternComponent* FormationPattern;

	UPROPERTY(EditAnywhere, Category = "Steering")
		AActor* SplinePlacementComponent;

public:
	bool AddCharacter(ASteeringActor* character);
	void RemoveCharacter(const ASteeringActor* character);
	void UpdateSlots();

	UPROPERTY(EditAnywhere, Category = "Steering")
	float FormationInteractionRadius = 1000;

private:
	TArray<FSlotAssignment> m_slotAssignments;
	Kinematic m_centerOfFormation;
	int m_initialMaxSpeed;

	void setPathFollowingBehaviour();
	void createFormationMember();
	void updateSlotAssignments();
	Kinematic getAnchorPoint() const;
	void dampMaxSpeed();
};
