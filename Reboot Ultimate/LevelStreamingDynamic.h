#pragma once

#include "LevelStreaming.h"

#include "UnrealString.h"
#include "Rotator.h"
#include "Vector.h"

#include "reboot.h" // too lazy to make cpp file for this

class ULevelStreamingDynamic : public ULevelStreaming
{
public:
	static ULevelStreamingDynamic* LoadLevelInstance(UObject* WorldContextObject, FString LevelName, FVector Location, FRotator Rotation, bool* bOutSuccess = nullptr)
	{
		static auto LoadLevelInstanceFn = FindObject<UFunction>(L"/Script/Engine.LevelStreamingDynamic.LoadLevelInstance");

		struct
		{
			UObject* WorldContextObject;                                       // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
			FString                                     LevelName;                                                // (Parm, ZeroConstructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
			FVector                                     Location;                                                 // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
			FRotator                                    Rotation;                                                 // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, NativeAccessSpecifierPublic)
			bool                                               bOutSuccess;                                              // (Parm, OutParm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
			ULevelStreamingDynamic* ReturnValue;                                              // (Parm, OutParm, ZeroConstructor, ReturnParm, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		}ULevelStreamingDynamic_LoadLevelInstance_Params{ WorldContextObject, LevelName, Location, Rotation };

		auto defaultObj = FindObject<ULevelStreamingDynamic>(L"/Script/Engine.Default__LevelStreamingDynamic");
		defaultObj->ProcessEvent(LoadLevelInstanceFn, &ULevelStreamingDynamic_LoadLevelInstance_Params);

		if (bOutSuccess)
			*bOutSuccess = ULevelStreamingDynamic_LoadLevelInstance_Params.bOutSuccess;

		return ULevelStreamingDynamic_LoadLevelInstance_Params.ReturnValue;
	}

	static ULevelStreamingDynamic* LoadLevelInstanceBySoftObjectPtr(UObject* WorldContextObject, TSoftObjectPtr<UWorld> Level, const FVector& Location, const FRotator& Rotation, bool* bOutSuccess = nullptr, const FString& OptionalLevelNameOverride = FString())
	{
		static auto LoadLevelInstanceBySoftObjectPtrFn = FindObject<UFunction>(L"/Script/Engine.LevelStreamingDynamic.LoadLevelInstanceBySoftObjectPtr");

		struct
		{
			UObject* WorldContextObject;                                       // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
			TSoftObjectPtr<UWorld>            Level;                                                // (Parm, ZeroConstructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
			FVector                                     Location;                                                 // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
			FRotator                                    Rotation;                                                 // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, NativeAccessSpecifierPublic)
			bool                                               bOutSuccess;                                              // (Parm, OutParm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
			FString                           OptionalLevelNameOverride;                                              //(Parm, ZeroConstructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
			ULevelStreamingDynamic* ReturnValue;                                              // (Parm, OutParm, ZeroConstructor, ReturnParm, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
		}ULevelStreamingDynamic_LoadLevelInstanceBySoftObjectPtr_Params{ WorldContextObject, Level, Location, Rotation , bOutSuccess , OptionalLevelNameOverride };

		auto defaultObj = FindObject<ULevelStreamingDynamic>(L"/Script/Engine.Default__LevelStreamingDynamic");
		defaultObj->ProcessEvent(LoadLevelInstanceBySoftObjectPtrFn, &ULevelStreamingDynamic_LoadLevelInstanceBySoftObjectPtr_Params);

		if (bOutSuccess)
			*bOutSuccess = ULevelStreamingDynamic_LoadLevelInstanceBySoftObjectPtr_Params.bOutSuccess;

		return ULevelStreamingDynamic_LoadLevelInstanceBySoftObjectPtr_Params.ReturnValue;
	}
};