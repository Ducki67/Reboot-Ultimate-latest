#include "FortWeaponRangedMountedCannon.h"
#include "FortAthenaVehicle.h"
#include "FortPlayerPawnAthena.h"
#include "FortAthenaSKPushCannon.h"
#include "FortMountedCannon.h"

bool AFortWeaponRangedMountedCannon::FireActorInCannon(FVector LaunchDir, bool bIsServer)
{
    static auto InstigatorOffset = GetOffset("Instigator");
    auto Pawn = Cast<AFortPlayerPawn>(this->Get(InstigatorOffset));

    LOG_INFO(LogDev, "Pawn: {}", __int64(Pawn));
    LOG_INFO(LogDev, "LaunchDir.X: {} LaunchDir.Y: {} LaunchDir.Z: {}", LaunchDir.X, LaunchDir.Y, LaunchDir.Z);

    if (!Pawn)
        return false;

    auto Vehicle = Pawn->GetVehicle();

    LOG_INFO(LogDev, "Vehicle: {}", __int64(Vehicle));

    if (!Vehicle)
        return false;

    auto PushCannon = Cast<AFortAthenaSKPushCannon>(Vehicle);

    LOG_INFO(LogDev, "PushCannon: {}", __int64(PushCannon));

    if (!PushCannon)
    {
        auto MountedCannon = Cast<AFortMountedCannon>(Vehicle);

        if (!MountedCannon)
            return false;
    }

    auto SeatIndex = Pawn->GetVehicleSeatIndex();
    LOG_INFO(LogDev, "SeatIndex: {}", SeatIndex);

    if (SeatIndex == 1)
    {
        LOG_INFO(LogDev, "Player is in cannon seat, launching pawn instead of firing weapon.");
        PushCannon->ShootPawnOut(LaunchDir);
        return false;
    }

    return FireActorInCannon(LaunchDir, bIsServer);
}

void AFortWeaponRangedMountedCannon::ServerFireActorInCannonHook(AFortWeaponRangedMountedCannon* Cannon, FVector LaunchDir)
{
    Cannon->FireActorInCannon(LaunchDir, true);
    return;
}