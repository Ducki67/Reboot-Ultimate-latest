#pragma once

#include "Object.h"
#include "TSubclassOf.h"
#include "Actor.h"

class UCheatManager : public UObject
{
public:
	void Teleport();
	void DestroyTarget();
	void God();
	void DestroyAll(TSubclassOf<AActor> AClass);
	void ChangeSize(float F);
	void ToggleDebugCamera();

	static UClass* StaticClass();
};