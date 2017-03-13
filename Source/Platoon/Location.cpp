// Fill out your copyright notice in the Description page of Project Settings.

#include "Platoon.h"
#include "Location.h"
#include <limits>

bool SteeringOutput::operator==(const SteeringOutput& other) const
{
	return Linear == other.Linear && Angular == other.Angular;
}

bool SteeringOutput::operator!=(const SteeringOutput& other) const
{
	return Linear != other.Linear || Angular != other.Angular;
}

void SteeringOutput::Clear()
{
	Linear = FVector::ZeroVector;
	Angular = 0;
}

float SteeringOutput::SquareMagnitude() const
{
	return Linear.SizeSquared() + Angular * Angular;
}

float SteeringOutput::Magnitude() const
{
	return FMath::Sqrt(SquareMagnitude());
}

bool Location::operator==(const Location& other) const
{
	return Position == other.Position && Orientation == other.Orientation;
}

bool Location::operator!=(const Location& other) const
{
	return Position != other.Position || Orientation != other.Orientation;
}

void Location::Clear()
{
	Position = FVector::ZeroVector;
	Orientation = 0.0f;
}

void Location::Integrate(const FVector& velocity, const float& rotation, const float& deltaTime)
{
	Position += (velocity * deltaTime);
	Orientation += (rotation * deltaTime);

	Orientation = FMath::Fmod(Orientation, 2*PI);
}

FVector Location::GetOrientationAsVector() const
{
	return FVector(-FMath::Sin(Orientation), FMath::Cos(Orientation), 0);
}

Kinematic& Kinematic::operator=(const Location& other)
{
	Orientation = other.Orientation;
	Position = other.Position;
	return *this;
}

Kinematic& Kinematic::operator=(const Kinematic& other)
{
	Orientation = other.Orientation;
	Position = other.Position;
	Velocity = other.Velocity;
	Rotation = other.Rotation;
	return *this;
}

bool Kinematic::operator==(const Kinematic& other) const
{
	return Position == other.Position && Orientation == other.Orientation && Velocity == other.Velocity && Rotation == other.Rotation;
}

bool Kinematic::operator!=(const Kinematic& other) const
{
	return Position != other.Position || Orientation != other.Orientation || Velocity != other.Velocity || Rotation != other.Rotation;
}

bool Kinematic::operator<(const Kinematic& other) const
{
	return Position.X < other.Position.X;
}

void Kinematic::operator+=(const Kinematic& other)
{
	Position += other.Position;
	Velocity += other.Velocity;
	Rotation += other.Rotation;
	Orientation += other.Orientation;
}

void Kinematic::operator-=(const Kinematic& other)
{
	Position -= other.Position;
	Velocity -= other.Velocity;
	Rotation -= other.Rotation;
	Orientation -= other.Orientation;
}

void Kinematic::operator*=(float f)
{
	Position *= f;
	Velocity *= f;
	Rotation *= f;
	Orientation *= f;
}

void Kinematic::Clear()
{
	Location::Clear();
	Velocity = FVector::ZeroVector;
	Rotation = 0.0f;
}

void Kinematic::Integrate(float deltaTime)
{
	Location::Integrate(Velocity, Rotation, deltaTime);
}

void Kinematic::Integrate(const SteeringOutput& steer, const float& deltaTime)
{
	Location::Integrate(Velocity, Rotation, deltaTime);

	Velocity += steer.Linear * deltaTime;
	FVector velocityNormalized = Velocity;
	velocityNormalized.Normalize(0);
	Rotation += steer.Angular * deltaTime;
}

void Kinematic::TrimMaxSpeed(const float& speed)
{
	if(Velocity.SizeSquared() > FMath::Square(speed))
	{
		Velocity.Normalize(0);
		Velocity *= speed;
	}
}