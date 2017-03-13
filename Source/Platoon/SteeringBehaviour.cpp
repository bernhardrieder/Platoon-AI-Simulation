// Fill out your copyright notice in the Description page of Project Settings.

#include "Platoon.h"
#include "SteeringBehaviour.h"
#include "Components/SplineComponent.h"
#include <string>

SteeringBehaviour::SteeringBehaviour(Kinematic* character): Character(character)
{
}

void SteeringBehaviour::SetCharacter(Kinematic* character)
{
	Character = character;
}

Seek::Seek(Kinematic* character, const FVector* target_position, float max_acceleration): SteeringBehaviour(character),
                                                                                          TargetPosition(target_position),
                                                                                          MaxAcceleration(max_acceleration)
{
}

void Seek::GetSteering(SteeringOutput* output)
{
	output->Linear = *TargetPosition;
	output->Linear -= Character->Position;

	if (output->Linear.SizeSquared() > 0)
	{
		output->Linear.Normalize(0);
		output->Linear *= MaxAcceleration;
	}
}

Flee::Flee(Kinematic* character, const FVector* target_position, float max_acceleration): Seek(character, target_position, max_acceleration)
{
}

void Flee::GetSteering(SteeringOutput* output)
{
	output->Linear = Character->Position;
	output->Linear -= *TargetPosition;

	if (output->Linear.SizeSquared() > 0)
	{
		output->Linear.Normalize(0);
		output->Linear *= MaxAcceleration;
	}
}

Arrive::Arrive(Kinematic* character, const FVector* target_position, float max_acceleration, float max_speed, float target_radius, float slow_radius, float time_to_target): SteeringBehaviour(character),
                                                                                                                                                                             TargetPosition(target_position),
                                                                                                                                                                             MaxAcceleration(max_acceleration),
                                                                                                                                                                             MaxSpeed(max_speed),
                                                                                                                                                                             TargetRadius(target_radius),
                                                                                                                                                                             SlowRadius(slow_radius),
                                                                                                                                                                             TimeToTarget(time_to_target)
{
}

void Arrive::GetSteering(SteeringOutput* output)
{
	output->Clear();

	FVector direction = *TargetPosition - Character->Position;
	float distance = direction.Size();

	if (distance < TargetRadius)
		return;

	float targetSpeed;
	if (distance > SlowRadius)
		targetSpeed = MaxSpeed;
	else
		targetSpeed = MaxSpeed * (distance / SlowRadius);

	//use as velocity now
	direction.Normalize(0);
	direction *= targetSpeed;

	output->Linear = direction - Character->Velocity;
	output->Linear /= TimeToTarget;

	if (output->Linear.SizeSquared() > 0)
	{
		output->Linear.Normalize(0);
		output->Linear *= MaxAcceleration;
	}
}

Pursue::Pursue(Kinematic* character, float max_acceleration, float max_prediction_time, const Kinematic* target): Seek(character, nullptr, max_acceleration),
                                                                                                                  MaxPredictionTime(max_prediction_time),
                                                                                                                  Target(target)
{
}

void Pursue::GetSteering(SteeringOutput* output)
{
	FVector direction = Target->Position - Character->Position;
	float distance = direction.Size();

	float speed = Character->Velocity.Size();
	float predictionTime;

	if (speed <= (distance / MaxPredictionTime))
		predictionTime = MaxPredictionTime;
	else
		predictionTime = distance / speed;

	FVector newTarget = Target->Position;
	newTarget += Target->Velocity * predictionTime;
	Seek::TargetPosition = &newTarget;
	Seek::GetSteering(output);
}

Evade::Evade(Kinematic* character, float max_acceleration, float max_prediction_time, const Kinematic* target): Flee(character, nullptr, max_acceleration),
                                                                                                                                                MaxPredictionTime(max_prediction_time),
                                                                                                                                                Target(target)
{
}

void Evade::GetSteering(SteeringOutput* output)
{
	/* <- same code like pursue -> */
	FVector direction = Target->Position - Character->Position;
	float distance = direction.Size();

	float speed = Character->Velocity.Size();
	float predictionTime;

	if (speed <= (distance / MaxPredictionTime))
		predictionTime = MaxPredictionTime;
	else
		predictionTime = distance / speed;

	FVector newTarget = Target->Position;
	newTarget += Target->Velocity * predictionTime;
	/* <- same code like pursue -> */

	Flee::TargetPosition = &newTarget;
	Flee::GetSteering(output);
}

PathFollowing::PathFollowing(Kinematic* character, float max_acceleration, USplineComponent* path, float path_offset): Seek(character, nullptr, max_acceleration),
                                                                                                                       Path(path),
                                                                                                                       PathOffset(path_offset)
{
}

void PathFollowing::GetSteering(SteeringOutput* output)
{
	CurrentParam = Path->FindInputKeyClosestToWorldLocation(Character->Position);
	CurrentParam += PathOffset;

	FVector followPos = Path->GetLocationAtSplineInputKey(CurrentParam, ESplineCoordinateSpace::World);
	followPos.Z = 0.f;

	TargetPosition = &followPos;

	Seek::GetSteering(output);
}

Separation::Separation(Kinematic* character, float threshold, float decay_coefficient, float max_acceleration): SteeringBehaviour(character),
                                                                                                                Threshold(threshold),
                                                                                                                DecayCoefficient(decay_coefficient),
                                                                                                                MaxAcceleration(max_acceleration)
{
}

void Align::GetSteering(SteeringOutput* output)
{
	float targetRotation;
	float rotation = Target->Orientation - Character->Orientation;

	//Map the result to the (-pi, pi) interval
	rotation = mapToRange(rotation);

	float rotationSize = FMath::Abs(rotation);
	if (rotationSize < TargetRadius)
		return;
	if (rotationSize > SlowRadius)
		targetRotation = MaxRotation;
	else
		targetRotation = MaxRotation * rotationSize / SlowRadius;

	targetRotation *= rotation / rotationSize;
	output->Angular = targetRotation - Character->Rotation;
	output->Angular /= TimeToTarget;

	float angularAcceleration = FMath::Abs(output->Angular);
	if (angularAcceleration > MaxAngularAcceleration)
	{
		output->Angular /= angularAcceleration;
		output->Angular *= MaxAngularAcceleration;
	}
}

float Align::mapToRange(float rotation)
{
	if (rotation > PI)
		return rotation - 2* PI;
	if (rotation < -PI)
		return rotation + 2* PI;
	return rotation;
}

LookWhereYouAreGoing::LookWhereYouAreGoing(Kinematic* character, float max_angular_acceleration, float max_rotation, float target_radius, float slow_radius, float time_to_target): Align(character, nullptr, max_angular_acceleration, max_rotation, target_radius, slow_radius, time_to_target)
{
}

void LookWhereYouAreGoing::GetSteering(SteeringOutput* output)
{
	if (Character->Velocity.Size() < std::numeric_limits<float>::epsilon())
		return;
	Kinematic newTarget;
	Target = &newTarget;
	Target->Orientation = FMath::Atan2(-Character->Velocity.X, Character->Velocity.Y);
	Align::GetSteering(output);
}

void Separation::GetSteering(SteeringOutput* output)
{
	FVector direction;
	float distance, strength;
	for (auto target : Targets)
	{
		direction = target->Position - Character->Position;
		distance = direction.Size();

		if (distance < Threshold)
		{
			strength = FMath::Min(DecayCoefficient / FMath::Square(distance), MaxAcceleration);
			direction.Normalize(0);
			output->Linear += strength * -direction;
		}
	}
}

void Separation::AddTarget(const Kinematic* target)
{
	Targets.push_back(target);
}

CollisionAvoidance::CollisionAvoidance(Kinematic* character, float max_acceleration, float target_radius): SteeringBehaviour(character),
                                                                                                           MaxAcceleration(max_acceleration),
                                                                                                           TargetRadius(target_radius)
{
}

void CollisionAvoidance::GetSteering(SteeringOutput* output)
{
	float shortestTime = INFINITY;

	const Kinematic* firstTarget = nullptr;
	float firstMinSeparation = 0, minSeparation = 0, firstDistance = 0, distance = 0;
	FVector firstRelativePos = FVector::ZeroVector, relativePos = FVector::ZeroVector, firstRelativeVel = FVector::ZeroVector, relativeVel = FVector::ZeroVector;

	float relativeSpeed, timeToCollision;
	for (auto target : Targets)
	{
		relativePos = target->Position - Character->Position;
		relativePos.Normalize();
		relativeVel = target->Velocity - Character->Velocity;
		relativeSpeed = relativeVel.Size();
		timeToCollision = -(FVector::DotProduct(relativePos, relativeVel) / FMath::Square(relativeSpeed));

		distance = FVector::Dist(target->Position, Character->Position);
		minSeparation = distance - relativeSpeed * timeToCollision;

		if (minSeparation > 2 * TargetRadius)
		{
			continue;
		}

		if (timeToCollision > 0 && timeToCollision < shortestTime)
		{
			shortestTime = timeToCollision;
			firstTarget = target;
			firstMinSeparation = minSeparation;
			firstDistance = distance;
			firstRelativePos = relativePos;
			firstRelativeVel = relativeVel;
		}
	}
	if (firstTarget == nullptr)
	{
		output->Linear = FVector::ZeroVector;
		return;
	}

	if (firstMinSeparation <= 0 || firstDistance < 2 * TargetRadius)
		relativePos = firstTarget->Position - Character->Position;
	else
		relativePos = firstRelativePos + firstRelativeVel * shortestTime;

	relativePos.Normalize(0);
	output->Linear = -relativePos * MaxAcceleration;
}

void CollisionAvoidance::AddTarget(const Kinematic* target)
{
	Targets.push_back(target);
}

ObstacleAvoidance::ObstacleAvoidance(Kinematic* character, float max_acceleration, const UWorld* world, float avoid_distance, float look_ahead_distance, float whiskers_distance, float whiskers_angle): Seek(character, nullptr, max_acceleration),
                                                                                                                                                                                                         World(world),
                                                                                                                                                                                                         AvoidDistance(avoid_distance),
                                                                                                                                                                                                         LookAheadDistance(look_ahead_distance),
                                                                                                                                                                                                         WhiskersDistance(whiskers_distance),
                                                                                                                                                                                                         WhiskersAngle(whiskers_angle)
{
}

void ObstacleAvoidance::GetSteering(SteeringOutput* output)
{
	FHitResult hitResult[3];

	FVector forwardVector[3];
	forwardVector[0] = Character->Velocity; //center ray
	forwardVector[0].Normalize(0);
	forwardVector[1] = forwardVector[0].RotateAngleAxis(FMath::RadiansToDegrees(-WhiskersAngle), FVector::UpVector);
	forwardVector[2] = forwardVector[0].RotateAngleAxis(FMath::RadiansToDegrees(WhiskersAngle), FVector::UpVector);

	float lookAhead[3]{LookAheadDistance, WhiskersDistance, WhiskersDistance};

	FVector startTrace = Character->Position;
	FHitResult resultHit;
	resultHit.bBlockingHit = false;

	for (int i = 0; i < 3; ++i)
	{
		FVector endTrace = startTrace + forwardVector[i] * lookAhead[i];
		DrawDebugLine(World, startTrace, endTrace, FColor::Red, false);

		if (World->LineTraceSingleByChannel(hitResult[i], startTrace, endTrace, ECC_GameTraceChannel1))
		{
			DrawDebugPoint(World, hitResult[i].ImpactPoint, 10.f, FColor::Red);

			if (resultHit.bBlockingHit)
			{
				//choose closest hit
				float distanceCurrentResult = FVector::Dist(Character->Position, resultHit.ImpactPoint);
				float distanceHit = FVector::Dist(Character->Position, hitResult[i].ImpactPoint);
				if (distanceHit < distanceCurrentResult)
					resultHit = hitResult[i];
			}
			else
				resultHit = hitResult[i];
		}
	}
	if (!resultHit.bBlockingHit)
		return;

	FVector target = resultHit.ImpactPoint + resultHit.Normal * AvoidDistance;
	DrawDebugDirectionalArrow(World, resultHit.ImpactPoint, target, 10000.f, FColor::Red);
	target.Z = 0.f;
	TargetPosition = &target;
	Seek::GetSteering(output);
}

Align::Align(Kinematic* character, Kinematic* target, float max_angular_acceleration, float max_rotation, float target_radius, float slow_radius, float time_to_target): SteeringBehaviour(character),
                                                                                                                                                                         Target(target),
                                                                                                                                                                         MaxAngularAcceleration(max_angular_acceleration),
                                                                                                                                                                         MaxRotation(max_rotation),
                                                                                                                                                                         TargetRadius(target_radius),
                                                                                                                                                                         SlowRadius(slow_radius),
                                                                                                                                                                         TimeToTarget(time_to_target)
{
}

BlendedSteering::BlendedSteering(Kinematic* character, float max_acceleration, float max_rotation): SteeringBehaviour(character), MaxAcceleration(max_acceleration), MaxRotation(max_rotation)
{
}

void BlendedSteering::GetSteering(SteeringOutput* output)
{
	SteeringOutput tmp;
	for (int i = 0; i < Behaviours.size(); ++i)
	{
		tmp.Clear();
		Behaviours[i].Behaviour->SetCharacter(Character);
		Behaviours[i].Behaviour->GetSteering(&tmp);
		output->Linear += Behaviours[i].Weight * tmp.Linear;
		output->Angular += Behaviours[i].Weight * tmp.Angular;
	}

	FVector linear = output->Linear;
	linear.Normalize(0);
	linear *= MaxAcceleration;
	output->Linear = output->Linear.Size() < linear.Size() ? output->Linear : linear;
	output->Angular = FMath::Min(output->Angular, MaxRotation);
}

PrioritySteering::PrioritySteering(Kinematic* character, float epsilon): SteeringBehaviour(character),
                                                                         Epsilon(epsilon)
{
}

void PrioritySteering::GetSteering(SteeringOutput* output)
{
	float epSquared = FMath::Square(Epsilon);

	output->Clear();

	for (int i = 0; i < Behaviours.size(); ++i)
	{
		Behaviours[i].SetCharacter(Character);
		Behaviours[i].GetSteering(output);

		if (output->SquareMagnitude() > epSquared)
		{
			return;
		}
	}
}
