// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Location.h"
#include <vector>
#include "Components/SplineComponent.h"
#include "Engine.h" //needed for drawdebuglines
#include <memory>

class SteeringBehaviour
{
public:
	SteeringBehaviour(Kinematic* character);

	virtual ~SteeringBehaviour() {}

	virtual void GetSteering(SteeringOutput* output) = 0;
	void SetCharacter(Kinematic* character);

protected:
	Kinematic* Character;
};


class Seek : public SteeringBehaviour
{
public:
	Seek(Kinematic* character, const FVector* target_position, float max_acceleration);
	void GetSteering(SteeringOutput* output) override;

protected:
	const FVector* TargetPosition;
	float MaxAcceleration;
};


class Flee : public Seek
{
public:
	Flee(Kinematic* character, const FVector* target_position, float max_acceleration);
	void GetSteering(SteeringOutput* output) override;
};

class Arrive : public SteeringBehaviour
{
public:
	Arrive(Kinematic* character, const FVector* target_position, float max_acceleration, float max_speed, float target_radius, float slow_radius, float time_to_target = 0.1f);
	void GetSteering(SteeringOutput* output) override;

protected:
	const FVector* TargetPosition;
	float MaxAcceleration;
	float MaxSpeed;
	float TargetRadius;
	float SlowRadius;
	float TimeToTarget = 0.1f;
};

class Pursue : public Seek
{
public:
	Pursue(Kinematic* character, float max_acceleration, float max_prediction_time, const Kinematic* target);
	void GetSteering(SteeringOutput* output) override;

protected:	
	float MaxPredictionTime;
	const Kinematic* Target;
};

class Evade : public Flee
{
public:
	Evade(Kinematic* character, float max_acceleration, float max_prediction_time, const Kinematic* target);
	void GetSteering(SteeringOutput* output) override;

protected:
	float MaxPredictionTime;
	const Kinematic* Target;
};

class PathFollowing : public Seek
{
public:
	PathFollowing(Kinematic* character, float max_acceleration, USplineComponent* path, float path_offset);
	void GetSteering(SteeringOutput* output) override;

protected:
	USplineComponent* Path;
	float PathOffset;
	float CurrentParam;
};

class Separation : public SteeringBehaviour
{
public:
	Separation(Kinematic* character, float threshold, float decay_coefficient, float max_acceleration);
	void GetSteering(SteeringOutput* output) override;
	void AddTarget(const Kinematic* target);

protected:
	std::vector<const Kinematic*> Targets;
	float Threshold;
	float DecayCoefficient;
	float MaxAcceleration;
};

class CollisionAvoidance : public SteeringBehaviour
{
public:
	CollisionAvoidance(Kinematic* character, float max_acceleration, float target_radius);
	void GetSteering(SteeringOutput* output) override;
	void AddTarget(const Kinematic* target);

protected:
	float MaxAcceleration;
	std::vector<const Kinematic*> Targets;
	float TargetRadius;
};

class ObstacleAvoidance : public Seek
{
public:
	ObstacleAvoidance(Kinematic* character, float max_acceleration, const UWorld* world, float avoid_distance, float look_ahead_distance, float whiskers_distance, float whiskers_angle);
	void GetSteering(SteeringOutput* output) override;

protected:
	const UWorld* World;
	float AvoidDistance;
	float LookAheadDistance;
	float WhiskersDistance;
	float WhiskersAngle;
};

class Align : public SteeringBehaviour
{
public:
	Align(Kinematic* character, Kinematic* target, float max_angular_acceleration, float max_rotation, float target_radius, float slow_radius, float time_to_target);
	void GetSteering(SteeringOutput* output) override;

protected:
	Kinematic* Target;

	float MaxAngularAcceleration;
	float MaxRotation;

	float TargetRadius;
	float SlowRadius;

	float TimeToTarget = 0.1f;

private:
	static float mapToRange(float rotation);
};

class LookWhereYouAreGoing : public Align
{
public:
	LookWhereYouAreGoing(Kinematic* character, float max_angular_acceleration, float max_rotation, float target_radius = std::numeric_limits<float>::epsilon(), float slow_radius = 0.0f, float time_to_target = 0.1f);
	void GetSteering(SteeringOutput* output) override;
};

class BlendedSteering : public SteeringBehaviour
{
public:
	struct BehaviourAndWeight
	{
		std::unique_ptr<SteeringBehaviour> Behaviour;
		float Weight;
	};

	BlendedSteering(Kinematic* character, float max_acceleration, float max_rotation);
	void GetSteering(SteeringOutput* output) override;

	std::vector<BehaviourAndWeight> Behaviours;
protected:	
	float MaxAcceleration;
	float MaxRotation = 0;
};

class PrioritySteering : public SteeringBehaviour
{
public:
	PrioritySteering(Kinematic* character, float epsilon);
	void GetSteering(SteeringOutput* output) override;

	std::vector<BlendedSteering> Behaviours;
protected:
	float Epsilon;
};