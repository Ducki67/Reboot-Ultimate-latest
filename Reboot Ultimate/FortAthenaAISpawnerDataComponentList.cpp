#include "FortAthenaAISpawnerDataComponentList.h"

TArray<UFortAthenaAISpawnerDataComponent*> UFortAthenaAISpawnerDataComponentList::GetList()
{
	static auto GetListFn = FindObject<UFunction>(L"/Script/FortniteGame.FortAthenaAISpawnerDataComponentList.GetList");
	TArray<UFortAthenaAISpawnerDataComponent*> ReturnValue;
	this->ProcessEvent(GetListFn, &ReturnValue);
	return ReturnValue;
}