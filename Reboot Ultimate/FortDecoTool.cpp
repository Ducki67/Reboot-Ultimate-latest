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

void AFortDecoTool::ServerCreateBuildingAndSpawnDeco(AFortDecoTool* DecoTool, FFrame& Stack, void* Ret)
{
    AFortPlayerPawn* Pawn = Cast<AFortPlayerPawn>(DecoTool->GetOwner());

    if (!Pawn)
        return;

    AFortPlayerControllerAthena* PlayerControllerAthena = Cast<AFortPlayerControllerAthena>(Pawn->GetController());

    if (!PlayerControllerAthena)
        return;

    UFortDecoItemDefinition* TrapItemDefinition = Cast<UFortDecoItemDefinition>(DecoTool->GetItemDefinition());

    if (!TrapItemDefinition)
        return;

    AFortPlayerStateAthena* PlayerStateAthena = Cast<AFortPlayerStateAthena>(Pawn->GetPlayerState());

    if (!PlayerStateAthena)
        return;

    TSubclassOf<ABuildingActor> BlueprintClass = TrapItemDefinition->GetBlueprintClass();

    if (!BlueprintClass.Get())
        return;

    auto Tool = DecoTool->GetItemDefinition();

    FVector BuildingLocation;
    FRotator BuildingRotation;
    FVector Location;
    FRotator Rotation;
    EBuildingAttachmentType InBuildingAttachmentType;
    // bool bSpawnDecoOnExtraPiece;
    // FVector BuildingExtraPieceLocation;
    Stack.StepCompiledIn(&BuildingLocation);
    Stack.StepCompiledIn(&BuildingRotation);
    Stack.StepCompiledIn(&Location);
    Stack.StepCompiledIn(&Rotation);
    Stack.StepCompiledIn(&InBuildingAttachmentType);
    // Stack.StepCompiledIn(&bSpawnDecoOnExtraPiece);
    // Stack.StepCompiledIn(&BuildingExtraPieceLocation);
    Stack.IncrementCode();

    UClass* BuildingClass = nullptr;
    FVector BuildLocation;
    FRotator BuildRotator{};

    FTransform Transform{};
    Transform.Translation = BuildLocation;
    Transform.Rotation = BuildRotator.Quaternion();
    Transform.Scale3D = { 1, 1, 1 };

    auto WorldInventory = PlayerControllerAthena->GetWorldInventory();

    auto BuildingActor = GetWorld()->SpawnActor<ABuildingSMActor>(BuildingClass, Transform);

    auto MatDefinition = UFortKismetLibrary::K2_GetResourceItemDefinition(BuildingActor->GetResourceType());

    UFortItem* MatInstance = WorldInventory->FindItemInstance(MatDefinition);

    std::wstring MatName;
    std::wstring MatNameShort;

    switch ((int)MatInstance)
    {
    case 0:
        MatName = L"Wood";
        MatNameShort = L"W";
        break;
    case 1:
        MatName = L"Stone";
        MatNameShort = L"S";
        break;
    case 2:
        MatName = L"Metal";
        MatNameShort = L"M";
        break;
    }

    std::wstring BuildingType;

    switch ((int)InBuildingAttachmentType)
    {
    case 0:
    case 6:
    case 7:
    case 2:
        BuildingType = L"Floor";
        break;
    case 1:
        BuildingType = L"Solid";
        break;
    }

    auto BuildingSMActor = Cast<ABuildingSMActor>(BuildingActor);

    ABuildingSMActor* Building = nullptr;
    TArray<ABuildingSMActor*> RemoveBuildings;
    char _Unknown;

    static __int64 (*CantBuild)(UObject*, UObject*, FVector, FRotator, char, TArray<ABuildingSMActor*>*, char*) = decltype(CantBuild)(Addresses::CantBuild);

    if (CantBuild(GetWorld(), BuildingSMActor, BuildingLocation, BuildingRotation, false, &RemoveBuildings, &_Unknown))
        return;

    auto ItemEntry = MatInstance->GetItemEntry();
    
    if (!ItemEntry)
        return;

    ItemEntry->GetCount() -= 10;

    bool bRemovedSuccessfully = false;

    if (ItemEntry->GetCount() <= 0)
    {
        WorldInventory->RemoveItem(MatInstance->GetItemEntry()->GetItemGuid(), &bRemovedSuccessfully, 10, true, false);
    }

    for (auto& RemoveBuilding : RemoveBuildings) RemoveBuilding->K2_DestroyActor();
    RemoveBuildings.Free();

    Building->SetPlayerPlaced(true);
    Building->InitializeBuildingActor(Building, nullptr, true, nullptr);

    ABuildingActor* (*SpawnDeco)(AFortDecoTool * DecoTool, UClass * Class, const FVector & Location, const FRotator & Rotation, ABuildingSMActor * AttachedActor, int a6, EBuildingAttachmentType InBuildingAttachmentType) = decltype(SpawnDeco)(0x69abec4 + uintptr_t(GetModuleHandle(0)));
    ABuildingActor* BuildingTrap = SpawnDeco(DecoTool, BlueprintClass, Location, Rotation, Building, 0, InBuildingAttachmentType);

    if (BuildingTrap)
    {
        BuildingTrap->SetTeam(PlayerStateAthena->GetTeamIndex());
    }
}