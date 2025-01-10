#include "FortDecoTool.h"
#include "FortPlayerControllerAthena.h"

void AFortDecoTool::ServerSpawnDeco(AFortDecoTool * DecoTool, FFrame & Stack, void* Ret)
{
    FVector Location;
    FRotator Rotation;
    ABuildingSMActor* AttachedActor;
    EBuildingAttachmentType InBuildingAttachmentType;

    Stack.StepCompiledIn(&Location);
    Stack.StepCompiledIn(&Rotation);
    Stack.StepCompiledIn(&AttachedActor);
    Stack.StepCompiledIn(&InBuildingAttachmentType);

    Stack.Code += Stack.Code != nullptr;

    UFortDecoItemDefinition* TrapItemDefinition = Cast<UFortDecoItemDefinition>(DecoTool->GetItemDefinition());
    AFortPlayerPawn* PlayerPawn = Cast<AFortPlayerPawn>(DecoTool->GetOwner());

    if (!TrapItemDefinition || !PlayerPawn)
        return;

    AFortPlayerControllerAthena* PlayerControllerAthena = Cast<AFortPlayerControllerAthena>(PlayerPawn->GetController());
    AFortPlayerStateAthena* PlayerStateAthena = Cast<AFortPlayerStateAthena>(PlayerPawn->GetPlayerState());
    TSubclassOf<ABuildingActor> BlueprintClass = TrapItemDefinition->GetBlueprintClass();

    if (!PlayerControllerAthena || !PlayerStateAthena || !BlueprintClass.Get())
        return;

    ABuildingActor* (*SpawnDeco)(AFortDecoTool * DecoTool, UClass * Class, const FVector & Location, const FRotator & Rotation, ABuildingSMActor * AttachedActor, int a6, EBuildingAttachmentType InBuildingAttachmentType) = decltype(SpawnDeco)(0x69abec4 + uintptr_t(GetModuleHandle(0)));
    ABuildingActor* BuildingTrap = SpawnDeco(DecoTool, BlueprintClass, Location, Rotation, AttachedActor, 0, InBuildingAttachmentType);

    if (BuildingTrap)
    {
        BuildingTrap->SetTeam(PlayerStateAthena->GetTeamIndex());
    }
}