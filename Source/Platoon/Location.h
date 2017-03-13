// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

struct SteeringOutput
{
	FVector Linear;
	float Angular;

	SteeringOutput() : Linear(FVector::ZeroVector), Angular(0) {}
	SteeringOutput(const FVector& linear, const float& angular) : Linear(linear), Angular(angular) {}

	bool operator ==(const SteeringOutput& other) const;
	bool operator !=(const SteeringOutput& other) const;
	
	void Clear();
	float SquareMagnitude() const;
	float Magnitude() const;
};

struct Location
{
	FVector Position;
	float Orientation;

	Location() : Position(FVector::ZeroVector), Orientation(0.0f) {}
	Location(const FVector& position) : Position(position), Orientation(0.0f) {}
	Location(const FVector& position, const float& orientation) : Position(position), Orientation(orientation) {}
	Location(const float & x, const float& y, const float& z, const float& orientation) : Position(x, y, z), Orientation(orientation) {}

	bool operator ==(const Location& other) const;
	bool operator !=(const Location& other) const;

	void Clear();
	void Integrate(const FVector& velocity, const float& rotation, const float& deltaTime);
	FVector GetOrientationAsVector() const;
};

struct Kinematic : Location
{
	FVector Velocity;
	float Rotation;

	Kinematic() : Location(), Velocity(0, 0, 0), Rotation(0) {}        
	Kinematic(const FVector& position, const FVector& velocity) : Location(position), Velocity(velocity), Rotation(0) {}
	Kinematic(const Location& loc, const FVector& velocity) : Location(loc), Velocity(velocity), Rotation(0) {}
	Kinematic(const Location& loc) : Location(loc), Velocity(0, 0, 0), Rotation(0)	{}
	Kinematic(const FVector& position, const float& orientation, const FVector& velocity, const float& rotation) : Location(position, orientation), Velocity(velocity), Rotation(rotation) {}

	Kinematic& operator =(const Location& other);
	Kinematic& operator =(const Kinematic& other);
	bool operator ==(const Kinematic& other) const;
	bool operator !=(const Kinematic& other) const;
	bool operator <(const Kinematic& other) const;
	void operator += (const Kinematic& other);
	void operator -= (const Kinematic& other);
	void operator *= (float f);

	void Clear();
	void Integrate(float deltaTime);
	void Integrate(const SteeringOutput& steer, const float& deltaTime);
	void TrimMaxSpeed(const float& speed);
};