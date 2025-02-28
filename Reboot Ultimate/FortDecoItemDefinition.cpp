#include "FortDecoItemDefinition.h"

UClass* UFortDecoItemDefinition::StaticClass()
{
	static auto ptr = FindObject<UClass>(L"/Script/FortniteGame.FortDecoItemDefinition");
	return ptr;
}
TSubclassOf<class ABuildingActor> UFortDecoItemDefinition::GetBlueprintClass()
{
    static auto GetBlueprintClassFn = FindObject<UFunction>(L"/Script/FortniteGame.FortDecoItemDefinition.GetBlueprintClass");
    struct { TSubclassOf<class ABuildingActor> ReturnValue; } FortDecoItemDefinition_GetBlueprintClass{};
    this->ProcessEvent(GetBlueprintClassFn, &FortDecoItemDefinition_GetBlueprintClass);

    return FortDecoItemDefinition_GetBlueprintClass.ReturnValue;
}