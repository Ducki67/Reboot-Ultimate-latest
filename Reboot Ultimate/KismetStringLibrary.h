#pragma once

#include "Object.h"
#include "UnrealString.h"
#include "NameTypes.h"

class UKismetStringLibrary : public UObject
{
public:
	static FName Conv_StringToName(const FString& InString);
};