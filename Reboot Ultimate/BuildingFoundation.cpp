#include "BuildingFoundation.h"
#include "FortGameModeAthena.h"
#include "LevelStreamingDynamic.h"

void ABuildingFoundation::SetDynamicFoundationTransformHook(UObject* Context, FFrame& Stack, void* Ret)
{
	FTransform NewTransform;
	Stack.StepCompiledIn(&NewTransform);

	auto BuildingFoundation = (ABuildingFoundation*)Context;

	LOG_INFO(LogDev, "SetDynamicFoundationTransformHook: {}", BuildingFoundation->GetName());

	if (std::floor(Fortnite_Version) != 13 && std::floor(Fortnite_Version) != 14)
	{
		SetFoundationTransform(BuildingFoundation, NewTransform);
	}
	else
	{
		BuildingFoundation->GetDynamicFoundationTransform() = NewTransform;

		BuildingFoundation->GetDynamicFoundationRepData().GetTranslation() = NewTransform.Translation;
		BuildingFoundation->GetDynamicFoundationRepData().GetRotation() = NewTransform.Rotation.Rotator();
		BuildingFoundation->OnRep_DynamicFoundationRepData();
	}

	return SetDynamicFoundationTransformOriginal(Context, Stack, Ret);
}

void ABuildingFoundation::SetDynamicFoundationEnabledHook(UObject* Context, FFrame& Stack, void* Ret)
{
	bool bEnabled;
	Stack.StepCompiledIn(&bEnabled);

	// LOG_INFO(LogDev, "{} TELL MILXNOR IF THIS PRINTS: {}", Context->GetFullName(), bEnabled);

	auto BuildingFoundation = (ABuildingFoundation*)Context;

	LOG_INFO(LogDev, "SetDynamicFoundationEnabledHook {}abled: {}", (bEnabled ? "en" : "dis"), BuildingFoundation->GetName());

	if (std::floor(Fortnite_Version) != 13 && std::floor(Fortnite_Version) != 14 && std::floor(Fortnite_Version) != 17)
	{
		ShowFoundation(BuildingFoundation, bEnabled);
	}
	else
	{
		if (std::floor(Fortnite_Version) == 17)
		{
			ShowFoundation(BuildingFoundation, bEnabled);
		}

		BuildingFoundation->OnRep_ServerStreamedInLevel();

		BuildingFoundation->GetDynamicFoundationRepData().GetEnabledState() = bEnabled ? EDynamicFoundationEnabledState::Enabled : EDynamicFoundationEnabledState::Disabled;
		BuildingFoundation->OnRep_DynamicFoundationRepData();

		BuildingFoundation->GetFoundationEnabledState() = bEnabled ? EDynamicFoundationEnabledState::Enabled : EDynamicFoundationEnabledState::Disabled;

		if (bEnabled)
		{
			for (const auto& AdditionalWorld : BuildingFoundation->GetAdditionalWorlds()) // this range-based 'for' statement requires a suitable "begin" function and none was f
			{
				ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(GetWorld(), AdditionalWorld, BuildingFoundation->GetActorLocation(), BuildingFoundation->GetActorRotation());
			}
		}
	}

	return SetDynamicFoundationEnabledOriginal(Context, Stack, Ret);
}