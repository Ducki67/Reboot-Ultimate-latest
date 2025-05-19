#pragma once

#include "FortPlayerPawnAthena.h"
#include "Actor.h"
#include "Vector.h"

class AFortCharacterVehicle : public AFortPlayerPawnAthena
{
public:
    static UClass* StaticClass()
    {
        static auto Class = FindObject<UClass>("/Script/FortniteGame.FortCharacterVehicle");
        return Class;
    }
};

class AFortCharacterVehicle_Ostrich : public AFortCharacterVehicle
{
public:
    static UClass* StaticClass()
    {
        static auto Class = FindObject<UClass>("/Script/FortniteGame.FortCharacterVehicle_Ostrich");
        return Class;
    }

    void OnReleaseChargeJump(float ChargeAlpha);
};