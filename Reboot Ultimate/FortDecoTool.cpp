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

void AFortDecoTool::ServerCreateBuildingAndSpawnDeco(FVector BuildingLocation, FRotator BuildingRotation, FVector Location, FRotator Rotation, EBuildingAttachmentType InBuildingAttachmentType)
{
    /*static auto WoodFloorClass = LoadObject<UClass>(L"/Game/Building/ActorBlueprints/Player/Wood/L1/PBWA_W1_Floor.PBWA_W1_Floor_C");
    static auto BrickFloorClass = LoadObject<UClass>(L"/Game/Building/ActorBlueprints/Player/Stone/L1/PBWA_S1_Floor.PBWA_S1_Floor_C");
    static auto MetalFloorClass = LoadObject<UClass>(L"/Game/Building/ActorBlueprints/Player/Metal/L1/PBWA_M1_Floor.PBWA_M1_Floor_C");

    if (auto Pawn = Cast<AFortPlayerPawn>(GetOwner()))
    {
        if (auto Controller = Cast<AFortPlayerController>(Pawn->GetController()))
        {
            static auto BroadcastRemoteClientInfoOffset = Controller->GetOffset("BroadcastRemoteClientInfo");
            UObject* BroadcastRemoteClientInfo = Controller->Get(BroadcastRemoteClientInfoOffset);

            auto RemoteBuildingMaterial = BroadcastRemoteClientInfo->RemoteBuildingMaterial;

            UClass* BuildingClass = nullptr;

            switch (RemoteBuildingMaterial)
            {
            case EFortResourceType::Wood:
                BuildingClass = WoodFloorClass;
                break;

            case EFortResourceType::Stone:
                BuildingClass = BrickFloorClass;
                break;

            case EFortResourceType::Metal:
                BuildingClass = MetalFloorClass;
                break;
            }

            if (!BuildingClass)
                return;

            if (auto BuildingActor = Controller->CreateBuildingActor(BuildingLocation, BuildingRotation, false, BuildingClass))
            {
                CallServerSpawnDeco(Location, Rotation, BuildingActor, InBuildingAttachmentType);
            }
        }
    }*/
}