#pragma once

#include "Object.h"
#include "Rotator.h"
#include "Transform.h"
#include "Vector.h"
#include "Class.h"

#include <string>
#include <format>
#include <iostream>
#include <random>
#include <unordered_set>
#include <math.h>

static double GetRandomDouble(float Min, float Max)
{
	std::random_device rd;
	std::mt19937 gen(rd());

	std::uniform_real_distribution<> distr(Min, Max);

	return distr(gen);
}

static float GetRandomFloat(float Min, float Max) // bruh
{
	return GetRandomDouble(Min, Max);
}

class UKismetMathLibrary : public UObject
{
public:
	static float RandomFloatInRange(float min, float max);
	static int RandomIntegerInRange(int min, int max);
	static bool RandomBoolWithWeight(float weight);
	static FRotator FindLookAtRotation(FVector start, FVector target);
	static FTransform MakeTransform(FVector location, FRotator rotation, FVector scale);

	static UClass* StaticClass();
};