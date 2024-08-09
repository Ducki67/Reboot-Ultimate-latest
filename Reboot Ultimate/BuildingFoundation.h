#pragma once

#include "BuildingSMActor.h"
#include "Stack.h"
#include "SoftObjectPtr.h"

enum class EDynamicFoundationType : uint8
{
	Static = 0,
	StartEnabled_Stationary = 1,
	StartEnabled_Dynamic = 2,
	StartDisabled = 3,
	EDynamicFoundationType_MAX = 4,
};

enum class EDynamicFoundationEnabledState : uint8_t
{
	Unknown = 0,
	Enabled = 1,
	Disabled = 2,
	EDynamicFoundationEnabledState_MAX = 3
};

struct FDynamicBuildingFoundationRepData
{
public:
	FRotator& GetRotation()
	{
		static auto RotationOffset = FindOffsetStruct("/Script/FortniteGame.DynamicBuildingFoundationRepData", "Rotation");
		return *(FRotator*)(__int64(this) + RotationOffset);
	}

	FVector& GetTranslation()
	{
		static auto TranslationOffset = FindOffsetStruct("/Script/FortniteGame.DynamicBuildingFoundationRepData", "Translation");
		return *(FVector*)(__int64(this) + TranslationOffset);
	}

	EDynamicFoundationEnabledState& GetEnabledState()
	{
		static auto EnabledStateOffset = FindOffsetStruct("/Script/FortniteGame.DynamicBuildingFoundationRepData", "EnabledState");
		return *(EDynamicFoundationEnabledState*)(__int64(this) + EnabledStateOffset);
	}
};

struct FBuildingFoundationStreamingData
{
public:
	FVector& GetFoundationLocation()
	{
		static auto FoundationLocationOffset = FindOffsetStruct("/Script/FortniteGame.BuildingFoundationStreamingData", "FoundationLocation");
		return *(FVector*)(__int64(this) + FoundationLocationOffset);
	}
};

class ABuildingFoundation : public ABuildingSMActor
{
public:
	static inline void (*SetDynamicFoundationEnabledOriginal)(UObject* Context, FFrame& Stack, void* Ret);
	static inline void (*SetDynamicFoundationTransformOriginal)(UObject* Context, FFrame& Stack, void* Ret);

	TArray<TSoftObjectPtr<UWorld>> GetAdditionalWorlds()
	{
		static auto AdditionalWorldsOffset = GetOffset("AdditionalWorlds");
		return Get<TArray<TSoftObjectPtr<UWorld>>>(AdditionalWorldsOffset);
	}

	EDynamicFoundationEnabledState& GetFoundationEnabledState()
	{
		static auto FoundationEnabledStateOffset = GetOffset("FoundationEnabledState");
		return Get<EDynamicFoundationEnabledState>(FoundationEnabledStateOffset);
	}

	EDynamicFoundationType& GetDynamicFoundationType()
	{
		static auto DynamicFoundationTypeOffset = GetOffset("DynamicFoundationType");
		return Get<EDynamicFoundationType>(DynamicFoundationTypeOffset);
	}

	FBuildingFoundationStreamingData& GetStreamingData()
	{
		static auto StreamingDataOffset = GetOffset("StreamingData");
		return Get<FBuildingFoundationStreamingData>(StreamingDataOffset);
	}

	FDynamicBuildingFoundationRepData& GetDynamicFoundationRepData()
	{
		static auto DynamicFoundationRepDataOffset = GetOffset("DynamicFoundationRepData");
		return Get<FDynamicBuildingFoundationRepData>(DynamicFoundationRepDataOffset);
	}

	FTransform& GetDynamicFoundationTransform()
	{
		static auto DynamicFoundationTransformOffset = GetOffset("DynamicFoundationTransform");
		return Get<FTransform>(DynamicFoundationTransformOffset);
	}

	void OnRep_DynamicFoundationRepData()
	{
		static auto OnRep_DynamicFoundationRepDataFn = FindObject<UFunction>(L"/Script/FortniteGame.BuildingFoundation.OnRep_DynamicFoundationRepData");
		this->ProcessEvent(OnRep_DynamicFoundationRepDataFn);
	}

	void OnRep_ServerStreamedInLevel()
	{
		static auto OnRep_ServerStreamedInLevelFn = FindObject<UFunction>(L"/Script/FortniteGame.BuildingFoundation.OnRep_ServerStreamedInLevel");
		this->ProcessEvent(OnRep_ServerStreamedInLevelFn);
	}

	static void SetDynamicFoundationTransformHook(UObject* Context, FFrame& Stack, void* Ret);
	static void SetDynamicFoundationEnabledHook(UObject* Context, FFrame& Stack, void* Ret);
};