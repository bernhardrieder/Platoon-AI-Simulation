// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "SteeringBehaviour.h"
#include <memory>
#include "SteeringActor.generated.h"

UCLASS()
class PLATOON_API ASteeringActor : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleDefaultsOnly)
		class UStaticMeshComponent* Mesh;
public:	
	ASteeringActor();
	virtual void BeginPlay() override;
	virtual void Tick( float DeltaSeconds ) override;

	void SetSteeringBehaviour(std::shared_ptr<SteeringBehaviour> steeringBehaviour);
	std::shared_ptr<SteeringBehaviour> GetSteeringBehvaiour() const;
	Kinematic* GetKinematic();
	void SetSteeringTarget(Kinematic target);
	const Kinematic* GetSteeringTarget() const;
	USplineComponent* FindSpline(AActor* SplineComponentPlacement) const;

	UPROPERTY(EditAnywhere, Category = Steering)
		float MaxSpeed = 500;

	UPROPERTY(EditAnywhere, Category = Steering)
		FColor DebugColor = FColor::Green;

protected:
	virtual void updateKinematics(float deltaTime);

	std::shared_ptr<SteeringBehaviour> m_steeringBehaviour = nullptr;
	Kinematic m_kinematic;
	std::unique_ptr<LookWhereYouAreGoing> m_lookWhereYouAreGoing = nullptr;
	Kinematic m_steeringTarget;
};
