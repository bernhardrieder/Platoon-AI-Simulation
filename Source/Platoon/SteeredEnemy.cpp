// Fill out your copyright notice in the Description page of Project Settings.

#include "Platoon.h"
#include "SteeredEnemy.h"


void ASteeredEnemy::BeginPlay()
{
	Super::BeginPlay();
	setPathFollowingBehaviour();
}

void ASteeredEnemy::setPathFollowingBehaviour()
{
	USplineComponent* Spline = FindSpline(SplineComponentPlacement);
	if (Spline)
	{
		SetActorLocation(Spline->GetWorldLocationAtTime(0));
		SetActorRotation(Spline->GetWorldRotationAtTime(0));
		GetKinematic()->Position = Spline->GetWorldLocationAtTime(0);
		GetKinematic()->Orientation = 0.75f;

		auto pathFollowing = std::make_shared<PathFollowing>(GetKinematic(), 150.f, Spline, 0.4f);
		SetSteeringBehaviour(pathFollowing);
	}
}
