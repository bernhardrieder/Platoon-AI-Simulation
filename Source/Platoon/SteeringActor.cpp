// Fill out your copyright notice in the Description page of Project Settings.

#include "Platoon.h"
#include "SteeringActor.h"
#include <string>


ASteeringActor::ASteeringActor()
{
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;

	PrimaryActorTick.bCanEverTick = true;
}

void ASteeringActor::BeginPlay()
{
	m_lookWhereYouAreGoing = std::make_unique<LookWhereYouAreGoing>(&m_kinematic, 10000.f, 1.f);
	Super::BeginPlay();
}

void ASteeringActor::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
	updateKinematics(DeltaTime);
}

void ASteeringActor::SetSteeringBehaviour(std::shared_ptr<SteeringBehaviour> steeringBehaviour)
{
	m_steeringBehaviour = steeringBehaviour;
}

std::shared_ptr<SteeringBehaviour> ASteeringActor::GetSteeringBehvaiour() const
{
	return m_steeringBehaviour;
}

Kinematic* ASteeringActor::GetKinematic()
{
	return &m_kinematic;
}

void ASteeringActor::SetSteeringTarget(Kinematic target)
{
	m_steeringTarget = target;
}

const Kinematic* ASteeringActor::GetSteeringTarget() const
{
	return &m_steeringTarget;
}

void ASteeringActor::updateKinematics(float deltaTime)
{
	if (m_steeringBehaviour == nullptr) return;
	SteeringOutput steer;
	m_steeringBehaviour->GetSteering(&steer); // only position steering
	m_lookWhereYouAreGoing->GetSteering(&steer); //only orientational steering

	steer.Linear.Z = 0;
	m_kinematic.Integrate(steer, deltaTime);
	m_kinematic.TrimMaxSpeed(MaxSpeed);
	
	SetActorLocationAndRotation(m_kinematic.Position, m_kinematic.GetOrientationAsVector().ToOrientationRotator());

	DrawDebugSphere(GetWorld(), m_kinematic.Position, 10, 1, DebugColor, false, 10.f);
	DrawDebugDirectionalArrow(GetWorld(), m_kinematic.Position, m_kinematic.Position + m_kinematic.Velocity, 10000.f, DebugColor);
}

USplineComponent* ASteeringActor::FindSpline(AActor* SplineComponentPlacement) const
{
	TArray<UActorComponent*> comp = SplineComponentPlacement->GetComponentsByClass(USplineComponent::StaticClass());
	for (auto a : comp)
	{
		USplineComponent* Spline = Cast<USplineComponent>(a);
		if (Spline)
			return Spline;
	}
	return nullptr;
}
