#pragma once

#include "Object.h"
#include "Rotator.h"
#include "Transform.h"
#include "Vector.h"
#include "Class.h"

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