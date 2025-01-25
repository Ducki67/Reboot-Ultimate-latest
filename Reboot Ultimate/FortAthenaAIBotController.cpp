#include "FortAthenaAIBotController.h"

#include "gui.h"
#include "FortAthenaMutator_InventoryOverride.h"
#include "FortAthenaMutator_ItemDropOnDeath.h"
#include "bots.h"
#include <algorithm>
#include "FortInventory.h"

void AFortAthenaAIBotController::SwitchTeam(uint8 TeamIndex)
{
	static auto SwitchTeamFn = FindObject<UFunction>(L"/Script/FortniteGame.FortAthenaAIBotController.SwitchTeam");
	this->ProcessEvent(SwitchTeamFn, &TeamIndex);
}

void AFortAthenaAIBotController::AddDigestedSkillSets()
{
	for (int i = 0; i < this->GetDigestedBotSkillSets().Num(); i++)
	{
		auto CurrentDigestedBotSkillSet = this->GetDigestedBotSkillSets().at(i);

		UClass* AimingDigestedSkillSetClass = FindObject<UClass>(L"/Script/FortniteGame.FortAthenaAIBotAimingDigestedSkillSet");
		UClass* HarvestDigestedSkillSetClass = FindObject<UClass>(L"/Script/FortniteGame.FortAthenaAIBotHarvestDigestedSkillSet");
		UClass* InventoryDigestedSkillSetClass = FindObject<UClass>(L"/Script/FortniteGame.FortAthenaAIBotInventoryDigestedSkillSet");
		UClass* LootingDigestedSkillSetClass = FindObject<UClass>(L"/Script/FortniteGame.FortAthenaAIBotLootingDigestedSkillSet");
		UClass* MovementDigestedSkillSetClass = FindObject<UClass>(L"/Script/FortniteGame.FortAthenaAIBotMovementDigestedSkillSet");
		UClass* PerceptionDigestedSkillSetClass = FindObject<UClass>(L"/Script/FortniteGame.FortAthenaAIBotPerceptionDigestedSkillSet");
		UClass* PlayStyleDigestedSkillSetClass = FindObject<UClass>(L"/Script/FortniteGame.FortAthenaAIBotPlayStyleDigestedSkillSet");

		if (CurrentDigestedBotSkillSet->IsA(AimingDigestedSkillSetClass))
			this->Get("CacheAimingDigestedSkillSet") = CurrentDigestedBotSkillSet;
		if (CurrentDigestedBotSkillSet->IsA(HarvestDigestedSkillSetClass))
			this->Get("CacheHarvestDigestedSkillSet") = CurrentDigestedBotSkillSet;
		if (CurrentDigestedBotSkillSet->IsA(InventoryDigestedSkillSetClass))
			this->Get("CacheInventoryDigestedSkillSet") = CurrentDigestedBotSkillSet;
		if (CurrentDigestedBotSkillSet->IsA(LootingDigestedSkillSetClass))
			this->Get("CacheLootingSkillSet") = CurrentDigestedBotSkillSet;
		if (CurrentDigestedBotSkillSet->IsA(MovementDigestedSkillSetClass))
			this->Get("CacheMovementSkillSet") = CurrentDigestedBotSkillSet;
		if (CurrentDigestedBotSkillSet->IsA(PerceptionDigestedSkillSetClass))
			this->Get("CachePerceptionDigestedSkillSet") = CurrentDigestedBotSkillSet;
		if (CurrentDigestedBotSkillSet->IsA(PlayStyleDigestedSkillSetClass))
			this->Get("CachePlayStyleSkillSet") = CurrentDigestedBotSkillSet;
	}
}

void AFortAthenaAIBotController::GiveItem(UFortItemDefinition* ItemDefinition, int Count)
{
	auto Inventory = this->GetInventory();

	if (!Inventory)
		return;

	UFortItem* Item = CreateItemInstance(this, ItemDefinition, Count);

	if (auto WeaponDefinition = Cast<UFortWeaponItemDefinition>(ItemDefinition))
		Item->GetItemEntry()->GetLoadedAmmo() = WeaponDefinition->GetClipSize();

	Inventory->GetItemList().GetReplicatedEntries().Add(*Item->GetItemEntry(), FFortItemEntry::GetStructSize());
	Inventory->GetItemList().MarkItemDirty(Item->GetItemEntry());
	Inventory->HandleInventoryLocalUpdate();

	// if (auto WeaponDefinition = Cast<UFortWeaponItemDefinition>(ItemDefinition))
		// this->GetPlayerBotPawn()->EquipWeaponDefinition(WeaponDefinition, Item->GetItemEntry()->GetItemGuid());
}

void AFortAthenaAIBotController::OnPossesedPawnDiedHook(AFortAthenaAIBotController* PlayerController, AActor* DamagedActor, float Damage, AController* InstigatedBy, AActor* DamageCauser, FVector HitLocation, UObject* FHitComponent, FName BoneName, FVector Momentum, void* DeathReport)
{
	LOG_INFO(LogDev, "OnPossesedPawnDiedHook!");

	auto GameMode = Cast<AFortGameModeAthena>(GetWorld()->GetGameMode());
	auto GameState = Cast<AFortGameStateAthena>(((AFortGameMode*)GetWorld()->GetGameMode())->GetGameState());

	auto DeadPawn = Cast<AFortPlayerPawn>(PlayerController->GetPawn());
	auto DeadPlayerState = Cast<AFortPlayerStateAthena>(PlayerController->GetPlayerState());

	AFortPlayerPawn* KillerPawn = nullptr;
	AFortPlayerStateAthena* KillerPlayerState = nullptr;
	AFortPlayerControllerAthena* KillerController = nullptr;
	AFortPlayerControllerAthena* KillerPC = nullptr;

	if (InstigatedBy)
	{
		KillerController = Cast<AFortPlayerControllerAthena>(InstigatedBy);

		if (KillerController)
		{
			KillerPawn = Cast<AFortPlayerPawn>(KillerController->GetPawn());
			KillerPlayerState = Cast<AFortPlayerStateAthena>(KillerController->GetPlayerState());

			if (KillerPlayerState)
			{
				KillerPC = Cast<AFortPlayerControllerAthena>(KillerPlayerState->GetOwner());
			}
		}
	}

	if (Globals::AmountOfHealthSiphon != 0)
	{
		if (KillerPawn && KillerPawn != DeadPawn && KillerPlayerState && KillerPlayerState != DeadPlayerState && KillerPC)
		{
			auto WorldInventory = KillerController->GetWorldInventory();

			if (!WorldInventory)
				return;

			static auto WoodItemData = FindObject<UFortItemDefinition>(L"/Game/Items/ResourcePickups/WoodItemData.WoodItemData");
			static auto StoneItemData = FindObject<UFortItemDefinition>(L"/Game/Items/ResourcePickups/StoneItemData.StoneItemData");
			static auto MetalItemData = FindObject<UFortItemDefinition>(L"/Game/Items/ResourcePickups/MetalItemData.MetalItemData");

			int MaxWood = WoodItemData->GetMaxStackSize();
			int MaxStone = StoneItemData->GetMaxStackSize();
			int MaxMetal = MetalItemData->GetMaxStackSize();

			auto WoodInstance = WorldInventory->FindItemInstance(WoodItemData);
			auto WoodCount = WoodInstance ? WoodInstance->GetItemEntry()->GetCount() : 0;

			auto StoneInstance = WorldInventory->FindItemInstance(StoneItemData);
			auto StoneCount = StoneInstance ? StoneInstance->GetItemEntry()->GetCount() : 0;

			auto MetalInstance = WorldInventory->FindItemInstance(MetalItemData);
			auto MetalCount = MetalInstance ? MetalInstance->GetItemEntry()->GetCount() : 0;

			if (WoodCount < MaxWood)
			{
				int WoodToAdd = FMath::Min(50, MaxWood - WoodCount);
				WorldInventory->AddItem(WoodItemData, nullptr, WoodToAdd);
			}
			if (StoneCount < MaxStone)
			{
				int StoneToAdd = FMath::Min(50, MaxStone - StoneCount);
				WorldInventory->AddItem(StoneItemData, nullptr, StoneToAdd);
			}
			if (MetalCount < MaxMetal)
			{
				int MetalToAdd = FMath::Min(50, MaxMetal - MetalCount);
				WorldInventory->AddItem(MetalItemData, nullptr, MetalToAdd);
			}

			WorldInventory->Update();

			float Health = KillerPawn->GetHealth();
			float Shield = KillerPawn->GetShield();

			float MaxHealth = KillerPawn->GetMaxHealth();
			float MaxShield = KillerPawn->GetMaxShield();

			float AmountGiven = 0;

			if ((MaxHealth - Health) > 0)
			{
				float AmountToGive = MaxHealth - Health >= Globals::AmountOfHealthSiphon ? Globals::AmountOfHealthSiphon : MaxHealth - Health;
				KillerPawn->SetHealth(Health + AmountToGive);
				AmountGiven += AmountToGive;
			}

			if ((MaxShield - Shield) >= 0 && AmountGiven < Globals::AmountOfHealthSiphon)
			{
				if (DeathReport && Globals::bUseSiphon)
				{
					KillerPC->SiphonEffect(DeathReport, KillerPC);
				}

				if (MaxShield - Shield > 0)
				{
					float AmountToGive = MaxShield - Shield >= Globals::AmountOfHealthSiphon ? Globals::AmountOfHealthSiphon : MaxShield - Shield;
					AmountToGive -= AmountGiven;

					if (AmountToGive > 0)
					{
						KillerPawn->SetShield(Shield + AmountToGive);
						AmountGiven += AmountToGive;
					}
				}
			}
		}
	}

	// if (!InstigatedBy)
		// return OnPossesedPawnDiedOriginal(PlayerController, DamagedActor, Damage, InstigatedBy, DamageCauser, HitLocation, FHitComponent, BoneName, Momentum);

	for (int i = 0; i < AllPlayerBotsToTick.size(); i++)
	{
		auto PlayerBot = AllPlayerBotsToTick[i];

		if (Cast<AController>(PlayerBot.AIBotController) == PlayerController)
		{
			PlayerBot.OnDied(Cast<AFortPlayerStateAthena>(InstigatedBy ? InstigatedBy->GetPlayerState() : nullptr));
			AllPlayerBotsToTick.erase(AllPlayerBotsToTick.begin() + i);
		}
	}

	for (int i = 0; i < AllBossesToTick.size(); i++)
	{
		auto Boss = AllBossesToTick[i];

		if (Cast<AController>(Boss.Controller) == PlayerController)
		{
			Boss.OnDied(Cast<AFortPlayerStateAthena>(InstigatedBy ? InstigatedBy->GetPlayerState() : nullptr));
			AllBossesToTick.erase(AllBossesToTick.begin() + i);
		}
	}
}

void AFortAthenaAIBotController::OnPerceptionSensedHook(AFortAthenaAIBotController* PlayerController, AActor* SourceActor, FAIStimulus& Stim)
{
	/*if (SourceActor->IsA(AFortPlayerPawnAthena::StaticClass()) && Cast<AFortPlayerPawnAthena>(SourceActor)->GetController() && !Cast<AFortPlayerPawnAthena>(SourceActor)->GetController()->IsA(AFortAthenaAIBotController::StaticClass()))
	{
		for (auto& Boss : AllBossesToTick)
		{
			if (Boss.Controller == PlayerController)
			{
				Boss.OnPerceptionSensed(SourceActor, Stim);
			}
		}
	}

	return OnPerceptionSensedOriginal(PlayerController, SourceActor, Stim);*/
}

void AFortAthenaAIBotController::OnAlertLevelChangedHook(AFortAthenaAIBotController* PlayerController, EAlertLevel OldAlertLevel, EAlertLevel NewAlertLevel)
{
	return OnAlertLevelChangedOriginal(PlayerController, OldAlertLevel, NewAlertLevel);
}

UClass* AFortAthenaAIBotController::StaticClass()
{
	static auto Class = FindObject<UClass>(L"/Script/FortniteGame.FortAthenaAIBotController");
	return Class;
}