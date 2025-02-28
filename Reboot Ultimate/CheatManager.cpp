#include "CheatManager.h"

#include "reboot.h"

void UCheatManager::Teleport()
{
	static auto TeleportFn = FindObject<UFunction>(L"/Script/Engine.CheatManager.Teleport");
	this->ProcessEvent(TeleportFn);
}

void UCheatManager::DestroyTarget()
{
	static auto DestroyTargetFn = FindObject<UFunction>(L"/Script/Engine.CheatManager.DestroyTarget");
	this->ProcessEvent(DestroyTargetFn);
}

void UCheatManager::God()
{
	static auto GodFn = FindObject<UFunction>(L"/Script/Engine.CheatManager.God");
	this->ProcessEvent(GodFn, nullptr);
}

void UCheatManager::ChangeSize(float F)
{
	static auto ChangeSizeFn = FindObject<UFunction>("/Script/Engine.CheatManager.ChangeSize");

	struct
	{
		float                              F;                                                                // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	}UCheatManager_ChangeSize_Params{ F };

	this->ProcessEvent(ChangeSizeFn, &UCheatManager_ChangeSize_Params);
}

void UCheatManager::DestroyAll(TSubclassOf<AActor> AClass)
{
	static auto DestroyAllFn = FindObject<UFunction>(L"/Script/Engine.CheatManager.DestroyAll");

	struct
	{
		TSubclassOf<AActor>          AClass;                                                           //(Parm, ZeroConstructor, IsPlainOldData, NoDestructor, UObjectWrapper, HasGetValueTypeHash, NativeAccessSpecifierPublic)ic)
	}UCheatManager_DestroyAll_Params{ AClass };

	this->ProcessEvent(DestroyAllFn, &UCheatManager_DestroyAll_Params);
}

void UCheatManager::ToggleDebugCamera()
{
	static auto ToggleDebugCameraFn = FindObject<UFunction>("/Script/Engine.CheatManager.ToggleDebugCamera");
	this->ProcessEvent(ToggleDebugCameraFn);
}

UClass* UCheatManager::StaticClass()
{
	static auto Class = FindObject<UClass>(L"/Script/Engine.CheatManager");
	return Class;
}