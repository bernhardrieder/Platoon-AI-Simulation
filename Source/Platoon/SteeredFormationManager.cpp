// Fill out your copyright notice in the Description page of Project Settings.

#include "Platoon.h"
#include "SteeredFormationManager.h"
#include "FormationPatternComponent.h"
#include <string>
#include "SteeredEnemy.h"

ASteeredFormationManager::ASteeredFormationManager() : ASteeringActor()
{
	FormationPattern = CreateDefaultSubobject<UFormationPatternComponent>(TEXT("FormationPattern"));
}

void ASteeredFormationManager::BeginPlay()
{
	Super::BeginPlay();
	m_initialMaxSpeed = MaxSpeed;
	if (GetWorld())
	{
		setPathFollowingBehaviour();
		createFormationMember();
	}
}

void ASteeredFormationManager::Tick(float DeltaSeconds)
{
	UpdateSlots();
	dampMaxSpeed();
	Super::Tick(DeltaSeconds);
}

bool ASteeredFormationManager::AddCharacter(ASteeringActor* character)
{	
	int occupiedSlots = m_slotAssignments.Num();
	if(FormationPattern->SupportSlots(occupiedSlots+1))
	{
		m_slotAssignments.Push(FSlotAssignment(character));
		updateSlotAssignments();
		return true;
	}
	return false;
}

void ASteeredFormationManager::RemoveCharacter(const ASteeringActor* character)
{
	int characterIndex = -1;
	for (int i = 0; i < m_slotAssignments.Num(); ++i)
	{
		if (m_slotAssignments[i].Character == character)
		{
			characterIndex = i;
			break;
		}
	}
	if (characterIndex != -1)
	{
		m_slotAssignments.RemoveAt(characterIndex);
		updateSlotAssignments();
	}
}

void ASteeredFormationManager::UpdateSlots()
{
	Kinematic anchor = getAnchorPoint();
	DrawDebugPoint(GetWorld(), anchor.Position, 10, FColor::Green);

	for(auto& slot : m_slotAssignments)
	{
		Kinematic slotRelative = FormationPattern->GetRelativeSlotKinematic(slot.SlotNumber);

		Kinematic target;
		target.Position = anchor.Position + (slotRelative.Position - m_centerOfFormation.Position).RotateAngleAxis(FMath::RadiansToDegrees(anchor.Orientation), FVector::UpVector);
		target.Orientation = anchor.Orientation + slotRelative.Orientation;
		slot.Character->SetSteeringTarget(target);

		DrawDebugSphere(GetWorld(), target.Position, 50, 1, FColor::Blue);
	}
}

void ASteeredFormationManager::setPathFollowingBehaviour()
{
	if (SplinePlacementComponent)
	{
		USplineComponent* Spline = FindSpline(SplinePlacementComponent);
		if (Spline)
		{
			SetActorLocation(Spline->GetWorldLocationAtTime(0));
			SetActorRotation(Spline->GetWorldRotationAtTime(0));
			GetKinematic()->Position = Spline->GetWorldLocationAtTime(0);
			GetKinematic()->Orientation = 1.f;

			auto pathFollowing = std::make_shared<PathFollowing>(GetKinematic(), 150.f, Spline, 0.4f);
			SetSteeringBehaviour(pathFollowing);
		}
	}
}

void ASteeredFormationManager::createFormationMember()
{
	int numOfActors = FormationPattern->SupportedSlots() > 4 ? 4 : FormationPattern->SupportedSlots();
	for (int i = 0; i < numOfActors; ++i)
	{
		ASteeringActor* actor = GetWorld()->SpawnActor<ASteeringActor>(SteeringActorClass);
		actor->MaxSpeed = 600;
		AddCharacter(actor);
		actor->DebugColor = FColor::Orange;
	}
	for (auto a : m_slotAssignments)
	{
		ASteeringActor* currentActor = a.Character;

		auto priority = std::make_shared<PrioritySteering>(currentActor->GetKinematic(), std::numeric_limits<float>::epsilon());
		currentActor->SetSteeringBehaviour(priority);

		auto arrive = std::make_unique<Arrive>(currentActor->GetKinematic(), &currentActor->GetSteeringTarget()->Position, 400.f, currentActor->MaxSpeed, 10.f, 600.f);

		auto collisionAvoidance = std::make_unique<CollisionAvoidance>(nullptr, 700.f, 200.f);
		//add enemies for collision avoidance		
		for (TActorIterator<ASteeredEnemy> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		{
			collisionAvoidance->AddTarget(ActorItr->GetKinematic());
		}

		auto separation = std::make_unique<Separation>(nullptr, 200.f, std::numeric_limits<float>::max(), 700.f);
		//add team members for separation
		for (auto other : m_slotAssignments)
		{
			if (other.Character != currentActor)
			{
				separation->AddTarget(other.Character->GetKinematic());
			}
		}

		auto obstacleAvoidance = std::make_unique<ObstacleAvoidance>(nullptr, 400.f, GetWorld(), 600.f, 500.f, 300.f, PI / 5);

		BlendedSteering blended_CollisionAvoidance = BlendedSteering(nullptr, 700.f, 0.f);
		blended_CollisionAvoidance.Behaviours.push_back({ std::move(collisionAvoidance), 1.f });

		BlendedSteering blended_Arrive = BlendedSteering(nullptr, 600.f, 0.f);
		blended_Arrive.Behaviours.push_back({ std::move(arrive), 1.f });

		BlendedSteering blended_ObstacleAvoidance = BlendedSteering(nullptr, 800.f, 0.f);
		blended_ObstacleAvoidance.Behaviours.push_back({ std::move(obstacleAvoidance), 1.f });

		BlendedSteering blended_Separation = BlendedSteering(nullptr, 500.f, 0.f);
		blended_Separation.Behaviours.push_back({ std::move(separation), 1.f });

		priority->Behaviours.push_back(std::move(blended_Separation));
		priority->Behaviours.push_back(std::move(blended_ObstacleAvoidance));
		priority->Behaviours.push_back(std::move(blended_CollisionAvoidance));
		priority->Behaviours.push_back(std::move(blended_Arrive));
	}
	UpdateSlots();


	for (auto current : m_slotAssignments)
	{
		FVector pos = FormationPattern->GetOverallSlotWorldLocation(current.SlotNumber);
		ASteeringActor* actor = current.Character;
		actor->GetKinematic()->Position = pos;
		actor->GetKinematic()->Orientation = GetKinematic()->Orientation;
		actor->SetActorLocation(pos);
	}
}

void ASteeredFormationManager::updateSlotAssignments()
{
	for (int i = 0; i < m_slotAssignments.Num(); ++i)
	{
		m_slotAssignments[i].SlotNumber = i;
	}

	m_centerOfFormation = FormationPattern->GetCenterOfFormation(m_slotAssignments);
}

Kinematic ASteeredFormationManager::getAnchorPoint() const
{
	return m_kinematic;
}

void ASteeredFormationManager::dampMaxSpeed()
{
	FVector centerOfMass;
	for (const auto& slot : m_slotAssignments)
	{
		centerOfMass += slot.Character->GetKinematic()->Position;
	}
	centerOfMass /= m_slotAssignments.Num();

	float distance = FVector::Dist(m_kinematic.Position, centerOfMass);

	if (distance < FormationInteractionRadius)
	{
		if (distance < 0)
			distance = 0;
		float multiplier = 1 - distance / FormationInteractionRadius;

		MaxSpeed = m_initialMaxSpeed*multiplier;
	}
	else
		MaxSpeed = 1;
}
