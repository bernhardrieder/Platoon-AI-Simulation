// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SteeringActor.h"
#include "SteeredEnemy.generated.h"

UCLASS()
class PLATOON_API ASteeredEnemy : public ASteeringActor
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = Steering)
	AActor* SplineComponentPlacement;

public:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
private:
	void setPathFollowingBehaviour();
};
