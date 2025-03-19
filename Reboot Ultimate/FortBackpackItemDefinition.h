#pragma once

#include "FortPawn.h"
#include "reboot.h"

class UFortBackpackItemDefinition : public AFortPawn
{
public:
	static UClass* StaticClass()
	{
		static auto Class = FindObject<UClass>(L"/Script/FortniteGame.FortBackpackItemDefinition");
		return Class;
	}
};

class UCustomCharacterPart : public AFortPawn
{
public:
	static UClass* StaticClass()
	{
		static auto Class = FindObject<UClass>(L"/Script/FortniteGame.CustomCharacterPart");
		return Class;
	}
};