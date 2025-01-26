#pragma once
#include "FortWeapon.h"
#include "Actor.h"

struct FMountedWeaponInfo
{
    bool bTargetSourceFromVehicleMuzzle = false;
    bool bNeedsVehicleAttachment = false;
};

struct FMountedWeaponInfoRepped
{
    AActor* HostVehicleCachedActor = nullptr;
    int32 HostVehicleSeatIndexCached = 0;
};

class AFortWeaponRangedForVehicle : public AFortWeapon // : public AFortWeaponRanged
{
public:
    FMountedWeaponInfo& GetMountedWeaponInfo()
    {
        static auto MountedWeaponInfoOffset = GetOffset("MountedWeaponInfo");
        return Get<FMountedWeaponInfo>(MountedWeaponInfoOffset);
    }

    FMountedWeaponInfoRepped& GetMountedWeaponInfoRepped()
    {
        static auto MountedWeaponInfoReppedOffset = GetOffset("MountedWeaponInfoRepped");
        return Get<FMountedWeaponInfoRepped>(MountedWeaponInfoReppedOffset);
    }

    static UClass* StaticClass()
    {
        static auto Class = FindObject<UClass>("/Script/FortniteGame.FortWeaponRangedForVehicle");
        return Class;
    }
};