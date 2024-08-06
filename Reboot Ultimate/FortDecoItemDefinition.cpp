#include "FortDecoItemDefinition.h"

UClass* UFortDecoItemDefinition::StaticClass()
{
	static auto ptr = FindObject<UClass>(L"/Script/FortniteGame.FortDecoItemDefinition");
	return ptr;
}