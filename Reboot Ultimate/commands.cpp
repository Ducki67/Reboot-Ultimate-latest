#include "commands.h"

#include "calendar.h"
#include "FortPawn.h"
#include "TSubclassOf.h"
#include "moderation.h"
#include "gui.h"
#include "BehaviorTree.h"
#include "AIBlueprintHelperLibrary.h"
#include "FortServerBotManagerAthena.h"
#include "FortAthenaAIBotSpawnerData.h"

#include <iomanip>
#include <sstream>
#include <map>
#include <string>

std::map<std::string, FVector> Waypoints;

enum class EMovementMode : uint8_t
{
	MOVE_None = 0,
	MOVE_Walking = 1,
	MOVE_NavWalking = 2,
	MOVE_Falling = 3,
	MOVE_Swimming = 4,
	MOVE_Flying = 5,
	MOVE_Custom = 6,
	MOVE_MAX = 7,
};

void ServerCheatHook(AFortPlayerControllerAthena* PlayerController, FString Msg)
{
	if (!Msg.Data.Data || Msg.Data.Num() <= 0)
		return;

	auto PlayerState = Cast<AFortPlayerStateAthena>(PlayerController->GetPlayerState());

	// std::cout << "aa!\n";

	if (!PlayerState || !IsOperator(PlayerState, PlayerController))
		return;

	std::vector<std::string> Arguments;
	auto OldMsg = Msg.ToString();

	auto ReceivingController = PlayerController; // for now
	auto ReceivingPlayerState = PlayerState; // for now

	auto firstBackslash = OldMsg.find_first_of("\\");
	auto lastBackslash = OldMsg.find_last_of("\\");

	static auto World_NetDriverOffset = GetWorld()->GetOffset("NetDriver");
	auto WorldNetDriver = GetWorld()->Get<UNetDriver*>(World_NetDriverOffset);
	auto& ClientConnections = WorldNetDriver->GetClientConnections();

	if (firstBackslash != std::string::npos && lastBackslash != std::string::npos)
	{
		if (firstBackslash != lastBackslash)
		{
			std::string player = OldMsg;

			player = player.substr(firstBackslash + 1, lastBackslash - firstBackslash - 1);

			for (int i = 0; i < ClientConnections.Num(); ++i)
			{
				static auto PlayerControllerOffset = ClientConnections.at(i)->GetOffset("PlayerController");
				auto CurrentPlayerController = Cast<AFortPlayerControllerAthena>(ClientConnections.at(i)->Get(PlayerControllerOffset));

				if (!CurrentPlayerController)
					continue;

				auto CurrentPlayerState = Cast<AFortPlayerStateAthena>(CurrentPlayerController->GetPlayerState());

				if (!CurrentPlayerState)
					continue;

				FString PlayerName = CurrentPlayerState->GetPlayerName();

				if (PlayerName.ToString() == player) // hopefully we arent on adifferent thread
				{
					ReceivingController = CurrentPlayerController;
					ReceivingPlayerState = CurrentPlayerState;
					PlayerName.Free();
					break;
				}

				PlayerName.Free();
			}
		}
		else
		{
			// SendMessageToConsole(PlayerController, L"Warning: You have a backslash but no ending backslash, was this by mistake? Executing on you.");
		}
	}

	if (!ReceivingController || !ReceivingPlayerState)
	{
		SendMessageToConsole(PlayerController, L"Unable to find player!");
		return;
	}

	{
		auto Message = Msg.ToString();

		size_t start = Message.find('\\');

		while (start != std::string::npos) // remove the playername
		{
			size_t end = Message.find('\\', start + 1);

			if (end == std::string::npos)
				break;

			Message.replace(start, end - start + 2, "");
			start = Message.find('\\');
		}

		int zz = 0;

		// std::cout << "Message Before: " << Message << '\n';

		while (Message.find(" ") != std::string::npos)
		{
			auto arg = Message.substr(0, Message.find(' '));
			Arguments.push_back(arg);
			// std::cout << std::format("[{}] {}\n", zz, arg);
			Message.erase(0, Message.find(' ') + 1);
			zz++;
		}

		// if (zz == 0)
		{
			Arguments.push_back(Message);
			// std::cout << std::format("[{}] {}\n", zz, Message);
			zz++;
		}

		// std::cout << "Message After: " << Message << '\n';
	}

	auto NumArgs = Arguments.size() == 0 ? 0 : Arguments.size() - 1;

	// std::cout << "NumArgs: " << NumArgs << '\n';

	// return;

	bool bSendHelpMessage = false;

	auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GetGameState());
	auto GameMode = Cast<AFortGameModeAthena>(GetWorld()->GetGameMode());

	if (Arguments.size() >= 1)
	{
		auto& Command = Arguments[0];
		std::transform(Command.begin(), Command.end(), Command.begin(), ::tolower);

		if (Command == "printsimulatelootdrops")
		{
			if (NumArgs < 1)
			{
				SendMessageToConsole(PlayerController, L"Please provide a LootTierGroup!");
				return;
			}

			auto& lootTierGroup = Arguments[1];

			auto LootDrops = PickLootDrops(UKismetStringLibrary::Conv_StringToName(std::wstring(lootTierGroup.begin(), lootTierGroup.end()).c_str()), -1, true);

			for (int i = 0; i < LootDrops.size(); ++i)
			{

			}

			SendMessageToConsole(PlayerController, L"Printed!");
		}
		else if (Command == "grant" || Command == "giveitem" || Command == "give")
		{
			if (NumArgs < 1)
			{
				SendMessageToConsole(PlayerController, L"Please provide an item!");
				return;
			}

			auto WorldInventory = ReceivingController->GetWorldInventory();

			if (!WorldInventory)
			{
				SendMessageToConsole(PlayerController, L"No world inventory!");
				return;
			}

			auto& weaponName = Arguments[1];

			if (weaponName == "ar_uc")
			{
				weaponName = "WID_Assault_Auto_Athena_UC_Ore_T03";
			}
			else if (weaponName == "ar_r" || weaponName == "ar")
			{
				weaponName = "WID_Assault_Auto_Athena_R_Ore_T03";
			}
			else if (weaponName == "ar_vr" || weaponName == "scar_vr")
			{
				weaponName = "WID_Assault_AutoHigh_Athena_VR_Ore_T03";
			}
			else if (weaponName == "ar_sr" || weaponName == "scar_sr" || weaponName == "scar")
			{
				weaponName = "WID_Assault_AutoHigh_Athena_SR_Ore_T03";
			}
			else if (weaponName == "ar_ur" || weaponName == "scar_ur" || weaponName == "skyesar" || weaponName == "skyear")
			{
				weaponName = "WID_Boss_Adventure_AR";
			}
			else if (weaponName == "minigun_vr")
			{
				weaponName = "WID_Assault_LMG_Athena_VR_Ore_T03";
			}
			else if (weaponName == "minigun_sr" || weaponName == "minigun")
			{
				weaponName = "WID_Assault_LMG_Athena_SR_Ore_T03";
			}
			else if (weaponName == "minigun_ur" || weaponName == "brutus")
			{
				weaponName = "WID_Boss_Hos_MG";
			}
			else if (weaponName == "pump_uc")
			{
				weaponName = "WID_Shotgun_Standard_Athena_C_Ore_T03";
			}
			else if (weaponName == "pump_r")
			{
				weaponName = "WID_Shotgun_Standard_Athena_UC_Ore_T03";
			}
			else if (weaponName == "pump_vr" || weaponName == "spaz_vr")
			{
				weaponName = "WID_Shotgun_Standard_Athena_VR_Ore_T03";
			}
			else if (weaponName == "pump_sr" || weaponName == "spaz_sr" || weaponName == "spaz")
			{
				weaponName = "WID_Shotgun_Standard_Athena_SR_Ore_T03";
			}
			else if (weaponName == "tac_uc")
			{
				weaponName = "WID_Shotgun_SemiAuto_Athena_R_Ore_T03";
			}
			else if (weaponName == "tac_r")
			{
				weaponName = "WID_Shotgun_SemiAuto_Athena_VR_Ore_T03";
			}
			else if (weaponName == "tac_vr")
			{
				weaponName = "WID_Shotgun_HighSemiAuto_Athena_VR_Ore_T03";
			}
			else if (weaponName == "tac_sr" || weaponName == "tac")
			{
				weaponName = "WID_Shotgun_HighSemiAuto_Athena_SR_Ore_T03";
			}
			else if (weaponName == "doublebarrel_vr")
			{
				weaponName = "WID_Shotgun_BreakBarrel_Athena_VR_Ore_T03";
			}
			else if (weaponName == "doublebarrel_sr" || weaponName == "doublebarrel")
			{
				weaponName = "WID_Shotgun_BreakBarrel_Athena_SR_Ore_T03";
			}
			else if (weaponName == "flint_c")
			{
				weaponName = "WID_Pistol_Flintlock_Athena_C";
			}
			else if (weaponName == "flint_uc" || weaponName == "flint")
			{
				weaponName = "WID_Pistol_Flintlock_Athena_UC";
			}
			else if (weaponName == "deagle_vr")
			{
				weaponName = "WID_Pistol_HandCannon_Athena_VR_Ore_T03";
			}
			else if (weaponName == "deagle_sr" || weaponName == "deagle")
			{
				weaponName = "WID_Pistol_HandCannon_Athena_SR_Ore_T03";
			}
			else if (weaponName == "heavy_r")
			{
				weaponName = "WID_Sniper_Heavy_Athena_R_Ore_T03";
			}
			else if (weaponName == "heavy_vr")
			{
				weaponName = "WID_Sniper_Heavy_Athena_VR_Ore_T03";
			}
			else if (weaponName == "heavy_sr" || weaponName == "heavy")
			{
				weaponName = "WID_Sniper_Heavy_Athena_SR_Ore_T03";
			}
			else if (weaponName == "hunting_uc")
			{
				weaponName = "WID_Sniper_NoScope_Athena_UC_Ore_T03";
			}
			else if (weaponName == "hunting_r")
			{
				weaponName = "WID_Sniper_NoScope_Athena_R_Ore_T03";
			}
			else if (weaponName == "hunting_vr")
			{
				weaponName = "WID_Sniper_NoScope_Athena_VR_Ore_T03";
			}
			else if (weaponName == "hunting_sr")
			{
				weaponName = "WID_Sniper_NoScope_Athena_SR_Ore_T03";
			}
			else if (weaponName == "bolt_c")
			{
				weaponName = "WID_Sniper_BoltAction_Scope_Athena_C_Ore_T03";
			}
			else if (weaponName == "bolt_uc")
			{
				weaponName = "WID_Sniper_BoltAction_Scope_Athena_UC_Ore_T03";
			}
			else if (weaponName == "bolt_r" || weaponName == "bolt")
			{
				weaponName = "WID_Sniper_BoltAction_Scope_Athena_R_Ore_T03";
			}
			else if (weaponName == "bolt_vr")
			{
				weaponName = "WID_Sniper_BoltAction_Scope_Athena_VR_Ore_T03";
			}
			else if (weaponName == "bolt_sr")
			{
				weaponName = "WID_Sniper_BoltAction_Scope_Athena_SR_Ore_T03";
			}
			else if (weaponName == "suppressed_vr")
			{
				weaponName = "WID_Sniper_Suppressed_Scope_Athena_VR_Ore_T03";
			}
			else if (weaponName == "suppressed_sr" || weaponName == "suppressed")
			{
				weaponName = "WID_Sniper_Suppressed_Scope_Athena_SR_Ore_T03";
			}
			else if (weaponName == "semi_uc")
			{
				weaponName = "WID_Sniper_Standard_Scope_Athena_VR_Ore_T03";
			}
			else if (weaponName == "semi_r" || weaponName == "semi")
			{
				weaponName = "WID_Sniper_Standard_Scope_Athena_SR_Ore_T03";
			}
			else if (weaponName == "stormscout_vr")
			{
				weaponName = "WID_Sniper_Weather_Athena_VR";
			}
			else if (weaponName == "stormscout_sr" || weaponName == "stormscout")
			{
				weaponName = "WID_Sniper_Weather_Athena_SR";
			}
			else if (weaponName == "lever_uc")
			{
				weaponName = "WID_Sniper_Cowboy_Athena_UC";
			}
			else if (weaponName == "lever_r")
			{
				weaponName = "WID_Sniper_Cowboy_Athena_R";
			}
			else if (weaponName == "lever_vr" || weaponName == "lever")
			{
				weaponName = "WID_Sniper_Cowboy_Athena_VR";
			}
			else if (weaponName == "lever_sr")
			{
				weaponName = "WID_Sniper_Cowboy_Athena_SR";
			}
			else if (weaponName == "hunterbolt_uc")
			{
				weaponName = "WID_Sniper_CoreSniper_Athena_UC";
			}
			else if (weaponName == "hunterbolt_r")
			{
				weaponName = "WID_Sniper_CoreSniper_Athena_R";
			}
			else if (weaponName == "hunterbolt_vr")
			{
				weaponName = "WID_Sniper_CoreSniper_Athena_VR";
			}
			else if (weaponName == "hunterbolt_sr" || weaponName == "hunterbolt")
			{
				weaponName = "WID_Sniper_CoreSniper_Athena_SR";
			}
			else if (weaponName == "firesniper" || weaponName == "dragonsbreath")
			{
				weaponName = "WID_WaffleTruck_Sniper_DragonBreath";
			}
			else if (weaponName == "exstormscout" || weaponName == "exoticstormscout")
			{
				weaponName = "WID_WaffleTruck_Sniper_StormScout";
			}
			else if (weaponName == "boom" || weaponName == "boomsniper")
			{
				weaponName = "WID_WaffleTruck_BoomSniper";
			}
			else if (weaponName == "rocket_r")
			{
				weaponName = "WID_Launcher_Rocket_Athena_R_Ore_T03";
			}
			else if (weaponName == "rocket_vr")
			{
				weaponName = "WID_Launcher_Rocket_Athena_VR_Ore_T03";
			}
			else if (weaponName == "rocket_sr" || weaponName == "rocket")
			{
				weaponName = "WID_Launcher_Rocket_Athena_SR_Ore_T03";
			}
			else if (weaponName == "pumpkin_uc")
			{
				weaponName = "WID_Launcher_Pumpkin_Athena_UC_Ore_T03";
			}
			else if (weaponName == "pumpkin_r")
			{
				weaponName = "WID_Launcher_Pumpkin_Athena_R_Ore_T03";
			}
			else if (weaponName == "pumpkin_vr")
			{
				weaponName = "WID_Launcher_Pumpkin_Athena_VR_Ore_T03";
			}
			else if (weaponName == "pumpkin_sr" || weaponName == "pumpkin")
			{
				weaponName = "WID_Launcher_Pumpkin_Athena_SR_Ore_T03";
			}
			else if (weaponName == "gl_r")
			{
				weaponName = "WID_Launcher_Grenade_Athena_R_Ore_T03";
			}
			else if (weaponName == "gl_vr")
			{
				weaponName = "WID_Launcher_Grenade_Athena_VR_Ore_T03";
			}
			else if (weaponName == "gl_sr" || weaponName == "gl")
			{
				weaponName = "WID_Launcher_Grenade_Athena_SR_Ore_T03";
			}
			else if (weaponName == "quad_vr")
			{
				weaponName = "WID_Launcher_Military_Athena_VR_Ore_T03";
			}
			else if (weaponName == "quad_sr" || weaponName == "quad" || weaponName == "quadlauncher")
			{
				weaponName = "WID_Launcher_Military_Athena_SR_Ore_T03";
			}
			else if (weaponName == "guidedmissile_vr" || weaponName == "guided_vr" || weaponName == "missile_vr")
			{
				weaponName = "WID_RC_Rocket_Athena_VR_T03";
			}
			else if (weaponName == "guidedmissile_sr" || weaponName == "guided_sr" || weaponName == "missile_sr" || weaponName == "guided")
			{
				weaponName = "WID_RC_Rocket_Athena_SR_T03";
			}
			else if (weaponName == "xenonbow" || weaponName == "xenon" || weaponName == "stwbow")
			{
				weaponName = "WID_Sniper_Neon_Bow_SR_Crystal_T04";
			}
			else if (weaponName == "kits" || weaponName == "kitslauncher")
			{
				weaponName = "WID_Launcher_Shockwave_Athena_UR_Ore_T03";
			}
			else if (weaponName == "rift" || weaponName == "rifts")
			{
				weaponName = "Athena_Rift_Item";
			}
			else if (weaponName == "crashpad" || weaponName == "crashes" || weaponName == "crash" || weaponName == "crashpads")
			{
				weaponName = "WID_Athena_AppleSun";
			}
			else if (weaponName == "chillers" || weaponName == "chiller" || weaponName == "chillergrenade")
			{
				weaponName = "Athena_IceGrenade";
			}
			else if (weaponName == "can" || weaponName == "rustycan" || weaponName == "cans")
			{
				weaponName = "WID_Athena_Bucket_Old";
			}
			else if (weaponName == "mythicgoldfish" || weaponName == "mythicfish" || weaponName == "goldfish")
			{
				weaponName = "WID_Athena_Bucket_Nice";
			}
			else if (weaponName == "stink" || weaponName == "stinkbomb" || weaponName == "stinks")
			{
				weaponName = "Athena_GasGrenade";
			}
			else if (weaponName == "shieldbubble")
			{
				weaponName = "Athena_SilverBlazer_V2";
			}
			else if (weaponName == "zaptrap")
			{
				weaponName = "Athena_ZippyTrout";
			}
			else if (weaponName == "batman")
			{
				weaponName = "WID_Badger_Grape_VR";
			}
			else if (weaponName == "spiderman")
			{
				weaponName = "WID_WestSausage_Parallel";
			}
			else if (weaponName == "shockwave" || weaponName == "shock" || weaponName == "shockwavegrenade" || weaponName == "shocks" || weaponName == "shockwaves")
			{
				weaponName = "Athena_ShockGrenade";
			}
			else if (weaponName == "impulse" || weaponName == "impulsegrenade" || weaponName == "impulses")
			{
				weaponName = "Athena_KnockGrenade";
			}
			else if (weaponName == "portafortress" || weaponName == "fortress")
			{
				weaponName = "Athena_SuperTowerGrenade_A";
			}
			else if (weaponName == "hopflop" || weaponName == "hopflopper")
			{
				weaponName = "WID_Athena_Flopper_HopFlopper";
			}
			else if (weaponName == "slurpfish")
			{
				weaponName = "WID_Athena_Flopper_Effective";
			}
			else if (weaponName == "zeropoint" || weaponName == "zeropointfish")
			{
				weaponName = "WID_Athena_Flopper_Zero";
			}
			else if (weaponName == "chugsplash" || weaponName == "chugs")
			{
				weaponName = "Athena_ChillBronco";
			}
			else if (weaponName == "minis")
			{
				weaponName = "Athena_ShieldSmall";
			}
			else if (weaponName == "bandage" || weaponName == "bandages")
			{
				weaponName = "Athena_Bandage";
			}
			else if (weaponName == "portafort" || weaponName == "paf")
			{
				weaponName = "Athena_TowerGrenade";
			}
			else if (weaponName == "c4")
			{
				weaponName = "Athena_C4";
			}
			else if (weaponName == "firefly" || weaponName == "fireflies")
			{
				weaponName = "WID_Athena_Grenade_Molotov";
			}
			else if (weaponName == "tire" || weaponName == "tires" || weaponName == "tyre")
			{
				weaponName = "ID_ValetMod_Tires_OffRoad_Thrown";
			}
			else if (weaponName == "doomgauntlets" || weaponName == "doom")
			{
				weaponName = "WID_HighTower_Date_ChainLightning_CoreBR";
			}
			else if (weaponName == "dub")
			{
				weaponName = "WID_WaffleTruck_Dub";
			}
			else if (weaponName == "hoprockdualies" || weaponName == "dualies")
			{
				weaponName = "WID_WaffleTruck_HopRockDualies";
			}
			else if (weaponName == "recon")
			{
				weaponName = "AGID_Athena_Scooter";
			}
			else if (weaponName == "jules" || weaponName == "julesgrappler" || weaponName == "julesgrap")
			{
				weaponName = "WID_Boss_GrapplingHoot";
			}
			else if (weaponName == "skye" || weaponName == "skyesgrappler" || weaponName == "skyegrap" || weaponName == "skyesgrap")
			{
				weaponName = "WID_Boss_Adventure_GH";
			}
			else if (weaponName == "captainamerica" || weaponName == "shield" || weaponName == "ca")
			{
				weaponName = "AGID_AshtonPack_Chicago";
			}
			else if (weaponName == "thorshammer" || weaponName == "thor" || weaponName == "thors")
			{
				weaponName = "AGID_AshtonPack_Turbo";
			}
			else if (weaponName == "batman" || weaponName == "batgrap")
			{
				weaponName = "WID_Badger_Grape_VR";
			}
			else if (weaponName == "flare" || Command == "flaregun")
			{
				weaponName = "WID_FringePlank_Athena_Prototype";
			}
			else if (weaponName == "grabitron")
			{
				weaponName = "WID_GravyGoblinV2_Athena";
			}
			else if (weaponName == "bouncer" || weaponName == "bouncers")
			{
				weaponName = "TID_Context_BouncePad_Athena";
			}
			else if (weaponName == "launchpad" || weaponName == "launch" || weaponName == "pad" || weaponName == "launches")
			{
				weaponName = "TID_Floor_Player_Launch_Pad_Athena";
			}
			else if (weaponName == "rocketammo" || weaponName == "rockets")
			{
				weaponName = "AmmoDataRockets";
			}
			else if (weaponName == "heavyammo")
			{
				weaponName = "AthenaAmmoDataBulletsHeavy";
			}
			else if (weaponName == "shells")
			{
				weaponName = "AthenaAmmoDataShells";
			}
			else if (weaponName == "medium" || weaponName == "mediumammo")
			{
				weaponName = "AthenaAmmoDataBulletsMedium";
			}
			else if (weaponName == "light" || weaponName == "lightammo")
			{
				weaponName = "AthenaAmmoDataBulletsLight";
			}
			else if (weaponName == "grappler" || weaponName == "grap" || weaponName == "grapple")
			{
				if (Fortnite_Version < 7.10)
				{
					weaponName = "WID_Hook_Gun_VR_Ore_T03";
				}
				else
				{
					weaponName = "WID_Hook_Gun_Slide";
				}
			}
			else if (weaponName == "presents" || weaponName == "present")
			{
				if (Fortnite_Version < 15)
				{
					weaponName = "Athena_GiftBox";
				}
				else
				{
					weaponName = "Athena_HolidayGiftBox";
				}
			}
			else if (weaponName == "balloons" || weaponName == "balloon")
			{
				if (Fortnite_Version < 7)
				{
					weaponName = "Athena_Balloons";
				}
				else
				{
					weaponName = "Athena_Balloons_Consumable";
				}
			}
			else if (weaponName == "snowman" || weaponName == "snowmen")
			{
				if (Fortnite_Version < 11)
				{
					weaponName = "Athena_SneakySnowman";
				}
				else
				{
					weaponName = "AGID_SneakySnowmanV2";
				}
			}
			else if (weaponName == "ironman" || weaponName == "iron-man")
			{
				if (Fortnite_Version < 14.00 || Fortnite_Version > 14.60)
				{
					weaponName = "AGID_AshtonPack_Indigo";
				}
				else
				{
					weaponName = "WID_HighTower_Tomato_Repulsor_CoreBR";
				}
			}
			else if (weaponName == "hunting")
			{
				if (Fortnite_Version < 12.41)
				{
					weaponName = "WID_Sniper_NoScope_Athena_R_Ore_T03";
				}
				else
				{
					weaponName = "WID_Sniper_NoScope_Athena_SR_Ore_T03";
				}
			}
			else if (weaponName == "god" || weaponName == "godgun" || weaponName == "testgod")
			{
				if (Fortnite_Version < 15.30)
				{
					weaponName = "/Game/Items/Weapons/Ranged/WIP/TestGod.TestGod";
				}
				else
				{
					weaponName = "/SaveTheWorld/Items/Weapons/Ranged/WIP/TestGod.TestGod";
				}
			}
			else if (weaponName == "pump")
			{
				if (Fortnite_Version < 6.31)
				{
					weaponName = "WID_Shotgun_Standard_Athena_UC_Ore_T03";
				}
				else
				{
					weaponName = "WID_Shotgun_Standard_Athena_SR_Ore_T03";
				}
			}
			else if (weaponName == "harpoon")
			{
				if (Fortnite_Version < 13.40)
				{
					weaponName = "WID_Athena_HappyGhost";
				}
				else
				{
					weaponName = "WID_Athena_HappyGhost_Infinite";
				}
			}

			auto WID = Cast<UFortWorldItemDefinition>(FindObject(weaponName, nullptr, ANY_PACKAGE));

			if (!WID)
			{
				SendMessageToConsole(PlayerController, L"Invalid WID! This usually means you either have the wrong name of an item, or the item doesn't exist on your version!");
				return;
			}

			int count = WID->GetMaxStackSize();

			if (weaponName == "bouncer" || weaponName == "bouncers" || weaponName == "TID_Context_BouncePad_Athena")
			{
				count = 6;
			}
			else if (weaponName == "launchpad" || weaponName == "launch" || weaponName == "pad" || weaponName == "launches" || weaponName == "TID_Floor_Player_Launch_Pad_Athena")
			{
				count = 3;
			}

			try
			{
				if (NumArgs >= 2)
					count = std::stoi(Arguments[2]);
			}
			catch (...)
			{
			}

			bool bShouldUpdate = false;
			WorldInventory->AddItem(WID, &bShouldUpdate, count);

			if (bShouldUpdate)
				WorldInventory->Update();

			SendMessageToConsole(PlayerController, L"Granted item!");
		}
		else if (Command == "togglesnowmap")
		{
			if (Calendar::HasSnowModification())
			{
				static bool bFirst = false;

				static float FullSnowValue = Calendar::GetFullSnowMapValue();
				static float NoSnowValue = 0.0f;

				bFirst ? Calendar::SetSnow(NoSnowValue) : Calendar::SetSnow(FullSnowValue);

				bFirst = !bFirst;

				SendMessageToConsole(PlayerController, L"Set full snow map!");
			}
			else
			{
				SendMessageToConsole(PlayerController, L"Version doesn't have any snow modification!");
			}
		}
		else if (Command == "op")
		{
			if (ReceivingController == PlayerController)
			{
				SendMessageToConsole(PlayerController, L"You can't op yourself!");
				return;
			}

			if (IsOp(ReceivingController))
			{
				SendMessageToConsole(PlayerController, L"Player is already operator!");
				return;
			}

			Op(ReceivingController);
			SendMessageToConsole(PlayerController, L"Granted operator to player!");
		}
		else if (Command == "deop")
		{
			if (!IsOp(ReceivingController))
			{
				SendMessageToConsole(PlayerController, L"Player is not operator!");
				return;
			}

			Deop(ReceivingController);
			SendMessageToConsole(PlayerController, L"Removed operator from player!");
		}
		else if (Command == "kick")
		{
			if (ReceivingController == PlayerController)
			{
				SendMessageToConsole(PlayerController, L"You can't kick yourself!");
				return;
			}

			FString Reason = L"You have been kicked.";

			static auto ClientReturnToMainMenu = FindObject<UFunction>(L"/Script/Engine.PlayerController.ClientReturnToMainMenu");
			ReceivingController->ProcessEvent(ClientReturnToMainMenu, &Reason);

			std::string KickedPlayerName;
			KickedPlayerName = ReceivingController->GetPlayerState()->GetPlayerName().ToString();

			std::wstringstream MsgStream;
			MsgStream << L"Successfully kicked " << std::wstring(KickedPlayerName.begin(), KickedPlayerName.end()) << L".";
			std::wstring Msg = MsgStream.str();

			SendMessageToConsole(PlayerController, Msg.c_str());
		}
		else if (Command == "ban")
		{
			if (ReceivingController == PlayerController)
			{
				SendMessageToConsole(PlayerController, L"You can't ban yourself!");
				return;
			}

			Ban(ReceivingController);

			FString Reason = L"You have been banned.";

			static auto ClientReturnToMainMenu = FindObject<UFunction>(L"/Script/Engine.PlayerController.ClientReturnToMainMenu");
			ReceivingController->ProcessEvent(ClientReturnToMainMenu, &Reason);

			std::string BannedPlayerName;
			BannedPlayerName = ReceivingController->GetPlayerState()->GetPlayerName().ToString();

			std::wstringstream MsgStream;
			MsgStream << L"Successfully banned " << std::wstring(BannedPlayerName.begin(), BannedPlayerName.end()) << L".";
			std::wstring Msg = MsgStream.str();

			SendMessageToConsole(PlayerController, Msg.c_str());
		}
		else if (Command == "givegameplayeffect" || Command == "givege")
		{
			auto PlayerState = ReceivingController->GetPlayerStateAthena();

			if (!PlayerState)
			{
				SendMessageToConsole(ReceivingController, L"No PlayerState!");
				return;
			}

			auto AbilitySystemComponent = PlayerState->GetAbilitySystemComponent();

			if (!AbilitySystemComponent)
			{
				SendMessageToConsole(ReceivingController, L"No AbilitySystemComponent!");
				return;
			}

			UClass* AbilityClassToGive = nullptr;

			if (NumArgs >= 1)
			{
				static auto BGAClass = FindObject<UClass>(L"/Script/Engine.BlueprintGeneratedClass");
				AbilityClassToGive = LoadObject<UClass>(Arguments[1], BGAClass);
			}

			AbilitySystemComponent->ApplyGameplayEffectToSelf(AbilityClassToGive, 1.f);

			SendMessageToConsole(PlayerController, L"Applied effect!");
		}
		else if (Command == "giveability" || Command == "givega")
		{
			auto PlayerState = ReceivingController->GetPlayerStateAthena();

			if (!PlayerState)
			{
				SendMessageToConsole(ReceivingController, L"No PlayerState!");
				return;
			}

			auto AbilitySystemComponent = PlayerState->GetAbilitySystemComponent();

			if (!AbilitySystemComponent)
			{
				SendMessageToConsole(ReceivingController, L"No AbilitySystemComponent!");
				return;
			}

			UClass* AbilityClassToGive = nullptr;

			if (NumArgs >= 1)
			{
				static auto BGAClass = FindObject<UClass>(L"/Script/Engine.BlueprintGeneratedClass");
				AbilityClassToGive = LoadObject<UClass>(Arguments[1], BGAClass);
			}

			AbilitySystemComponent->GiveAbilityEasy(AbilityClassToGive);

			SendMessageToConsole(PlayerController, L"Applied Ability!");
		}
		/*else if (Command == "applytag")
		{
			auto PlayerState = ReceivingController->GetPlayerStateAthena();

			if (!PlayerState)
			{
				SendMessageToConsole(ReceivingController, L"No PlayerState!");
				return;
			}

			auto AbilitySystemComponent = PlayerState->GetAbilitySystemComponent();

			if (!AbilitySystemComponent)
			{
				SendMessageToConsole(ReceivingController, L"No AbilitySystemComponent!");
				return;
			}

			if (NumArgs >= 1)
			{
				std::string TagStrToApply = Arguments[1];

				FGameplayTag TagToApply = FGameplayTag();
				TagToApply.TagName = UKismetStringLibrary::Conv_StringToName(std::wstring(TagStrToApply.begin(), TagStrToApply.end()).c_str());

				AbilitySystemComponent->NetMulticast_InvokeGameplayCueAdded(TagToApply, FPredictionKey(), AbilitySystemComponent->MakeEffectContext());
				AbilitySystemComponent->NetMulticast_InvokeGameplayCueExecuted(TagToApply, FPredictionKey(), AbilitySystemComponent->MakeEffectContext());
			}
		}*/
		else if (Command == "setpickaxe" || Command == "pickaxe")
		{
			if (NumArgs < 1)
			{
				SendMessageToConsole(PlayerController, L"Please provide a pickaxe!");
				return;
			}

			if (Fortnite_Version < 3) // Idk why but emptyslot kicks the player because of the validate.
			{
				SendMessageToConsole(PlayerController, L"Not supported on this version!");
				return;
			}

			auto WorldInventory = ReceivingController->GetWorldInventory();

			if (!WorldInventory)
			{
				SendMessageToConsole(PlayerController, L"No world inventory!");
				return;
			}

			auto& pickaxeName = Arguments[1];

			if (pickaxeName == "reaper" || pickaxeName == "scythe") // if this git ever gets released feel free to pull request open more of these i cba
			{
				pickaxeName = "WID_Harvest_HalloweenScythe_Athena_C_T01";
			}
			else if (pickaxeName == "raiders" || pickaxeName == "revenge")
			{
				pickaxeName = "WID_Harvest_Pickaxe_Lockjaw_Athena_C_T01";
			}
			else if (pickaxeName == "icebreaker" || pickaxeName == "shovel")
			{
				pickaxeName = "WID_Harvest_Pickaxe_WinterCamo_Athena";
			}
			else if (pickaxeName == "default")
			{
				pickaxeName = "WID_Harvest_Pickaxe_Athena_C_T01";
			}
			else if (pickaxeName == "candy")
			{
				pickaxeName = "WID_Harvest_Pickaxe_HolidayCandyCane_Athena";
			}
			else if (pickaxeName == "galaxy")
			{
				pickaxeName = "WID_Harvest_Pickaxe_Celestial";
			}
			else if (pickaxeName == "spectral" || pickaxeName == "prismatic")
			{
				pickaxeName = "WID_Harvest_Pickaxe_Prismatic";
			}
			else if (pickaxeName == "tat")
			{
				pickaxeName = "WID_Harvest_Pickaxe_Heart_Athena";
			}
			else if (pickaxeName == "acdc")
			{
				pickaxeName = "WID_Harvest_Pickaxe_Teslacoil_Athena";
			}
			else if (pickaxeName == "lucky")
			{
				pickaxeName = "WID_Harvest_Pickaxe_Smiley_Athena_C_T01";
			}
			else if (pickaxeName == "studded")
			{
				pickaxeName = "WID_Harvest_Pickaxe_SharpDresser";
			}
			else if (pickaxeName == "driver" || pickaxeName == "golf" || pickaxeName == "club")
			{
				pickaxeName = "WID_Harvest_Pickaxe_GolfClub";
			}
			else if (pickaxeName == "guiding" || pickaxeName == "glow" || pickaxeName == "redriding")
			{
				pickaxeName = "WID_Harvest_Pickaxe_RedRiding";
			}
			else if (pickaxeName == "coldsnap" || pickaxeName == "gnome")
			{
				pickaxeName = "WID_Harvest_Pickaxe_Gnome";
			}
			else if (pickaxeName == "vision")
			{
				pickaxeName = "WID_Harvest_Pickaxe_StreetGoth";
			}
			else if (pickaxeName == "bootstrap" || pickaxeName == "fish")
			{
				pickaxeName = "WID_Harvest_Pickaxe_TeriyakiFish";
			}
			else if (pickaxeName == "widows" || pickaxeName == "widowsbite")
			{
				pickaxeName = "WID_Harvest_Pickaxe_AshtonBoardwalk";
			}
			else if (pickaxeName == "riperippers" || pickaxeName == "bunker" || pickaxeName == "1h")
			{
				pickaxeName = "WID_Harvest_Pickaxe_BunkerMan_1H";
			}
			else if (pickaxeName == "starwand" || pickaxeName == "star" || pickaxeName == "wand")
			{
				pickaxeName = "WID_Harvest_Pickaxe_StarWand";
			}
			else if (pickaxeName == "rey" || pickaxeName == "quarterstaff")
			{
				pickaxeName = "WID_Harvest_Pickaxe_GalileoRocket";
			}
			else if (pickaxeName == "harly" || pickaxeName == "bat")
			{
				pickaxeName = "WID_Harvest_Pickaxe_LollipopTricksterFemale";
			}
			else if (pickaxeName == "fncs")
			{
				pickaxeName = "WID_Harvest_Pickaxe_FNCS";
			}
			else if (pickaxeName == "butterfly" || pickaxeName == "knives")
			{
				pickaxeName = "WID_Harvest_Pickaxe_SpyMale1H";
			}
			else if (pickaxeName == "mjolnir")
			{
				pickaxeName = "WID_Harvest_Pickaxe_HightowerTapas1H";
			}
			else if (pickaxeName == "leviathan")
			{
				pickaxeName = "WID_Harvest_Pickaxe_HistorianMale";
			}

			static auto AthenaPickaxeItemDefinitionClass = FindObject<UClass>(L"/Script/FortniteGame.AthenaPickaxeItemDefinition");

			auto Pickaxe1 = FindObject(pickaxeName + "." + pickaxeName, nullptr, ANY_PACKAGE);

			UFortWeaponMeleeItemDefinition* NewPickaxeItemDefinition = nullptr;

			if (Pickaxe1)
			{
				if (Pickaxe1->IsA(AthenaPickaxeItemDefinitionClass))
				{
					static auto WeaponDefinitionOffset = Pickaxe1->GetOffset("WeaponDefinition");
					NewPickaxeItemDefinition = Pickaxe1->Get<UFortWeaponMeleeItemDefinition*>(WeaponDefinitionOffset);
				}
				else
				{
					NewPickaxeItemDefinition = Cast<UFortWeaponMeleeItemDefinition>(Pickaxe1);
				}
			}

			if (!NewPickaxeItemDefinition)
			{
				SendMessageToConsole(PlayerController, L"Invalid pickaxe item definition!");
				return;
			}

			auto PickaxeInstance = WorldInventory->GetPickaxeInstance();

			if (PickaxeInstance)
			{
				WorldInventory->RemoveItem(PickaxeInstance->GetItemEntry()->GetItemGuid(), nullptr, PickaxeInstance->GetItemEntry()->GetCount(), true);
			}

			WorldInventory->AddItem(NewPickaxeItemDefinition, nullptr, 1);
			WorldInventory->Update();

			SendMessageToConsole(PlayerController, L"Successfully set pickaxe!");
		}
		else if (Command == "pickaxeall")
		{
			static auto World_NetDriverOffset = GetWorld()->GetOffset("NetDriver");
			auto WorldNetDriver = GetWorld()->Get<UNetDriver*>(World_NetDriverOffset);
			auto& ClientConnections = WorldNetDriver->GetClientConnections();

			for (int z = 0; z < ClientConnections.Num(); z++)
			{
				auto ClientConnection = ClientConnections.at(z);
				auto FortPC = Cast<AFortPlayerController>(ClientConnection->GetPlayerController());

				if (!FortPC)
					continue;

				auto WorldInventory = FortPC->GetWorldInventory();

				if (!WorldInventory)
				{
					SendMessageToConsole(PlayerController, L"No world inventory!");
					return;
				}

				if (NumArgs < 1)
				{
					SendMessageToConsole(PlayerController, L"Please provide a pickaxe!");
					return;
				}

				if (Fortnite_Version < 3)
				{
					SendMessageToConsole(PlayerController, L"Not supported on this version!");
					return;
				}

				auto& pickaxeName = Arguments[1];

				if (pickaxeName == "reaper" || pickaxeName == "scythe") // if this git ever gets released feel free to pull request open more of these i cba
				{
					pickaxeName = "WID_Harvest_HalloweenScythe_Athena_C_T01";
				}
				else if (pickaxeName == "raiders" || pickaxeName == "revenge")
				{
					pickaxeName = "WID_Harvest_Pickaxe_Lockjaw_Athena_C_T01";
				}
				else if (pickaxeName == "icebreaker" || pickaxeName == "shovel")
				{
					pickaxeName = "WID_Harvest_Pickaxe_WinterCamo_Athena";
				}
				else if (pickaxeName == "default")
				{
					pickaxeName = "WID_Harvest_Pickaxe_Athena_C_T01";
				}
				else if (pickaxeName == "candy")
				{
					pickaxeName = "WID_Harvest_Pickaxe_HolidayCandyCane_Athena";
				}
				else if (pickaxeName == "galaxy")
				{
					pickaxeName = "WID_Harvest_Pickaxe_Celestial";
				}
				else if (pickaxeName == "spectral" || pickaxeName == "prismatic")
				{
					pickaxeName = "WID_Harvest_Pickaxe_Prismatic";
				}
				else if (pickaxeName == "tat")
				{
					pickaxeName = "WID_Harvest_Pickaxe_Heart_Athena";
				}
				else if (pickaxeName == "acdc")
				{
					pickaxeName = "WID_Harvest_Pickaxe_Teslacoil_Athena";
				}
				else if (pickaxeName == "lucky")
				{
					pickaxeName = "WID_Harvest_Pickaxe_Smiley_Athena_C_T01";
				}
				else if (pickaxeName == "studded")
				{
					pickaxeName = "WID_Harvest_Pickaxe_SharpDresser";
				}
				else if (pickaxeName == "driver" || pickaxeName == "golf" || pickaxeName == "club")
				{
					pickaxeName = "WID_Harvest_Pickaxe_GolfClub";
				}
				else if (pickaxeName == "guiding" || pickaxeName == "glow" || pickaxeName == "redriding")
				{
					pickaxeName = "WID_Harvest_Pickaxe_RedRiding";
				}
				else if (pickaxeName == "coldsnap" || pickaxeName == "gnome")
				{
					pickaxeName = "WID_Harvest_Pickaxe_Gnome";
				}
				else if (pickaxeName == "vision")
				{
					pickaxeName = "WID_Harvest_Pickaxe_StreetGoth";
				}
				else if (pickaxeName == "bootstrap" || pickaxeName == "fish")
				{
					pickaxeName = "WID_Harvest_Pickaxe_TeriyakiFish";
				}
				else if (pickaxeName == "widows" || pickaxeName == "widowsbite")
				{
					pickaxeName = "WID_Harvest_Pickaxe_AshtonBoardwalk";
				}
				else if (pickaxeName == "riperippers" || pickaxeName == "bunker" || pickaxeName == "1h")
				{
					pickaxeName = "WID_Harvest_Pickaxe_BunkerMan_1H";
				}
				else if (pickaxeName == "starwand" || pickaxeName == "star" || pickaxeName == "wand")
				{
					pickaxeName = "WID_Harvest_Pickaxe_StarWand";
				}
				else if (pickaxeName == "rey" || pickaxeName == "quarterstaff")
				{
					pickaxeName = "WID_Harvest_Pickaxe_GalileoRocket";
				}
				else if (pickaxeName == "harly" || pickaxeName == "bat")
				{
					pickaxeName = "WID_Harvest_Pickaxe_LollipopTricksterFemale";
				}
				else if (pickaxeName == "fncs")
				{
					pickaxeName = "WID_Harvest_Pickaxe_FNCS";
				}
				else if (pickaxeName == "butterfly" || pickaxeName == "knives")
				{
					pickaxeName = "WID_Harvest_Pickaxe_SpyMale1H";
				}
				else if (pickaxeName == "mjolnir")
				{
					pickaxeName = "WID_Harvest_Pickaxe_HightowerTapas1H";
				}
				else if (pickaxeName == "leviathan")
				{
					pickaxeName = "WID_Harvest_Pickaxe_HistorianMale";
				}

				static auto AthenaPickaxeItemDefinitionClass = FindObject<UClass>(L"/Script/FortniteGame.AthenaPickaxeItemDefinition");

				auto Pickaxe1 = FindObject(pickaxeName + "." + pickaxeName, nullptr, ANY_PACKAGE);

				UFortWeaponMeleeItemDefinition* NewPickaxeItemDefinition = nullptr;

				if (Pickaxe1)
				{
					if (Pickaxe1->IsA(AthenaPickaxeItemDefinitionClass))
					{
						static auto WeaponDefinitionOffset = Pickaxe1->GetOffset("WeaponDefinition");
						NewPickaxeItemDefinition = Pickaxe1->Get<UFortWeaponMeleeItemDefinition*>(WeaponDefinitionOffset);
					}
					else
					{
						NewPickaxeItemDefinition = Cast<UFortWeaponMeleeItemDefinition>(Pickaxe1);
					}
				}

				if (!NewPickaxeItemDefinition)
				{
					SendMessageToConsole(PlayerController, L"Invalid pickaxe item definition!");
					return;
				}

				auto PickaxeInstance = WorldInventory->GetPickaxeInstance();

				if (PickaxeInstance)
				{
					WorldInventory->RemoveItem(PickaxeInstance->GetItemEntry()->GetItemGuid(), nullptr, PickaxeInstance->GetItemEntry()->GetCount(), true);
				}

				WorldInventory->AddItem(NewPickaxeItemDefinition, nullptr, 1);
				WorldInventory->Update();

				SendMessageToConsole(PlayerController, L"Successfully gave pickaxe to all players!");
			}
		}
		else if (Command == "load")
		{
			if (!Globals::bCreative)
			{
				SendMessageToConsole(PlayerController, L"You are not playing creative!");
				return;
			}

			static auto CreativePlotLinkedVolumeOffset = ReceivingController->GetOffset("CreativePlotLinkedVolume", false);
			auto Volume = CreativePlotLinkedVolumeOffset == -1 ? nullptr : ReceivingController->GetCreativePlotLinkedVolume();

			if (Arguments.size() <= 1)
			{
				SendMessageToConsole(PlayerController, L"Please provide a filename!\n");
				return;
			}

			std::string FileName = "islandSave";

			try { FileName = Arguments[1]; }
			catch (...) {}

			float X{ -1 }, Y{ -1 }, Z{ -1 };

			if (Arguments.size() >= 4)
			{
				try { X = std::stof(Arguments[2]); }
				catch (...) {}
				try { Y = std::stof(Arguments[3]); }
				catch (...) {}
				try { Z = std::stof(Arguments[4]); }
				catch (...) {}
			}
			else
			{
				if (!Volume)
				{
					SendMessageToConsole(PlayerController, L"They do not have an island!");
					return;
				}
			}

			if (X != -1 && Y != -1 && Z != -1) // omg what if they want to spawn it at -1 -1 -1!!!
				Builder::LoadSave(FileName, FVector(X, Y, Z), FRotator());
			else
				Builder::LoadSave(FileName, Volume);

			SendMessageToConsole(PlayerController, L"Loaded!");
		}
		else if (Command == "spawnpickup" || Command == "pickup")
		{
			if (NumArgs < 1)
			{
				SendMessageToConsole(PlayerController, L"Please provide a WID!");
				return;
			}

			auto Pawn = ReceivingController->GetMyFortPawn();

			if (!Pawn)
			{
				SendMessageToConsole(PlayerController, L"No pawn!");
				return;
			}

			auto& weaponName = Arguments[1];

			if (weaponName == "ar_uc")
			{
				weaponName = "WID_Assault_Auto_Athena_UC_Ore_T03";
			}
			else if (weaponName == "ar_r" || weaponName == "ar")
			{
				weaponName = "WID_Assault_Auto_Athena_R_Ore_T03";
			}
			else if (weaponName == "ar_vr" || weaponName == "scar_vr")
			{
				weaponName = "WID_Assault_AutoHigh_Athena_VR_Ore_T03";
			}
			else if (weaponName == "ar_sr" || weaponName == "scar_sr" || weaponName == "scar")
			{
				weaponName = "WID_Assault_AutoHigh_Athena_SR_Ore_T03";
			}
			else if (weaponName == "ar_ur" || weaponName == "scar_ur" || weaponName == "skyesar" || weaponName == "skyear")
			{
				weaponName = "WID_Boss_Adventure_AR";
			}
			else if (weaponName == "minigun_vr")
			{
				weaponName = "WID_Assault_LMG_Athena_VR_Ore_T03";
			}
			else if (weaponName == "minigun_sr" || weaponName == "minigun")
			{
				weaponName = "WID_Assault_LMG_Athena_SR_Ore_T03";
			}
			else if (weaponName == "minigun_ur" || weaponName == "brutus")
			{
				weaponName = "WID_Boss_Hos_MG";
			}
			else if (weaponName == "pump_uc")
			{
				weaponName = "WID_Shotgun_Standard_Athena_C_Ore_T03";
			}
			else if (weaponName == "pump_r")
			{
				weaponName = "WID_Shotgun_Standard_Athena_UC_Ore_T03";
			}
			else if (weaponName == "pump_vr" || weaponName == "spaz_vr")
			{
				weaponName = "WID_Shotgun_Standard_Athena_VR_Ore_T03";
			}
			else if (weaponName == "pump_sr" || weaponName == "spaz_sr" || weaponName == "spaz")
			{
				weaponName = "WID_Shotgun_Standard_Athena_SR_Ore_T03";
			}
			else if (weaponName == "tac_uc")
			{
				weaponName = "WID_Shotgun_SemiAuto_Athena_R_Ore_T03";
			}
			else if (weaponName == "tac_r")
			{
				weaponName = "WID_Shotgun_SemiAuto_Athena_VR_Ore_T03";
			}
			else if (weaponName == "tac_vr")
			{
				weaponName = "WID_Shotgun_HighSemiAuto_Athena_VR_Ore_T03";
			}
			else if (weaponName == "tac_sr" || weaponName == "tac")
			{
				weaponName = "WID_Shotgun_HighSemiAuto_Athena_SR_Ore_T03";
			}
			else if (weaponName == "doublebarrel_vr")
			{
				weaponName = "WID_Shotgun_BreakBarrel_Athena_VR_Ore_T03";
			}
			else if (weaponName == "doublebarrel_sr" || weaponName == "doublebarrel")
			{
				weaponName = "WID_Shotgun_BreakBarrel_Athena_SR_Ore_T03";
			}
			else if (weaponName == "flint_c")
			{
				weaponName = "WID_Pistol_Flintlock_Athena_C";
			}
			else if (weaponName == "flint_uc" || weaponName == "flint")
			{
				weaponName = "WID_Pistol_Flintlock_Athena_UC";
			}
			else if (weaponName == "deagle_vr")
			{
				weaponName = "WID_Pistol_HandCannon_Athena_VR_Ore_T03";
			}
			else if (weaponName == "deagle_sr" || weaponName == "deagle")
			{
				weaponName = "WID_Pistol_HandCannon_Athena_SR_Ore_T03";
			}
			else if (weaponName == "heavy_r")
			{
				weaponName = "WID_Sniper_Heavy_Athena_R_Ore_T03";
			}
			else if (weaponName == "heavy_vr")
			{
				weaponName = "WID_Sniper_Heavy_Athena_VR_Ore_T03";
			}
			else if (weaponName == "heavy_sr" || weaponName == "heavy")
			{
				weaponName = "WID_Sniper_Heavy_Athena_SR_Ore_T03";
			}
			else if (weaponName == "hunting_uc")
			{
				weaponName = "WID_Sniper_NoScope_Athena_UC_Ore_T03";
			}
			else if (weaponName == "hunting_r")
			{
				weaponName = "WID_Sniper_NoScope_Athena_R_Ore_T03";
			}
			else if (weaponName == "hunting_vr")
			{
				weaponName = "WID_Sniper_NoScope_Athena_VR_Ore_T03";
			}
			else if (weaponName == "hunting_sr")
			{
				weaponName = "WID_Sniper_NoScope_Athena_SR_Ore_T03";
			}
			else if (weaponName == "bolt_c")
			{
				weaponName = "WID_Sniper_BoltAction_Scope_Athena_C_Ore_T03";
			}
			else if (weaponName == "bolt_uc")
			{
				weaponName = "WID_Sniper_BoltAction_Scope_Athena_UC_Ore_T03";
			}
			else if (weaponName == "bolt_r" || weaponName == "bolt")
			{
				weaponName = "WID_Sniper_BoltAction_Scope_Athena_R_Ore_T03";
			}
			else if (weaponName == "bolt_vr")
			{
				weaponName = "WID_Sniper_BoltAction_Scope_Athena_VR_Ore_T03";
			}
			else if (weaponName == "bolt_sr")
			{
				weaponName = "WID_Sniper_BoltAction_Scope_Athena_SR_Ore_T03";
			}
			else if (weaponName == "suppressed_vr")
			{
				weaponName = "WID_Sniper_Suppressed_Scope_Athena_VR_Ore_T03";
			}
			else if (weaponName == "suppressed_sr" || weaponName == "suppressed")
			{
				weaponName = "WID_Sniper_Suppressed_Scope_Athena_SR_Ore_T03";
			}
			else if (weaponName == "semi_uc")
			{
				weaponName = "WID_Sniper_Standard_Scope_Athena_VR_Ore_T03";
			}
			else if (weaponName == "semi_r" || weaponName == "semi")
			{
				weaponName = "WID_Sniper_Standard_Scope_Athena_SR_Ore_T03";
			}
			else if (weaponName == "stormscout_vr")
			{
				weaponName = "WID_Sniper_Weather_Athena_VR";
			}
			else if (weaponName == "stormscout_sr" || weaponName == "stormscout")
			{
				weaponName = "WID_Sniper_Weather_Athena_SR";
			}
			else if (weaponName == "lever_uc")
			{
				weaponName = "WID_Sniper_Cowboy_Athena_UC";
			}
			else if (weaponName == "lever_r")
			{
				weaponName = "WID_Sniper_Cowboy_Athena_R";
			}
			else if (weaponName == "lever_vr" || weaponName == "lever")
			{
				weaponName = "WID_Sniper_Cowboy_Athena_VR";
			}
			else if (weaponName == "lever_sr")
			{
				weaponName = "WID_Sniper_Cowboy_Athena_SR";
			}
			else if (weaponName == "hunterbolt_uc")
			{
				weaponName = "WID_Sniper_CoreSniper_Athena_UC";
			}
			else if (weaponName == "hunterbolt_r")
			{
				weaponName = "WID_Sniper_CoreSniper_Athena_R";
			}
			else if (weaponName == "hunterbolt_vr")
			{
				weaponName = "WID_Sniper_CoreSniper_Athena_VR";
			}
			else if (weaponName == "hunterbolt_sr" || weaponName == "hunterbolt")
			{
				weaponName = "WID_Sniper_CoreSniper_Athena_SR";
			}
			else if (weaponName == "firesniper" || weaponName == "dragonsbreath")
			{
				weaponName = "WID_WaffleTruck_Sniper_DragonBreath";
			}
			else if (weaponName == "exstormscout" || weaponName == "exoticstormscout")
			{
				weaponName = "WID_WaffleTruck_Sniper_StormScout";
			}
			else if (weaponName == "boom" || weaponName == "boomsniper")
			{
				weaponName = "WID_WaffleTruck_BoomSniper";
			}
			else if (weaponName == "rocket_r")
			{
				weaponName = "WID_Launcher_Rocket_Athena_R_Ore_T03";
			}
			else if (weaponName == "rocket_vr")
			{
				weaponName = "WID_Launcher_Rocket_Athena_VR_Ore_T03";
			}
			else if (weaponName == "rocket_sr" || weaponName == "rocket")
			{
				weaponName = "WID_Launcher_Rocket_Athena_SR_Ore_T03";
			}
			else if (weaponName == "pumpkin_uc")
			{
				weaponName = "WID_Launcher_Pumpkin_Athena_UC_Ore_T03";
			}
			else if (weaponName == "pumpkin_r")
			{
				weaponName = "WID_Launcher_Pumpkin_Athena_R_Ore_T03";
			}
			else if (weaponName == "pumpkin_vr")
			{
				weaponName = "WID_Launcher_Pumpkin_Athena_VR_Ore_T03";
			}
			else if (weaponName == "pumpkin_sr" || weaponName == "pumpkin")
			{
				weaponName = "WID_Launcher_Pumpkin_Athena_SR_Ore_T03";
			}
			else if (weaponName == "gl_r")
			{
				weaponName = "WID_Launcher_Grenade_Athena_R_Ore_T03";
			}
			else if (weaponName == "gl_vr")
			{
				weaponName = "WID_Launcher_Grenade_Athena_VR_Ore_T03";
			}
			else if (weaponName == "gl_sr" || weaponName == "gl")
			{
				weaponName = "WID_Launcher_Grenade_Athena_SR_Ore_T03";
			}
			else if (weaponName == "quad_vr")
			{
				weaponName = "WID_Launcher_Military_Athena_VR_Ore_T03";
			}
			else if (weaponName == "quad_sr" || weaponName == "quad" || weaponName == "quadlauncher")
			{
				weaponName = "WID_Launcher_Military_Athena_SR_Ore_T03";
			}
			else if (weaponName == "guidedmissile_vr" || weaponName == "guided_vr" || weaponName == "missile_vr")
			{
				weaponName = "WID_RC_Rocket_Athena_VR_T03";
			}
			else if (weaponName == "guidedmissile_sr" || weaponName == "guided_sr" || weaponName == "missile_sr" || weaponName == "guided")
			{
				weaponName = "WID_RC_Rocket_Athena_SR_T03";
			}
			else if (weaponName == "xenonbow" || weaponName == "xenon" || weaponName == "stwbow")
			{
				weaponName = "WID_Sniper_Neon_Bow_SR_Crystal_T04";
			}
			else if (weaponName == "kits" || weaponName == "kitslauncher")
			{
				weaponName = "WID_Launcher_Shockwave_Athena_UR_Ore_T03";
			}
			else if (weaponName == "rift" || weaponName == "rifts")
			{
				weaponName = "Athena_Rift_Item";
			}
			else if (weaponName == "crashpad" || weaponName == "crashes" || weaponName == "crash" || weaponName == "crashpads")
			{
				weaponName = "WID_Athena_AppleSun";
			}
			else if (weaponName == "chillers" || weaponName == "chiller" || weaponName == "chillergrenade")
			{
				weaponName = "Athena_IceGrenade";
			}
			else if (weaponName == "can" || weaponName == "rustycan" || weaponName == "cans")
			{
				weaponName = "WID_Athena_Bucket_Old";
			}
			else if (weaponName == "mythicgoldfish" || weaponName == "mythicfish" || weaponName == "goldfish")
			{
				weaponName = "WID_Athena_Bucket_Nice";
			}
			else if (weaponName == "stink" || weaponName == "stinkbomb" || weaponName == "stinks")
			{
				weaponName = "Athena_GasGrenade";
			}
			else if (weaponName == "shieldbubble")
			{
				weaponName = "Athena_SilverBlazer_V2";
			}
			else if (weaponName == "zaptrap")
			{
				weaponName = "Athena_ZippyTrout";
			}
			else if (weaponName == "batman")
			{
				weaponName = "WID_Badger_Grape_VR";
			}
			else if (weaponName == "spiderman")
			{
				weaponName = "WID_WestSausage_Parallel";
			}
			else if (weaponName == "shockwave" || weaponName == "shock" || weaponName == "shockwavegrenade" || weaponName == "shocks" || weaponName == "shockwaves")
			{
				weaponName = "Athena_ShockGrenade";
			}
			else if (weaponName == "impulse" || weaponName == "impulsegrenade" || weaponName == "impulses")
			{
				weaponName = "Athena_KnockGrenade";
			}
			else if (weaponName == "portafortress" || weaponName == "fortress")
			{
				weaponName = "Athena_SuperTowerGrenade_A";
			}
			else if (weaponName == "hopflop" || weaponName == "hopflopper")
			{
				weaponName = "WID_Athena_Flopper_HopFlopper";
			}
			else if (weaponName == "slurpfish")
			{
				weaponName = "WID_Athena_Flopper_Effective";
			}
			else if (weaponName == "chugsplash" || weaponName == "chugs")
			{
				weaponName = "Athena_ChillBronco";
			}
			else if (weaponName == "minis")
			{
				weaponName = "Athena_ShieldSmall";
			}
			else if (weaponName == "bandage" || weaponName == "bandages")
			{
				weaponName = "Athena_Bandage";
			}
			else if (weaponName == "portafort" || weaponName == "paf")
			{
				weaponName = "Athena_TowerGrenade";
			}
			else if (weaponName == "c4")
			{
				weaponName = "Athena_C4";
			}
			else if (weaponName == "firefly" || weaponName == "fireflies")
			{
				weaponName = "WID_Athena_Grenade_Molotov";
			}
			else if (weaponName == "tire" || weaponName == "tires" || weaponName == "tyre")
			{
				weaponName = "ID_ValetMod_Tires_OffRoad_Thrown";
			}
			else if (weaponName == "doomgauntlets" || weaponName == "doom")
			{
				weaponName = "WID_HighTower_Date_ChainLightning_CoreBR";
			}
			else if (weaponName == "dub")
			{
				weaponName = "WID_WaffleTruck_Dub";
			}
			else if (weaponName == "hoprockdualies" || weaponName == "dualies")
			{
				weaponName = "WID_WaffleTruck_HopRockDualies";
			}
			else if (weaponName == "recon")
			{
				weaponName = "AGID_Athena_Scooter";
			}
			else if (weaponName == "jules" || weaponName == "julesgrappler" || weaponName == "julesgrap")
			{
				weaponName = "WID_Boss_GrapplingHoot";
			}
			else if (weaponName == "skye" || weaponName == "skyesgrappler" || weaponName == "skyegrap" || weaponName == "skyesgrap")
			{
				weaponName = "WID_Boss_Adventure_GH";
			}
			else if (weaponName == "captainamerica" || weaponName == "shield" || weaponName == "ca")
			{
				weaponName = "AGID_AshtonPack_Chicago";
			}
			else if (weaponName == "thorshammer" || weaponName == "thor" || weaponName == "thors")
			{
				weaponName = "AGID_AshtonPack_Turbo";
			}
			else if (weaponName == "batman" || weaponName == "batgrap")
			{
				weaponName = "WID_Badger_Grape_VR";
			}
			else if (weaponName == "flare" || Command == "flaregun")
			{
				weaponName = "WID_FringePlank_Athena_Prototype";
			}
			else if (weaponName == "grabitron")
			{
				weaponName = "WID_GravyGoblinV2_Athena";
			}
			else if (weaponName == "grappler" || weaponName == "grap" || weaponName == "grapple")
			{
				if (Fortnite_Version < 7.10)
				{
					weaponName = "WID_Hook_Gun_VR_Ore_T03";
				}
				else
				{
					weaponName = "WID_Hook_Gun_Slide";
				}
			}
			else if (weaponName == "snowman" || weaponName == "snowmen")
			{
				if (Fortnite_Version < 11)
				{
					weaponName = "Athena_SneakySnowman";
				}
				else
				{
					weaponName = "AGID_SneakySnowmanV2";
				}
			}
			else if (weaponName == "ironman" || weaponName == "iron-man")
			{
				if (Fortnite_Version < 14.00 || Fortnite_Version > 14.60)
				{
					weaponName = "AGID_AshtonPack_Indigo";
				}
				else
				{
					weaponName = "WID_HighTower_Tomato_Repulsor_CoreBR";
				}
			}
			else if (weaponName == "hunting")
			{
				if (Fortnite_Version < 12.41)
				{
					weaponName = "WID_Sniper_NoScope_Athena_R_Ore_T03";
				}
				else
				{
					weaponName = "WID_Sniper_NoScope_Athena_SR_Ore_T03";
				}
			}
			else if (weaponName == "pump")
			{
				if (Fortnite_Version < 6.31)
				{
					weaponName = "WID_Shotgun_Standard_Athena_UC_Ore_T03";
				}
				else
				{
					weaponName = "WID_Shotgun_Standard_Athena_SR_Ore_T03";
				}
			}
			else if (weaponName == "harpoon")
			{
				if (Fortnite_Version < 13.40)
				{
					weaponName = "WID_Athena_HappyGhost";
				}
				else
				{
					weaponName = "WID_Athena_HappyGhost_Infinite";
				}
			}

			int amount = 1;

			try
			{
				if (NumArgs >= 3)
					amount = std::stoi(Arguments[3]);
			}
			catch (...)
			{
			}

			constexpr int Max = 100;

			if (amount > Max)
			{
				SendMessageToConsole(PlayerController, (std::wstring(L"You went over the limit! Only spawning ") + std::to_wstring(Max) + L".").c_str());
				amount = Max;
			}

			// LOG_INFO(LogDev, "weaponName: {}", weaponName);

			auto WID = Cast<UFortWorldItemDefinition>(FindObject(weaponName, nullptr, ANY_PACKAGE));

			if (!WID)
			{
				SendMessageToConsole(PlayerController, L"Invalid WID!");
				return;
			}

			int count = WID->GetMaxStackSize();

			try
			{
				if (NumArgs >= 2)
					count = std::stoi(Arguments[2]);
			}
			catch (...)
			{
			}

			auto Location = Pawn->GetActorLocation();

			auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GetGameState());

			PickupCreateData CreateData;
			CreateData.ItemEntry = FFortItemEntry::MakeItemEntry(WID, count, -1, MAX_DURABILITY, WID->GetFinalLevel(GameState->GetWorldLevel()));
			CreateData.SpawnLocation = Location;
			CreateData.bShouldFreeItemEntryWhenDeconstructed = true;

			for (int i = 0; i < amount; i++)
			{
				AFortPickup::SpawnPickup(CreateData);
			}
		}
		else if (Command == "getlocation" || Command == "getloc" || Command == "cords" || Command == "a")
		{
			auto Pawn = ReceivingController->GetMyFortPawn();

			auto PawnLocation = Pawn->GetActorLocation();

			FString Loc = PawnLocation.ToString();

			SendMessageToConsole(PlayerController, Loc);

			Pawn->CopyToClipboard(Loc);
		}
		else if (Command == "dbno" || Command == "dnbo")
		{
			auto Pawn = ReceivingController->GetMyFortPawn();

			if (!Pawn)
			{
				SendMessageToConsole(ReceivingController, L"No pawn!");
				return;
			}

			Pawn->SetDBNO(!Pawn->IsDBNO());
			SendMessageToConsole(PlayerController, std::wstring(L"DBNO set to " + std::to_wstring(!(bool)Pawn->IsDBNO())).c_str());
		}
		else if (Command == "logprocessevent")
		{
			Globals::bLogProcessEvent = !Globals::bLogProcessEvent;
			SendMessageToConsole(PlayerController, L"Toggled logprocessevent!");
		}
		else if (Command == "fakewin")
		{
			GameMode->EndMatch();
			PlayerController->PlayWinEffects(PlayerController->GetPawn(), PlayerController->GetMyFortPawn()->GetCurrentWeapon()->GetWeaponData(), 1, false);
			PlayerController->ClientNotifyWon(PlayerController->GetPawn(), PlayerController->GetMyFortPawn()->GetCurrentWeapon()->GetWeaponData(), 1);
			PlayerController->ClientNotifyTeamWon(PlayerController->GetPawn(), PlayerController->GetMyFortPawn()->GetCurrentWeapon()->GetWeaponData(), 1);

			SendMessageToConsole(PlayerController, L"You won!");
		}
		else if (Command == "startaircraft")
		{
			if (bStartedBus)
			{
				SendMessageToConsole(ReceivingController, L"Bus has already started!");
				return;
			}

			UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"startaircraft", ReceivingController);

			SendMessageToConsole(ReceivingController, L"Started bus.");
		}
		else if (Command == "creativestartgame")
		{
			if (!Globals::bCreative)
			{
				SendMessageToConsole(PlayerController, L"You are not playing creative!");
				return;
			}

			if (Addresses::CreativeStartGame != 0)
			{
				static void (*StartGame)(__int64 idk) = decltype(StartGame)(Addresses::CreativeStartGame);

				__int64 idk = 1;

				StartGame(idk);

				SendMessageToConsole(PlayerController, L"Creative game started!");
			}
		}
		else if (Command == "listplayers")
		{
			std::string PlayerNames;

			for (int i = 0; i < ClientConnections.Num(); i++)
			{
				static auto PlayerControllerOffset = ClientConnections.at(i)->GetOffset("PlayerController");
				auto CurrentPlayerController = Cast<AFortPlayerControllerAthena>(ClientConnections.at(i)->Get(PlayerControllerOffset));

				if (!CurrentPlayerController)
					continue;

				auto CurrentPlayerState = Cast<AFortPlayerStateAthena>(CurrentPlayerController->GetPlayerState());

				if (!CurrentPlayerState->IsValidLowLevel())
					continue;

				PlayerNames += "\"" + CurrentPlayerState->GetPlayerName().ToString() + "\" ";
			}

			SendMessageToConsole(PlayerController, std::wstring(PlayerNames.begin(), PlayerNames.end()).c_str());
		}
		else if (Command == "launch")
		{
			float X = 0.0f, Y = 0.0f, Z = 0.0f;

			bool bIgnoreFallDamage = true;

			int argCount = Arguments.size();

			try
			{
				if (argCount == 1)
				{
					Z = 0.0f;
				}
				if (argCount == 2)
				{
					Z = std::stof(Arguments[1]);
				}
				else if (argCount == 3)
				{
					X = std::stof(Arguments[1]);
					Z = std::stof(Arguments[2]);
				}
				else if (argCount == 4)
				{
					X = std::stof(Arguments[1]);
					Y = std::stof(Arguments[2]);
					Z = std::stof(Arguments[3]);
				}
			}
			catch (...)
			{
				SendMessageToConsole(PlayerController, L"Invalid input. Please provide numeric values.");
				return;
			}

			auto Pawn = ReceivingController->GetMyFortPawn();

			if (!Pawn)
			{
				SendMessageToConsole(PlayerController, L"No pawn to launch!");
				return;
			}

			static auto LaunchCharacterFn = FindObject<UFunction>(L"/Script/Engine.Character.LaunchCharacter");

			struct
			{
				FVector LaunchVelocity; // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
				bool bXYOverride;       // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
				bool bZOverride;        // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
				bool bIgnoreFallDamage; // (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
			} ACharacter_LaunchCharacter_Params{ FVector(X, Y, Z), false, false, bIgnoreFallDamage }; // i dont think bIgnoreFallDamage works icl

			Pawn->ProcessEvent(LaunchCharacterFn, &ACharacter_LaunchCharacter_Params);

			SendMessageToConsole(PlayerController, L"Launched character!");
		}
		else if (Command == "falldmg" || Command == "falldamage")
		{
			/*bool FallDamage = false;

			if (auto Pawn = Cast<AFortPlayerPawn>(ReceivingController->GetMyFortPawn()))
			{
				if (FallDamage)
				{
					static auto FallDamageClass = FindObject<UClass>(L"/Game/Athena/Items/Gameplay/BackPacks/Ashton/GE_AshtonPack_FallDamageImmune.GE_AshtonPack_FallDamageImmune_C");
					PlayerState->GetAbilitySystemComponent()->RemoveActiveGameplayEffectBySourceEffect(FallDamageClass, 1, PlayerState->GetAbilitySystemComponent());
					SendMessageToConsole(PlayerController, L"Fall damage has been turned on.");
				}
				else
				{
					static auto FallDamageClass = FindObject<UClass>(L"/Game/Athena/Items/Gameplay/BackPacks/Ashton/GE_AshtonPack_FallDamageImmune.GE_AshtonPack_FallDamageImmune_C");
					auto AbilitySystemComponent = PlayerState->GetAbilitySystemComponent();
					AbilitySystemComponent->ApplyGameplayEffectToSelf(FallDamageClass, 1.f);
					SendMessageToConsole(PlayerController, L"Fall damage has been turned off.");
				}

				FallDamage = !FallDamage;
			}*/

			if (auto Pawn = Cast<AFortPlayerPawn>(ReceivingController->GetMyFortPawn()))
				PlayerState->DisableFallDamage();

			SendMessageToConsole(PlayerController, L"You have **PERMANENTLY** turned off fall damage for this game.");
		}
		else if (Command == "changespread")
		{
			if (NumArgs < 1)
			{
				SendMessageToConsole(ReceivingController, L"Please provide a valid clip size!");
				return;
			}

			auto WorldInventory = ReceivingController->GetWorldInventory();

			if (!WorldInventory)
			{
				SendMessageToConsole(PlayerController, L"No world inventory!");
				return;
			}

			static auto WeaponDef = FindObject<UFortWeaponItemDefinition>("/Game/Athena/Items/Weapons/WID_Sniper_BoltAction_Scope_Athena_SR_Ore_T03.WID_Sniper_BoltAction_Scope_Athena_SR_Ore_T03");

			int NewSpread = WeaponDef->GetSpread();

			try { NewSpread = std::stoi(Arguments[1]); }
			catch (...) {}

			WeaponDef->GetSpread() = NewSpread;

			bool bShouldUpdate = false;
			WorldInventory->AddItem(WeaponDef, &bShouldUpdate);

			if (bShouldUpdate)
				WorldInventory->Update();

			SendMessageToConsole(PlayerController, L"Granted item!");
		}
		else if (Command == "setshield" || Command == "shield")
		{
			auto Pawn = ReceivingController->GetMyFortPawn();

			if (!Pawn)
			{
				SendMessageToConsole(PlayerController, L"No pawn!");
				return;
			}

			if (Arguments.size() < 2)
			{
				SendMessageToConsole(PlayerController, L"Please choose a number to set your shield to!");
			}
			else
			{
				float Shield = 0.f;

				if (NumArgs >= 1)
				{
					try { Shield = std::stof(Arguments[1]); }
					catch (...) {}
				}

				Pawn->SetShield(Shield);

				std::wstringstream ss;
				ss << std::fixed << std::setprecision(0) << Shield;

				std::wstring Message = L"Shield set to " + ss.str() + L"!\n";
				SendMessageToConsole(PlayerController, Message.c_str());
			}
		}
		else if (Command == "setmaxshield" || Command == "maxshield")
		{
			auto Pawn = ReceivingController->GetMyFortPawn();

			if (!Pawn)
			{
				SendMessageToConsole(PlayerController, L"No pawn!");
				return;
			}

			if (Arguments.size() < 2)
			{
				SendMessageToConsole(PlayerController, L"Please choose a number to set your max shield to!");
			}
			else
			{
				float MaxShield = 0.f;

				if (NumArgs >= 1)
				{
					try { MaxShield = std::stof(Arguments[1]); }
					catch (...) {}
				}

				Pawn->SetMaxShield(MaxShield);

				std::wstringstream ss;
				ss << std::fixed << std::setprecision(0) << MaxShield;

				std::wstring Message = L"Max shield set to " + ss.str() + L"!\n";
				SendMessageToConsole(PlayerController, Message.c_str());
			}
		}
		/*else if (Command == "lategame" || Command == "lg")
		{
			if (Globals::bLateGame)
			{
				SendMessageToConsole(PlayerController, L"You can only run this command on Full Map!");
			}
			else
			{
				static auto World_NetDriverOffset = GetWorld()->GetOffset("NetDriver");
				auto WorldNetDriver = GetWorld()->Get<UNetDriver*>(World_NetDriverOffset);
				auto& ClientConnections = WorldNetDriver->GetClientConnections();

				for (int z = 0; z < ClientConnections.Num(); z++)
				{
					auto ClientConnection = ClientConnections.at(z);
					auto FortPC = Cast<AFortPlayerController>(ClientConnection->GetPlayerController());

					if (!FortPC)
						continue;

					auto WorldInventory = FortPC->GetWorldInventory();

					if (!WorldInventory)
						continue;

					static auto WoodItemData = FindObject<UFortItemDefinition>(
						L"/Game/Items/ResourcePickups/WoodItemData.WoodItemData");
					static auto StoneItemData = FindObject<UFortItemDefinition>(
						L"/Game/Items/ResourcePickups/StoneItemData.StoneItemData");
					static auto MetalItemData = FindObject<UFortItemDefinition>(
						L"/Game/Items/ResourcePickups/MetalItemData.MetalItemData");
					static auto Gold = FindObject<UFortItemDefinition>(
						L"/Game/Items/ResourcePickups/Athena_WadsItemData.Athena_WadsItemData");

					static UFortItemDefinition* Primary = nullptr;

					do
					{
						Primary = FindObject<UFortItemDefinition>(GetRandomItem(Primaries, z), nullptr, ANY_PACKAGE);
					} while (!Primary);

					static UFortItemDefinition* Secondary = nullptr;

					do
					{
						Secondary = FindObject<UFortItemDefinition>(GetRandomItem(Secondaries, z), nullptr, ANY_PACKAGE);
					} while (!Secondary);

					static UFortItemDefinition* Tertiary = nullptr;

					do
					{
						Tertiary = FindObject<UFortItemDefinition>(GetRandomItem(Tertiaries, z), nullptr, ANY_PACKAGE);
					} while (!Tertiary);

					static UFortItemDefinition* Consumable1 = nullptr;

					do
					{
						Consumable1 = FindObject<UFortItemDefinition>(GetRandomItem(Consumables1, z), nullptr, ANY_PACKAGE);
					} while (!Consumable1);

					static UFortItemDefinition* Consumable2 = nullptr;

					do
					{
						Consumable2 = FindObject<UFortItemDefinition>(GetRandomItem(Consumables2, z), nullptr, ANY_PACKAGE);
					} while (!Consumable2);

					static UFortItemDefinition* Trap = nullptr;

					do
					{
						Trap = FindObject<UFortItemDefinition>(GetRandomItem(Traps, z), nullptr, ANY_PACKAGE);
					} while (!Trap);

					static auto HeavyAmmo = FindObject<UFortItemDefinition>(
						L"/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsHeavy.AthenaAmmoDataBulletsHeavy");
					static auto ShellsAmmo = FindObject<UFortItemDefinition>(
						L"/Game/Athena/Items/Ammo/AthenaAmmoDataShells.AthenaAmmoDataShells");
					static auto MediumAmmo = FindObject<UFortItemDefinition>(
						L"/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsMedium.AthenaAmmoDataBulletsMedium");
					static auto LightAmmo = FindObject<UFortItemDefinition>(
						L"/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsLight.AthenaAmmoDataBulletsLight");
					static auto RocketAmmo = FindObject<UFortItemDefinition>(
						L"/Game/Athena/Items/Ammo/AmmoDataRockets.AmmoDataRockets");
					static auto ExplosiveAmmo = FindObject<UFortItemDefinition>(
						L"/Game/Items/Ammo/AmmoDataExplosive.AmmoDataExplosive");
					static auto EnergyCells = FindObject<UFortItemDefinition>(
						L"/Game/Items/Ammo/AmmoDataEnergyCell.AmmoDataEnergyCell");
					static auto Arrows = FindObject<UFortItemDefinition>(
						L"/PrimalGameplay/Items/Ammo/AthenaAmmoDataArrows.AthenaAmmoDataArrows");
					static auto ReconAmmo = FindObject<UFortItemDefinition>(
						L"/MotherGameplay/Items/Scooter/Ammo_Athena_Mother_Scooter.Ammo_Athena_Mother_Scooter");
					static auto STWHeavy = FindObject<UFortItemDefinition>(
						L"/Game/Items/Ammo/AmmoDataBulletsHeavy.AmmoDataBulletsHeavy");
					static auto STWMedium = FindObject<UFortItemDefinition>(
						L"/Game/Items/Ammo/AmmoDataBulletsMedium.AmmoDataBulletsMedium");
					static auto STWLight = FindObject<UFortItemDefinition>(
						L"/Game/Items/Ammo/AmmoDataBulletsLight.AmmoDataBulletsLight");
					static auto STWShells = FindObject<UFortItemDefinition>(
						L"/Game/Items/Ammo/AmmoDataShells.AmmoDataShells");

					WorldInventory->AddItem(WoodItemData, nullptr, (std::rand() % 646) + 186);
					WorldInventory->AddItem(StoneItemData, nullptr, (std::rand() % 646) + 186);
					WorldInventory->AddItem(MetalItemData, nullptr, (std::rand() % 646) + 186);

					if (Fortnite_Version < 15)
					{
						WorldInventory->AddItem(Gold, nullptr, (std::rand() % 7500) + 1200);
					}
					else
					{
						WorldInventory->AddItem(Gold, nullptr, Gold->GetMaxStackSize());
					}

					WorldInventory->AddItem(Primary, nullptr, 1);
					WorldInventory->AddItem(Secondary, nullptr, 1);
					WorldInventory->AddItem(Tertiary, nullptr, 1);
					WorldInventory->AddItem(Consumable1, nullptr, Consumable1->GetMaxStackSize());
					WorldInventory->AddItem(Consumable2, nullptr, Consumable2->GetMaxStackSize());
					WorldInventory->AddItem(ShellsAmmo, nullptr, (std::rand() % 576) + 87);
					WorldInventory->AddItem(HeavyAmmo, nullptr, (std::rand() % 576) + 50);
					WorldInventory->AddItem(MediumAmmo, nullptr, (std::rand() % 824) + 186);
					WorldInventory->AddItem(LightAmmo, nullptr, (std::rand() % 824) + 124);
					WorldInventory->AddItem(RocketAmmo, nullptr, (std::rand() % 12) + 3);
					WorldInventory->AddItem(ExplosiveAmmo, nullptr, (std::rand() % 999) + 186);
					WorldInventory->AddItem(EnergyCells, nullptr, (std::rand() % 999) + 186);
					WorldInventory->AddItem(STWHeavy, nullptr, (std::rand() % 999) + 186);
					WorldInventory->AddItem(STWMedium, nullptr, (std::rand() % 999) + 186);
					WorldInventory->AddItem(STWLight, nullptr, (std::rand() % 999) + 186);
					WorldInventory->AddItem(STWShells, nullptr, (std::rand() % 999) + 186);
					WorldInventory->AddItem(Arrows, nullptr, (std::rand() % 30) + 12);
					WorldInventory->AddItem(ReconAmmo, nullptr, 999);

					WorldInventory->Update();
				}

				for (int i = 0; i < ClientConnections.Num(); i++)
				{
					auto Pawn = ReceivingController->GetMyFortPawn();

					if (!Pawn)
					{
						SendMessageToConsole(PlayerController, L"No pawn!");
						return;
					}

					float MaxHealth = Pawn->GetMaxHealth();

					auto HealthSet = Pawn->GetHealthSet();

					if (!HealthSet)
					{
						SendMessageToConsole(PlayerController, L"No HealthSet!");
						return;
					}

					static auto HealthOffset = HealthSet->GetOffset("Health");
					auto& Health = HealthSet->Get<FFortGameplayAttributeData>(HealthOffset);

					if (Health.GetMinimum() != MaxHealth)
					{
						Health.GetMinimum() = MaxHealth;
					}
					else
					{
						Health.GetMinimum() = 0;
					}
				}

				for (int i = 0; i < ClientConnections.Num(); i++)
				{
					auto PlayerController = Cast<AFortPlayerController>(ClientConnections.at(i)->GetPlayerController());
					auto Pawn = PlayerController->GetMyFortPawn();

					Pawn->TeleportTo(FVector{ 0,0,105000 }, FRotator{ 0,0,0 });

					float height = 2000;

					Pawn->ProcessEvent(Pawn->FindFunction("TeleportToSkyDive"), &height);
				}

				UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"startsafezone", nullptr);

				auto GameMode = Cast<AFortGameModeAthena>(GetWorld()->GetGameMode());
				auto SafeZoneIndicator = GameMode->GetSafeZoneIndicator();

				if (SafeZoneIndicator)
				{
					SafeZoneIndicator->SkipShrinkSafeZone();
					SafeZoneIndicator->SkipShrinkSafeZone();
					SafeZoneIndicator->SkipShrinkSafeZone();
					SafeZoneIndicator->SkipShrinkSafeZone();
					SafeZoneIndicator->SkipShrinkSafeZone();
				}

				UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"pausesafezone", nullptr);

				// SafeZoneIndicator->GetSafeZoneCenter();

				SendMessageToConsole(PlayerController, L"Skipped to Lategame Zone!");
			}
		}*/
		/*else if (Command == "setbotmaxshield" || Command == "botmaxshield")
		{
			AFortPlayerPawnAthena* BotPawn = nullptr;

			if (!ShouldUseAIBotController())
			{
				BotPawn = Cast<AFortPlayerPawnAthena>(Controller->GetPawn());
			}
			else
			{
				BotPawn = Cast<AFortPlayerPawnAthena>(AIBotController->GetPawn());
			}

			if (!BotPawn)
			{
				SendMessageToConsole(PlayerController, L"No bot pawn found!");
				return;
			}

			float MaxShield = 0.f;

			if (NumArgs >= 1)
			{
				try { MaxShield = std::stof(Arguments[1]); }
				catch (...) {}
			}

			BotPawn->SetMaxShield(MaxShield);

			std::wstringstream ss;
			ss << std::fixed << std::setprecision(0) << MaxShield;

			std::wstring Message = L"Bot max shield set to " + ss.str() + L"!\n";
			SendMessageToConsole(PlayerController, Message.c_str());
		}*/
		else if (Command == "regen")
		{
			auto Pawn = ReceivingController->GetMyFortPawn();

			if (!Pawn)
			{
				SendMessageToConsole(PlayerController, L"No pawn!");
				return;
			}

			float MaxShield = Pawn->GetMaxShield();
			float MaxHealth = Pawn->GetMaxHealth();

			Pawn->SetHealth(MaxHealth);
			Pawn->SetShield(MaxShield);

			if (auto Pawn = Cast<AFortPlayerPawn>(ReceivingController->GetMyFortPawn()))
				PlayerState->ApplySiphonEffect();

			SendMessageToConsole(PlayerController, L"Regenerated health and shield!\n");
		}
		else if (Command == "siphonanim" || Command == "siphoneffect")
		{
			if (auto Pawn = Cast<AFortPlayerPawn>(ReceivingController->GetMyFortPawn()))
				PlayerState->ApplySiphonEffect();

			SendMessageToConsole(PlayerController, L"Applied Siphon Effect!");
		}
		else if (Command == "siphon" || Command == "amountofhealthshield" || Command == "setsiphon")
		{
			float Siphon = 0.f;

			if (NumArgs >= 1)
			{
				try { Siphon = std::stof(Arguments[1]); }
				catch (...) {}
			}

			Globals::AmountOfHealthSiphon = Siphon;

			std::wstringstream ss;
			ss << std::fixed << std::setprecision(0) << Siphon;

			std::wstring Message = L"Siphon amount set to " + ss.str() + L"!\n";
			SendMessageToConsole(PlayerController, Message.c_str());
		}
		else if (Command == "god")
		{
			auto Pawn = ReceivingController->GetMyFortPawn();

			if (!Pawn)
			{
				SendMessageToConsole(PlayerController, L"No pawn!");
				return;
			}

			float MaxHealth = Pawn->GetMaxHealth();

			auto HealthSet = Pawn->GetHealthSet();

			if (!HealthSet)
			{
				SendMessageToConsole(PlayerController, L"No HealthSet!");
				return;
			}

			static auto HealthOffset = HealthSet->GetOffset("Health");
			auto& Health = HealthSet->Get<FFortGameplayAttributeData>(HealthOffset);

			if (Health.GetMinimum() != MaxHealth)
			{
				Health.GetMinimum() = MaxHealth;
				SendMessageToConsole(PlayerController, L"God ON.");
			}
			else
			{
				Health.GetMinimum() = 0;
				SendMessageToConsole(PlayerController, L"God OFF.");
			}
		}
		else if (Command == "oldgod" || Command == "canbedamaged")
		{
			auto Pawn = ReceivingController->GetMyFortPawn();

			if (!Pawn)
			{
				SendMessageToConsole(PlayerController, L"No pawn!");
				return;
			}

			Pawn->SetCanBeDamaged(!Pawn->CanBeDamaged());
			SendMessageToConsole(PlayerController, std::wstring(L"God set to " + std::to_wstring(!(bool)Pawn->CanBeDamaged())).c_str());
		}
		else if (Command == "spawnaifromclass")
		{
			auto Pawn = ReceivingController->GetPawn();

			if (!Pawn)
			{
				SendMessageToConsole(PlayerController, L"No pawn!");
				return;
			}

			if (NumArgs < 1)
			{
				SendMessageToConsole(PlayerController, L"Please provide a class!");
				return;
			}

			TSubclassOf<APawn> PawnClass = FindObject<UClass>(Arguments[1]);

			if (NumArgs < 2)
			{
				SendMessageToConsole(PlayerController, L"Please provide a behavior tree!");
				return;
			}

			UBehaviorTree* BehaviorTree = FindObject<UBehaviorTree>(Arguments[2]);

			auto AIPawn = UAIBlueprintHelperLibrary::SpawnAIFromClass(GetWorld(), PawnClass, BehaviorTree, Pawn->GetActorLocation(), Pawn->GetActorRotation(), true, nullptr);

			if (AIPawn)
			{
				SendMessageToConsole(PlayerController, L"Summoned!");
			}
			else
			{
				SendMessageToConsole(PlayerController, L"Failed to summon AI!");
			}
		}
		else if (Command == "spawnjules")
		{
			auto Pawn = ReceivingController->GetPawn();

			if (!Pawn)
			{
				SendMessageToConsole(PlayerController, L"No pawn!");
				return;
			}

			if (NumArgs < 1)
			{
				SendMessageToConsole(PlayerController, L"Please provide a bot data!");
				return;
			}

			UFortAthenaAIBotCustomizationData* InBotData = FindObject<UFortAthenaAIBotCustomizationData>("/Game/Athena/AI/MANG/BotData/BotData_MANG_POI_Agency_2.BotData_MANG_POI_Agency_2");

			if (!InBotData)
			{
				SendMessageToConsole(PlayerController, L"Invalid bot data!");
				return;
			}

			FFortAthenaAIBotRunTimeCustomizationData RunTimeData;
			RunTimeData.HasCustomSquadId() = true;
			RunTimeData.ShouldCheckForOverlaps() = true;
			RunTimeData.GetCullDistanceSquared() = 0.f;
			RunTimeData.GetCustomSquadId() = 16;
			RunTimeData.GetPredefinedCosmeticSetTag() = FGameplayTag();

			auto SpawnedPawn = UFortServerBotManagerAthena::SpawnBotHook(BotManager, Pawn->GetActorLocation(), Pawn->GetActorRotation(), InBotData, RunTimeData);

			if (SpawnedPawn)
			{
				SendMessageToConsole(PlayerController, L"Summoned!");
			}
			else
			{
				SendMessageToConsole(PlayerController, L"Failed to summon!");
			}
		}
		else if (Command == "spawnshark")
		{
			auto Pawn = ReceivingController->GetPawn();

			if (!Pawn)
			{
				SendMessageToConsole(PlayerController, L"No pawn!");
				return;
			}

			if (NumArgs < 1)
			{
				SendMessageToConsole(PlayerController, L"Please provide a component list!");
				return;
			}

			UFortAthenaAISpawnerDataComponentList* InComponentList = FindObject<UFortAthenaAISpawnerDataComponentList>("/SpicySake/Content/AISpawnerData/Parent/AISpawnerData_SpicySake_Parent_BP.AISpawnerData_SpicySake_Parent_BP_C");

			if (!InComponentList)
			{
				SendMessageToConsole(PlayerController, L"Invalid component list!");
				return;
			}

			auto SpawnedPawn = BotManager->SpawnAI(Pawn->GetActorLocation(), Pawn->GetActorRotation(), InComponentList);

			if (SpawnedPawn)
			{
				SendMessageToConsole(PlayerController, L"Summoned!");
			}
			else
			{
				SendMessageToConsole(PlayerController, L"Failed to summon!");
			}
		}
		else if (Command == "changesize") // This works but doesn't visually update on the client (If done fully on the client with Rift or Cranium (No gameserver) it fully works)
		{
			if (NumArgs < 1)
			{
				SendMessageToConsole(PlayerController, L"Please provide a size!");
				return;
			}

			float F = 0.f;

			try { F = std::stof(Arguments[1]); }
			catch (...) {}

			auto CheatManager = ReceivingController->SpawnCheatManager(UCheatManager::StaticClass());

			if (!CheatManager)
			{
				SendMessageToConsole(PlayerController, L"Failed to spawn player's cheat manager!");
				return;
			}

			CheatManager->ChangeSize(F);
			CheatManager = nullptr;

			ReceivingController->GetMyFortPawn()->ForceNetUpdate();

			SendMessageToConsole(PlayerController, L"Changed size/hitbox!");
		}
		else if (Command == "destroyall")
		{
			if (NumArgs < 1)
			{
				SendMessageToConsole(PlayerController, L"Please provide a class!");
				return;
			}

			TSubclassOf<AActor> AClass = FindObject<UClass>(Arguments[1]);

			auto CheatManager = ReceivingController->SpawnCheatManager(UCheatManager::StaticClass());

			if (!CheatManager)
			{
				SendMessageToConsole(PlayerController, L"Failed to spawn player's cheat manager!");
				return;
			}

			CheatManager->DestroyAll(AClass);
			CheatManager = nullptr;

			SendMessageToConsole(PlayerController, L"Destroyed class!");
		}
		else if (Command == "destroyfishingholes")
		{
			TSubclassOf<AActor> FishingHoleClass = FindObject<UClass>(L"/Game/Athena/Items/EnvironmentalItems/FlopperSpawn/BGA_Athena_FlopperSpawn_World.BGA_Athena_FlopperSpawn_World_C");

			auto AllFishingHoles = UGameplayStatics::GetAllActorsOfClass(GetWorld(), FishingHoleClass);

			LOG_INFO(LogDev, "AllFishingHoles.Num(): {}", AllFishingHoles.Num());

			for (int i = 0; i < AllFishingHoles.Num(); i++)
			{
				auto FishingHole = AllFishingHoles.at(i);

				FishingHole->K2_DestroyActor();

				SendMessageToConsole(PlayerController, L"Destroyed fishing holes.");

				LOG_INFO(LogDev, "Destroyed Fishing Hole {}", FishingHole->GetFullName());
			}
		}
		else if (Command == "getscript" || Command == "script")
		{
			auto& IP = PlayerState->GetSavedNetworkAddress();
			auto IPStr = IP.ToString();

			if (IPStr == "127.0.0.1")
			{
				auto Pawn = ReceivingController->GetMyFortPawn();

				if (!Pawn)
				{
					SendMessageToConsole(PlayerController, L"No pawn!");
					return;
				}

				Pawn->LaunchURL(L"https://pastebin.com/4pmMgegz");
				SendMessageToConsole(PlayerController, L"Successfully opened script on the host's browser.");
			}
			else
			{
				SendMessageToConsole(PlayerController, L"Only the host can run this command!");
				return;
			}
		}
		else if (Command == "tutorial")
		{
			auto& IP = PlayerState->GetSavedNetworkAddress();
			auto IPStr = IP.ToString();

			if (IPStr == "127.0.0.1")
			{
				auto Pawn = ReceivingController->GetMyFortPawn();

				if (!Pawn)
				{
					SendMessageToConsole(PlayerController, L"No pawn!");
					return;
				}

				Pawn->LaunchURL(L"https://youtu.be/f9PHq9FUHbw?si=SYcONbJ2DSAKG8wZ");
				SendMessageToConsole(PlayerController, L"Successfully opened tutorial on the host's browser.");
			}
			else
			{
				SendMessageToConsole(PlayerController, L"Only the host can run this command!");
				return;
			}
		}
		else if (Command == "applycid" || Command == "skin")
		{
			auto PlayerState = Cast<AFortPlayerState>(ReceivingController->GetPlayerState());

			if (!PlayerState) // ???
			{
				SendMessageToConsole(PlayerController, L"No playerstate!");
				return;
			}

			auto Pawn = Cast<AFortPlayerPawn>(ReceivingController->GetMyFortPawn());

			std::string CIDStr = Arguments[1];

			if (CIDStr == "rene" || CIDStr == "renegaderaider") // if this git ever gets released feel free to pull request open more of these i cba
			{
				CIDStr = "CID_028_Athena_Commando_F";
			}
			else if (CIDStr == "ghoul" || CIDStr == "ghoultrooper")
			{
				CIDStr = "CID_029_Athena_Commando_F_Halloween";
			}
			else if (CIDStr == "skull" || CIDStr == "skulltrooper")
			{
				CIDStr = "CID_030_Athena_Commando_M_Halloween";
			}
			else if (CIDStr == "aerial" || CIDStr == "aerialassault")
			{
				CIDStr = "CID_017_Athena_Commando_M";
			}
			else if (CIDStr == "recon" || CIDStr == "reconexpert")
			{
				CIDStr = "CID_022_Athena_Commando_F";
			}
			else if (CIDStr == "desperado")
			{
				CIDStr = "CID_026_Athena_Commando_M";
			}
			else if (CIDStr == "blackknight" || CIDStr == "bk")
			{
				CIDStr = "CID_035_Athena_Commando_M_Medieval";
			}
			else if (CIDStr == "sparkle" || CIDStr == "sparklespecialist")
			{
				CIDStr = "CID_039_Athena_Commando_F_Disco";
			}
			else if (CIDStr == "rednosed" || CIDStr == "rnr" || CIDStr == "reindeer")
			{
				CIDStr = "CID_047_Athena_Commando_F_HolidayReindeer";
			}
			else if (CIDStr == "ginger")
			{
				CIDStr = "CID_048_Athena_Commando_F_HolidayGingerbread";
			}
			else if (CIDStr == "mogul")
			{
				CIDStr = "CID_061_Athena_Commando_F_SkiGirl";
			}
			else if (CIDStr == "usa")
			{
				CIDStr = "CID_062_Athena_Commando_F_SkiGirl_USA";
			}
			else if (CIDStr == "can")
			{
				CIDStr = "CID_063_Athena_Commando_F_SkiGirl_CAN";
			}
			else if (CIDStr == "gbr")
			{
				CIDStr = "CID_064_Athena_Commando_F_SkiGirl_GBR";
			}
			else if (CIDStr == "fra")
			{
				CIDStr = "CID_065_Athena_Commando_F_SkiGirl_FRA";
			}
			else if (CIDStr == "ger")
			{
				CIDStr = "CID_066_Athena_Commando_F_SkiGirl_GER";
			}
			else if (CIDStr == "chn")
			{
				CIDStr = "CID_067_Athena_Commando_F_SkiGirl_CHN";
			}
			else if (CIDStr == "kor")
			{
				CIDStr = "CID_068_Athena_Commando_F_SkiGirl_KOR";
			}
			else if (CIDStr == "rapscallion")
			{
				CIDStr = "CID_135_Athena_Commando_F_Jailbird";
			}
			else if (CIDStr == "galaxy")
			{
				CIDStr = "CID_175_Athena_Commando_M_Celestial";
			}
			else if (CIDStr == "heidi")
			{
				CIDStr = "CID_226_Athena_Commando_F_Octoberfest";
			}
			else if (CIDStr == "ikonik")
			{
				CIDStr = "CID_313_Athena_Commando_M_KpopFashion";
			}
			else if (CIDStr == "kuno")
			{
				CIDStr = "CID_362_Athena_Commando_F_BandageNinja";
			}
			else if (CIDStr == "aura")
			{
				CIDStr = "CID_397_Athena_Commando_F_TreasureHunterFashion";
			}
			else if (CIDStr == "wonder")
			{
				CIDStr = "CID_434_Athena_Commando_F_StealthHonor";
			}
			else if (CIDStr == "worldcup" || CIDStr == "wc")
			{
				CIDStr = "CID_478_Athena_Commando_F_WorldCup";
			}
			else if (CIDStr == "crystal")
			{
				CIDStr = "CID_493_Athena_Commando_F_JurassicArchaeology";
			}
			else if (CIDStr == "fennix")
			{
				CIDStr = "CID_504_Athena_Commando_M_Lopex";
			}

			auto CIDDef = FindObject(CIDStr, nullptr, ANY_PACKAGE);

			if (!CIDDef)
			{
				SendMessageToConsole(PlayerController, L"Invalid character item definition!");
				return;
			}

			LOG_INFO(LogDev, "Applying {}", CIDDef->GetFullName());

			if (!ApplyCID(Pawn, CIDDef))
			{
				SendMessageToConsole(PlayerController, L"Failed while applying skin! Please check the server log.");
				return;
			}

			SendMessageToConsole(PlayerController, L"Applied CID!");
		}
		else if (Command == "skinall")
		{
			for (int i = 0; i < ClientConnections.Num(); i++)
			{
				auto PlayerController = Cast<AFortPlayerController>(ClientConnections.at(i)->GetPlayerController());
				auto Pawn = PlayerController->GetMyFortPawn();
				auto FortPlayerPawn = Cast<AFortPlayerPawn>(Pawn);

				std::string CIDStr = Arguments[1];

				if (CIDStr == "rene" || CIDStr == "renegaderaider") // if this git ever gets released feel free to pull request open more of these i cba
				{
					CIDStr = "CID_028_Athena_Commando_F";
				}
				else if (CIDStr == "ghoul" || CIDStr == "ghoultrooper")
				{
					CIDStr = "CID_029_Athena_Commando_F_Halloween";
				}
				else if (CIDStr == "skull" || CIDStr == "skulltrooper")
				{
					CIDStr = "CID_030_Athena_Commando_M_Halloween";
				}
				else if (CIDStr == "aerial" || CIDStr == "aerialassault")
				{
					CIDStr = "CID_017_Athena_Commando_M";
				}
				else if (CIDStr == "recon" || CIDStr == "reconexpert")
				{
					CIDStr = "CID_022_Athena_Commando_F";
				}
				else if (CIDStr == "desperado")
				{
					CIDStr = "CID_026_Athena_Commando_M";
				}
				else if (CIDStr == "blackknight" || CIDStr == "bk")
				{
					CIDStr = "CID_035_Athena_Commando_M_Medieval";
				}
				else if (CIDStr == "sparkle" || CIDStr == "sparklespecialist")
				{
					CIDStr = "CID_039_Athena_Commando_F_Disco";
				}
				else if (CIDStr == "rednosed" || CIDStr == "rnr" || CIDStr == "reindeer")
				{
					CIDStr = "CID_047_Athena_Commando_F_HolidayReindeer";
				}
				else if (CIDStr == "ginger")
				{
					CIDStr = "CID_048_Athena_Commando_F_HolidayGingerbread";
				}
				else if (CIDStr == "mogul")
				{
					CIDStr = "CID_061_Athena_Commando_F_SkiGirl";
				}
				else if (CIDStr == "usa")
				{
					CIDStr = "CID_062_Athena_Commando_F_SkiGirl_USA";
				}
				else if (CIDStr == "can")
				{
					CIDStr = "CID_063_Athena_Commando_F_SkiGirl_CAN";
				}
				else if (CIDStr == "gbr")
				{
					CIDStr = "CID_064_Athena_Commando_F_SkiGirl_GBR";
				}
				else if (CIDStr == "fra")
				{
					CIDStr = "CID_065_Athena_Commando_F_SkiGirl_FRA";
				}
				else if (CIDStr == "ger")
				{
					CIDStr = "CID_066_Athena_Commando_F_SkiGirl_GER";
				}
				else if (CIDStr == "chn")
				{
					CIDStr = "CID_067_Athena_Commando_F_SkiGirl_CHN";
				}
				else if (CIDStr == "kor")
				{
					CIDStr = "CID_068_Athena_Commando_F_SkiGirl_KOR";
				}
				else if (CIDStr == "rapscallion")
				{
					CIDStr = "CID_135_Athena_Commando_F_Jailbird";
				}
				else if (CIDStr == "galaxy")
				{
					CIDStr = "CID_175_Athena_Commando_M_Celestial";
				}
				else if (CIDStr == "heidi")
				{
					CIDStr = "CID_226_Athena_Commando_F_Octoberfest";
				}
				else if (CIDStr == "ikonik")
				{
					CIDStr = "CID_313_Athena_Commando_M_KpopFashion";
				}
				else if (CIDStr == "kuno")
				{
					CIDStr = "CID_362_Athena_Commando_F_BandageNinja";
				}
				else if (CIDStr == "aura")
				{
					CIDStr = "CID_397_Athena_Commando_F_TreasureHunterFashion";
				}
				else if (CIDStr == "wonder")
				{
					CIDStr = "CID_434_Athena_Commando_F_StealthHonor";
				}
				else if (CIDStr == "worldcup" || CIDStr == "wc")
				{
					CIDStr = "CID_478_Athena_Commando_F_WorldCup";
				}
				else if (CIDStr == "crystal")
				{
					CIDStr = "CID_493_Athena_Commando_F_JurassicArchaeology";
				}
				else if (CIDStr == "fennix")
				{
					CIDStr = "CID_504_Athena_Commando_M_Lopex";
				}

				auto CIDDef = FindObject(CIDStr, nullptr, ANY_PACKAGE);

				if (!CIDDef)
				{
					SendMessageToConsole(PlayerController, L"Invalid character item definition!");
					return;
				}

				LOG_INFO(LogDev, "Applying {}", CIDDef->GetFullName());

				if (!ApplyCID(FortPlayerPawn, CIDDef))
				{
					SendMessageToConsole(PlayerController, L"Failed while applying skin! Please check the server log.");
					return;
				}

				SendMessageToConsole(PlayerController, L"Applied CID!");
			}
		}
		else if (Command == "applyhid" || Command == "hero")
		{
			auto PlayerState = Cast<AFortPlayerState>(ReceivingController->GetPlayerState());

			if (!PlayerState) // ???
			{
				SendMessageToConsole(PlayerController, L"No playerstate!");
				return;
			}

			auto Pawn = Cast<AFortPlayerPawn>(ReceivingController->GetMyFortPawn());

			std::string HIDStr = Arguments[1];
			auto HIDDef = FindObject(HIDStr, nullptr, ANY_PACKAGE);

			if (!HIDDef)
			{
				SendMessageToConsole(PlayerController, L"Invalid hero item definition!");
				return;
			}

			LOG_INFO(LogDev, "Applying {}", HIDDef->GetFullName());

			ApplyHID(Pawn, HIDDef);

			SendMessageToConsole(PlayerController, L"Applied HID!");
		}
		else if (Command == "suicide" || Command == "kill")
		{
			static auto ServerSuicideFn = FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerController.ServerSuicide");
			ReceivingController->ProcessEvent(ServerSuicideFn);

			SendMessageToConsole(PlayerController, L"Killed player!");
		}
		else if (Command == "spawn" || Command == "summon")
		{
			if (Arguments.size() <= 1)
			{
				SendMessageToConsole(PlayerController, L"Please provide a name or BP!\n");
				return;
			}

			auto& ActorName = Arguments[1];

			auto Pawn = ReceivingController->GetPawn();

			if (!Pawn)
			{
				SendMessageToConsole(PlayerController, L"No pawn to spawn BP at!");
				return;
			}

			FVector Loc = Pawn->GetActorLocation();
			int Count = 1;

			try
			{
				if (Arguments.size() >= 5)
				{
					Loc.X = std::stof(Arguments[2]);
					Loc.Y = std::stof(Arguments[3]);
					Loc.Z = std::stof(Arguments[4]) - 75;

					if (Arguments.size() >= 6)
						Count = std::stoi(Arguments[5]);
				}
				else
				{
					Loc.Z += 250;
				}

				if (Arguments.size() == 3)
				{
					Count = std::stoi(Arguments[2]);
				}
			}
			catch (const std::invalid_argument&)
			{
				SendMessageToConsole(PlayerController, L"Invalid input for coordinates or count!");
				return;
			}
			catch (const std::out_of_range&)
			{
				SendMessageToConsole(PlayerController, L"Input value out of range!");
				return;
			}

			FRotator SpawnRotation = Pawn->GetActorRotation();

			bool SpawnBots = ActorName == "bot" || ActorName == "bots";

			int Max = SpawnBots ? 99 : 100;

			if (Count > Max)
			{
				SendMessageToConsole(PlayerController, (std::wstring(L"You went over the limit! Only spawning ") + std::to_wstring(Max) + L".").c_str());
				Count = Max;
			}

			if (ActorName == "driftboard" || ActorName == "hoverboard")
				ActorName = "/Game/Athena/DrivableVehicles/JackalVehicle_Athena.JackalVehicle_Athena_C";
			else if (ActorName == "surfboard")
				ActorName = "/Game/Athena/DrivableVehicles/SurfboardVehicle_Athena.SurfboardVehicle_Athena_C";
			else if (ActorName == "quadcrasher" || ActorName == "quad")
				ActorName = "/Game/Athena/DrivableVehicles/AntelopeVehicle.AntelopeVehicle_C";
			else if (ActorName == "baller")
				ActorName = "/Game/Athena/DrivableVehicles/Octopus/OctopusVehicle.OctopusVehicle_C";
			else if (ActorName == "plane")
				ActorName = "/Game/Athena/DrivableVehicles/Biplane/BluePrints/FerretVehicle.FerretVehicle_C";
			else if (ActorName == "golfcart" || ActorName == "golf")
				ActorName = "/Game/Athena/DrivableVehicles/Golf_Cart/Golf_Cart_Base/Blueprints/GolfCartVehicleSK.GolfCartVehicleSK_C";
			else if (ActorName == "ufo")
				SendMessageToConsole(PlayerController, L"for some fuckass reason this doesn't work, go to the V3 script (cheat script if your hosting) and grab the spawn from there");
			else if (ActorName == "cannon")
				ActorName = "/Game/Athena/DrivableVehicles/PushCannon.PushCannon_C";
			else if (ActorName == "shoppingcart" || ActorName == "shopping")
				ActorName = "/Game/Athena/DrivableVehicles/ShoppingCartVehicleSK.ShoppingCartVehicleSK_C";
			else if (ActorName == "mech" || ActorName == "brute")
				ActorName = "/Game/Athena/DrivableVehicles/Mech/TestMechVehicle.TestMechVehicle_C";
			else if (ActorName == "bear" || ActorName == "truck")
				ActorName = "/Valet/BasicTruck/Valet_BasicTruck_Vehicle.Valet_BasicTruck_Vehicle_C";
			else if (ActorName == "prevelant" || ActorName == "car")
				ActorName = "/Valet/BasicCar/Valet_BasicCar_Vehicle.Valet_BasicCar_Vehicle_C";
			else if (ActorName == "whiplash" || ActorName == "sportscar")
				ActorName = "/Valet/SportsCar/Valet_SportsCar_Vehicle.Valet_SportsCar_Vehicle_C";
			else if (ActorName == "taxi")
				ActorName = "/Valet/TaxiCab/Valet_TaxiCab_Vehicle.Valet_TaxiCab_Vehicle_C";
			else if (ActorName == "mudflap")
				ActorName = "/Valet/BigRig/Valet_BigRig_Vehicle.Valet_BigRig_Vehicle_C";
			else if (ActorName == "stark")
				ActorName = "/Valet/SportsCar/Valet_SportsCar_Vehicle_HighTower.Valet_SportsCar_Vehicle_HighTower_C";
			else if (ActorName == "boat")
				ActorName = "/Game/Athena/DrivableVehicles/Meatball/Meatball_Large/MeatballVehicle_L.MeatballVehicle_L_C";
			else if (ActorName == "heli" || ActorName == "helicopter")
				ActorName = "/Hoagie/HoagieVehicle.HoagieVehicle_C";
			else if (ActorName == "shark")
				ActorName = "/SpicySake/Pawns/NPC_Pawn_SpicySake_Parent.NPC_Pawn_SpicySake_Parent_C";
			else if (ActorName == "klombo")
				ActorName = "/ButterCake/Pawns/NPC_Pawn_ButterCake_Base.NPC_Pawn_ButterCake_Base_C";
			else if (ActorName == "umbrella")
				ActorName = "/Game/Athena/Apollo/Environments/BuildingActors/Papaya/Papaya_BouncyUmbrella_C.Papaya_BouncyUmbrella_C_C";
			else if (ActorName == "dumpster")
				ActorName = "/Game/Athena/Items/EnvironmentalItems/HidingProps/Props/B_HidingProp_Dumpster.B_HidingProp_Dumpster_C";
			else if (ActorName == "tire")
				ActorName = "/Game/Building/ActorBlueprints/Prop/Prop_TirePile_04.Prop_TirePile_04_C";
			else if (ActorName == "llama")
				ActorName = "/Game/Athena/SupplyDrops/Llama/AthenaSupplyDrop_Llama.AthenaSupplyDrop_Llama_C";
			else if (ActorName == "rift")
				ActorName = "/Game/Athena/Items/ForagedItems/Rift/BGA_RiftPortal_Athena_Spawner.BGA_RiftPortal_Athena_Spawner_C";
			else if (ActorName == "airvent")
				ActorName = "/Game/Athena/Environments/Blueprints/DUDEBRO/BGA_HVAC.BGA_HVAC_C";
			else if (ActorName == "geyser")
				ActorName = "/Game/Athena/Environments/Blueprints/DudeBro/BGA_DudeBro_Mini.BGA_DudeBro_Mini_C";
			else if (ActorName == "nobuildzone")
				ActorName = "/Game/Athena/Prototype/Blueprints/Galileo/BP_Galileo_NoBuildZone.BP_Galileo_NoBuildZone_C";
			else if (ActorName == "launch" || ActorName == "launchpad")
				ActorName = "/Game/Athena/Items/Traps/Launchpad/BluePrint/Trap_Floor_Player_Launch_Pad.Trap_Floor_Player_Launch_Pad_C";
			else if (ActorName == "gascan" || ActorName == "gas")
				ActorName = "/Game/Athena/Items/Weapons/Prototype/PetrolPump/BGA_Petrol_Pickup.BGA_Petrol_Pickup_C";
			else if (ActorName == "supplydrop")
				if (Fortnite_Version >= 12.30 && Fortnite_Version <= 12.61)
					ActorName = "/Game/Athena/SupplyDrops/AthenaSupplyDrop_Donut.AthenaSupplyDrop_Donut_C";
				else if (Fortnite_Version == 5.10 || Fortnite_Version == 9.41 || Fortnite_Version == 14.20 || Fortnite_Version == 18.00)
					ActorName = "/Game/Athena/SupplyDrops/AthenaSupplyDrop_BDay.AthenaSupplyDrop_BDay_C";
				else if (Fortnite_Version == 1.11 || Fortnite_Version == 7.10 || Fortnite_Version == 7.20 || Fortnite_Version == 7.30 || Fortnite_Version == 11.31 || Fortnite_Version == 15.10 || Fortnite_Version == 19.01)
					ActorName = "/Game/Athena/SupplyDrops/AthenaSupplyDrop_Holiday.AthenaSupplyDrop_Holiday_C";
				else if (Fortnite_Version == 5.40 || Fortnite_Version == 5.41)
					ActorName = "/Game/Athena/SupplyDrops/Bling/AthenaSupplyDrop_Bling.AthenaSupplyDrop_Bling_C";
				else ActorName = "/Game/Athena/SupplyDrops/AthenaSupplyDrop.AthenaSupplyDrop_C";
			else if (ActorName == "zeropoint")
				if (Fortnite_Version < 15.00)
					ActorName = "/Game/Athena/Environments/Nexus/Blueprints/BP_ZeroPoint_Exploding.BP_ZeroPoint_Exploding_C";
				else ActorName = "/Game/Athena/Environments/Nexus/Blueprints/BP_ZeroPoint_2Point0.BP_ZeroPoint_2Point0_C";
			else if (ActorName == "lowgrav" || ActorName == "lowgravzone")
				if (Fortnite_Version < 12.00)
					ActorName = "/Game/Athena/Prototype/Blueprints/Cube/BGA_Cube_Area_Effect.BGA_Cube_Area_Effect_C";
				else ActorName = "/MotherGameplay/Items/Alpaca/BGA_Alpaca_AbductedPOI.BGA_Alpaca_AbductedPOI_C";

			static auto BGAClass = FindObject<UClass>(L"/Script/Engine.BlueprintGeneratedClass");
			static auto ClassClass = FindObject<UClass>(L"/Script/CoreUObject.Class");
			auto ClassObj = ActorName.contains("/Script/") ? FindObject<UClass>(ActorName, ClassClass) : LoadObject<UClass>(ActorName, BGAClass);

			if (ClassObj || SpawnBots)
			{
				int AmountSpawned = 0;

				for (int i = 0; i < Count; i++)
				{
					FTransform SpawnTransform;
					SpawnTransform.Translation = Loc;
					SpawnTransform.Rotation = SpawnRotation.Quaternion();
					SpawnTransform.Scale3D = FVector(1, 1, 1);

					auto NewActor = SpawnBots ? Bots::SpawnBot(SpawnTransform, Pawn) : GetWorld()->SpawnActor<AActor>(ClassObj, SpawnTransform);

					if (!NewActor)
					{
						SendMessageToConsole(PlayerController, (L"Failed to spawn actor(s)!"));
					}
					else
					{
						NewActor->ForceNetUpdate();
						AmountSpawned++;

						if (ActorName != "ufo")
						{
							SendMessageToConsole(PlayerController, L"Summoned!");
						}
					}
				}
			}
			else
			{
				if (ActorName != "ufo")
				{
					SendMessageToConsole(PlayerController, L"Not a valid class!");
				}
			}
		}
		else if (Command == "rift")
		{
			auto Pawn = ReceivingController->GetPawn();

			if (!Pawn)
			{
				SendMessageToConsole(PlayerController, L"No pawn to spawn the rift at!");
				return;
			}

			int Count = 1;

			if (Arguments.size() >= 2)
			{
				try { Count = std::stod(Arguments[1]); }
				catch (...) {}
			}

			constexpr int Max = 1;

			if (Count > Max)
			{
				SendMessageToConsole(PlayerController, L"You can only spawn 1 rift!");
			}
			else
			{
				for (int i = 0; i < Count; i++)
				{
					auto Loc = Pawn->GetActorLocation();
					Loc.Z += 0;

					static auto BGAClass = FindObject<UClass>(L"/Script/Engine.BlueprintGeneratedClass");
					static auto ClassClass = FindObject<UClass>(L"/Script/CoreUObject.Class");
					auto RiftClass = LoadObject<UClass>(L"/Game/Athena/Items/ForagedItems/Rift/BGA_RiftPortal_Athena_Spawner.BGA_RiftPortal_Athena_Spawner_C", BGAClass);

					if (!RiftClass)
					{
						SendMessageToConsole(PlayerController, L"Failed to find rift path!");
						return;
					}

					FTransform RiftSpawnTransform;
					RiftSpawnTransform.Translation = Loc;
					RiftSpawnTransform.Scale3D = FVector(1, 1, 1);

					auto NewRift = GetWorld()->SpawnActor<AActor>(RiftClass, Loc, FQuat(), FVector(1, 1, 1));

					if (!NewRift)
					{
						SendMessageToConsole(PlayerController, L"Failed to spawn the rift!");
					}
					else
					{
						NewRift->ForceNetUpdate();
						SendMessageToConsole(PlayerController, L"Rift summoned!");
					}
				}
			}
		}
		else if (Command == "tptomaxheight" || Command == "max" || Command == "tptomax")
		{
			auto Pawn = ReceivingController->GetPawn();

			if (!Pawn)
			{
				SendMessageToConsole(PlayerController, L"No pawn available!");
				return;
			}

			Pawn->TeleportTo(FVector{ 0,0,105000 }, FRotator{ 0,0,0 });

			float height = 2000;

			Pawn->ProcessEvent(Pawn->FindFunction("TeleportToSkyDive"), &height);
			SendMessageToConsole(PlayerController, L"Teleported player to max height!");
		}
		else if (Command == "spawnbot" || Command == "bot")
		{
			auto Pawn = ReceivingController->GetPawn();

			if (!Pawn)
			{
				SendMessageToConsole(PlayerController, L"No pawn to spawn bot at!");
				return;
			}

			int Count = 1;

			if (Arguments.size() >= 2)
			{
				try { Count = std::stod(Arguments[1]); }
				catch (...) {}
			}

			auto GameMode = Cast<AFortGameModeAthena>(GetWorld()->GetGameMode());

			bool bShouldSpawnAtZoneCenter = false;

			if (NumArgs >= 3 && Arguments[2] == "center")
				bShouldSpawnAtZoneCenter = true;

			if (bShouldSpawnAtZoneCenter && GameMode->GetGameStateAthena()->GetGamePhaseStep() <= EAthenaGamePhaseStep::BusFlying)
				bShouldSpawnAtZoneCenter = false;

			FRotator SpawnRotation = Pawn->GetActorRotation();

			int SizeMultiplier = 1;

			if (Arguments.size() >= 4)
			{
				try { SizeMultiplier = std::stod(Arguments[3]); }
				catch (...) {}
			}

			constexpr int Max = 99;

			if (Count > Max)
			{
				SendMessageToConsole(PlayerController, (std::wstring(L"You went over the limit! Only spawning ") + std::to_wstring(Max) + L".").c_str());
				Count = Max;
			}

			int AmountSpawned = 0;

			for (int i = 0; i < Count; i++)
			{
				FActorSpawnParameters SpawnParameters{};
				// SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

				auto SafeZoneIndicator = GameMode->GetSafeZoneIndicator();

				auto Loc = bShouldSpawnAtZoneCenter ? SafeZoneIndicator->GetSafeZoneCenter() : Pawn->GetActorLocation();
				Loc.Z += bShouldSpawnAtZoneCenter ? 10000 : 1000;

				FTransform Transform;
				Transform.Translation = Loc;
				Transform.Scale3D = FVector(1 * SizeMultiplier, 1 * SizeMultiplier, 1 * SizeMultiplier);
				Transform.Rotation = SpawnRotation.Quaternion();

				auto NewActor = Bots::SpawnBot(Transform, Pawn);

				if (!NewActor)
				{
					SendMessageToConsole(PlayerController, L"Failed to spawn an actor!");
				}
				else
				{
					AmountSpawned++;
				}
			}

			if (!bShouldSpawnAtZoneCenter)
				SendMessageToConsole(PlayerController, L"Summoned!");
			else
				SendMessageToConsole(PlayerController, L"Summoned at zone center!");
		}
		else if (Command == "marktoteleport" || Command == "marktotp" || Command == "markertp" || Command == "marktp" || Command == "mark" || Command == "marker")
		{
			auto Pawn = ReceivingController->GetMyFortPawn();

			bMarkToTeleport = !bMarkToTeleport;

			std::wstring message = L"Marker Teleporting is now ";
			message += (bMarkToTeleport ? L"on." : L"off.");

			SendMessageToConsole(PlayerController, message.c_str());
		}
		else if (Command == "demospeed")
		{
			float SpeedMultiplier = 1.0f;

			if (NumArgs >= 1)
			{
				try { SpeedMultiplier = std::stof(Arguments[1]); }
				catch (...) { SpeedMultiplier = 1.0f; }
			}

			std::wstring CommandString = L"demospeed ";
			CommandString += std::to_wstring(SpeedMultiplier);

			UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), CommandString.c_str(), nullptr);

			std::wstringstream ss;
			ss << std::fixed << std::setprecision(0) << SpeedMultiplier;

			std::wstring Message = L"Demospeed set to " + ss.str() + L"!\n";
			SendMessageToConsole(PlayerController, Message.c_str());
		}
		else if (Command == "settimeofday" || Command == "time" || Command == "hour")
		{
			static auto SetTimeOfDayFn = FindObject<UFunction>(L"/Script/FortniteGame.FortKismetLibrary.SetTimeOfDay");

			float NewTimeOfDay = 0.f;

			try { NewTimeOfDay = std::stoi(Arguments[1]); }
			catch (...) {}

			struct
			{
				UObject* WorldContextObject;
				float                              TimeOfDay;
			}params{ GetWorld() , NewTimeOfDay };

			UFortKismetLibrary::StaticClass()->ProcessEvent(SetTimeOfDayFn, &params);

			std::wstringstream ss;
			ss << std::fixed << std::setprecision(0) << NewTimeOfDay;

			std::wstring Message = L"Time of day set to " + ss.str() + L"!\n";
			SendMessageToConsole(PlayerController, Message.c_str());
		}
		else if (Command == "pausetimeofday" || Command == "pausetime" || Command == "pausehour")
		{
			static auto SetTimeOfDaySpeedFn = FindObject<UFunction>(L"/Script/FortniteGame.FortKismetLibrary.SetTimeOfDaySpeed");

			static bool isPaused = false;

			float newSpeed = isPaused ? 1.0f : 0.0f;

			struct
			{
				UObject* WorldContextObject;
				float Speed;
			} params{ GetWorld(), newSpeed };

			UFortKismetLibrary::StaticClass()->ProcessEvent(SetTimeOfDaySpeedFn, &params);

			isPaused = !isPaused;

			std::wstring Message = isPaused ? L"Time of day paused!\n" : L"Time of day resumed!\n";
			SendMessageToConsole(PlayerController, Message.c_str());
		}
		else if (Command == "sethealth" || Command == "health")
		{
			auto Pawn = ReceivingController->GetMyFortPawn();

			if (!Pawn)
			{
				SendMessageToConsole(PlayerController, L"No pawn!");
				return;
			}

			if (Arguments.size() < 2)
			{
				SendMessageToConsole(PlayerController, L"Please choose a number to set your health to!");
			}
			else
			{
				float Health = 100.f;

				try { Health = std::stof(Arguments[1]); }
				catch (...) {}

				Pawn->SetHealth(Health);

				std::wstringstream ss;
				ss << std::fixed << std::setprecision(0) << Health;

				std::wstring Message = L"Health set to " + ss.str() + L"!\n";
				SendMessageToConsole(PlayerController, Message.c_str());
			}
		}
		else if (Command == "setmaxhealth" || Command == "maxhealth")
		{
			auto Pawn = ReceivingController->GetMyFortPawn();

			if (!Pawn)
			{
				SendMessageToConsole(PlayerController, L"No pawn!");
				return;
			}

			if (Arguments.size() < 2)
			{
				SendMessageToConsole(PlayerController, L"Please choose a number to set your max health to!");
			}
			else
			{
				float MaxHealth = 100.f;

				try { MaxHealth = std::stof(Arguments[1]); }
				catch (...) {}

				Pawn->SetMaxHealth(MaxHealth);

				std::wstringstream ss;
				ss << std::fixed << std::setprecision(0) << MaxHealth;

				std::wstring Message = L"Max health set to " + ss.str() + L"!\n";
				SendMessageToConsole(PlayerController, Message.c_str());
			}
		}
		else if (Command == "pausesafezone" || Command == "pausezone")
		{
			auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GetGameState());
			auto GameMode = Cast<AFortGameModeAthena>(GetWorld()->GetGameMode());

			UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"pausesafezone", nullptr);

			SendMessageToConsole(PlayerController, L"Paused/Unpaused Zone.");

			/*if (!GameState->IsSafeZonePaused())
			{
				SendMessageToConsole(PlayerController, L"Paused Zone.");
			}
			else if (GameState->IsSafeZonePaused())
			{
				SendMessageToConsole(PlayerController, L"Unpaused Zone.");
			}*/
		}
		else if (Command == "teleport" || Command == "tp" || Command == "to")
		{
			UCheatManager*& CheatManager = ReceivingController->SpawnCheatManager(UCheatManager::StaticClass());

			if (!CheatManager)
			{
				SendMessageToConsole(PlayerController, L"Failed to spawn player's cheat manager!");
				return;
			}

			CheatManager->Teleport();
			CheatManager = nullptr;
			SendMessageToConsole(PlayerController, L"Teleported!");
		}
		else if (Command == "savewaypoint" || Command == "s")
		{
			if (NumArgs < 1)
			{
				SendMessageToConsole(PlayerController, L"Please provide a phrase to save the waypoint.");
				return;
			}

			auto Pawn = ReceivingController->GetMyFortPawn();

			if (!Pawn)
			{
				SendMessageToConsole(PlayerController, L"No pawn to get location from!");
				return;
			}

			auto PawnLocation = Pawn->GetActorLocation();
			std::string Phrase = Arguments[1];

			if (Waypoints.find(Phrase) != Waypoints.end())
			{
				if (NumArgs >= 2 && Arguments[2] == "override" || "o")
				{
					Waypoints[Phrase] = PawnLocation;
					SendMessageToConsole(PlayerController, L"Waypoint overridden successfully!");
				}
				else
				{
					SendMessageToConsole(PlayerController, L"A waypoint with this phrase already exists! Use 'waypoint {phrase} override' to overwrite it.");
				}
			}
			else
			{
				Waypoints[Phrase] = PawnLocation;
				SendMessageToConsole(PlayerController, L"Waypoint saved! Use « cheat waypoint (phrase) » to teleport to that location!");
			}
		}
		else if (Command == "waypoint" || Command == "w")
		{
			if (NumArgs < 1) 
			{
				SendMessageToConsole(PlayerController, L"Please provide a waypoint phrase to teleport to.");
				return;
			}

			std::string phrase = Arguments[1];

			if (Waypoints.find(phrase) == Waypoints.end()) 
			{
				SendMessageToConsole(PlayerController, L"A saved waypoint with this phrase was not found!");
				return;
			}

			FVector Destination = Waypoints[phrase];

			auto Pawn = ReceivingController->GetMyFortPawn();

			if (Pawn) 
			{
				Pawn->TeleportTo(Destination, Pawn->GetActorRotation());
				SendMessageToConsole(PlayerController, L"Teleported to waypoint!");

				static auto LaunchCharacterFn = FindObject<UFunction>(L"/Script/Engine.Character.LaunchCharacter");

				struct
				{
					FVector LaunchVelocity;
					bool bXYOverride;
					bool bZOverride;
					bool bIgnoreFallDamage;
				} ACharacter_LaunchCharacter_Params{ FVector(0.0f, 0.0f, -10000000.0f), false, false, true }; // sort of works to stop momentum

				Pawn->ProcessEvent(LaunchCharacterFn, &ACharacter_LaunchCharacter_Params);
			}
			else 
			{
				SendMessageToConsole(PlayerController, L"No pawn to teleport!");
			}
		}
		else if (Command == "fly")
		{
			auto Pawn = Cast<APawn>(ReceivingController->GetPawn());

			if (!Pawn)
			{
				SendMessageToConsole(PlayerController, L"No pawn found!");
				return;
			}

			static auto CharMovementOffset = Pawn->GetOffset("CharacterMovement");
			if (CharMovementOffset != -1)
			{
				auto CharMovement = Pawn->Get<UObject*>(CharMovementOffset);

				static auto MovementOffset = CharMovement->GetOffset("MovementMode", false);
				if (MovementOffset != -1)
				{
					EMovementMode MovementMode = CharMovement->Get<EMovementMode>(MovementOffset);
					EMovementMode NewMode = EMovementMode::MOVE_Walking;

					if (MovementMode != EMovementMode::MOVE_Flying)
					{
						NewMode = EMovementMode::MOVE_Flying;
					}

					static auto SetMovementModeFn = FindObject<UFunction>(L"/Script/Engine.CharacterMovementComponent.SetMovementMode");

					if (SetMovementModeFn)
					{
						CharMovement->ProcessEvent(SetMovementModeFn, &NewMode);
						SendMessageToConsole(PlayerController, L"Toggled flight!");
					}
				}
				else
				{
					SendMessageToConsole(PlayerController, L"Movement mode not found!");
					return;
				}
			}
			else
			{
				SendMessageToConsole(PlayerController, L"Character movement not found!");
				return;
			}
		}
		else if (Command == "ghost")
		{
			auto Pawn = Cast<APawn>(ReceivingController->GetPawn());

			if (!Pawn)
			{
				SendMessageToConsole(PlayerController, L"No pawn!");
				return;
			}

			Pawn->SetActorEnableCollision(!Pawn->GetActorEnableCollision());

			static auto CharMovementOffset = Pawn->GetOffset("CharacterMovement");
			if (CharMovementOffset != -1)
			{
				auto CharMovement = Pawn->Get<UObject*>(CharMovementOffset);

				static auto MovementOffset = CharMovement->GetOffset("MovementMode", false);
				if (MovementOffset != -1)
				{
					EMovementMode MovementMode = CharMovement->Get<EMovementMode>(MovementOffset);
					EMovementMode NewMode = EMovementMode::MOVE_Walking;

					if (MovementMode != EMovementMode::MOVE_Flying)
					{
						NewMode = EMovementMode::MOVE_Flying;
					}

					static auto SetMovementModeFn = FindObject<UFunction>(L"/Script/Engine.CharacterMovementComponent.SetMovementMode");

					if (SetMovementModeFn)
					{
						CharMovement->ProcessEvent(SetMovementModeFn, &NewMode);
						SendMessageToConsole(PlayerController, L"Toggled ghost!");
					}
				}
				else
				{
					SendMessageToConsole(PlayerController, L"Movement mode not found!");
					return;
				}
			}
			else
			{
				SendMessageToConsole(PlayerController, L"Character movement not found!");
				return;
			}
		}
		else if (Command == "ballermove")
		{
			auto Pawn = Cast<AFortPlayerPawn>(ReceivingController->GetMyFortPawn());

			if (!Pawn)
			{
				SendMessageToConsole(PlayerController, L"No pawn found!");
				return;
			}

			auto Vehicle = Pawn->GetVehicle();

			if (!Vehicle)
			{
				SendMessageToConsole(PlayerController, L"No vehicle found!");
				return;
			}

			/*
			 
			[02/18/24-02:06:08] LogDev: Rotation: 0.000000, -0.000000, -1.000000
			[02/18/24-02:06:08] LogDev: Translation: -117158.195312, -113867.359375, 3847.555420
			[02/18/24-02:06:08] LogDev: LinearVelocity: 1.496917, -0.250735, -0.000311
			[02/18/24-02:06:08] LogDev: AngularVelocity: 0.000000, 0.000000, 0.000000
			[02/18/24-02:06:08] LogDev: SyncKey: 53640

			*/

			struct FReplicatedPhysicsPawnState
			{
				FQuat Rotation;
				FVector Translation;
				FVector LinearVelocity;
				FVector AngularVelocity;
				uint16 SyncKey;
			};

			FReplicatedPhysicsPawnState State{};
			State.Rotation = FRotator(0.f, -0.f, -1.f).Quaternion();
			State.Translation = Vehicle->GetActorLocation() + FVector(1000, 1000, 1000);
			State.LinearVelocity = FVector(200, 250, 300);
			State.AngularVelocity = FVector(0, 0, 0);
			State.SyncKey = 53640;

			static auto ServerUpdatePhysicsParamsFn = FindObject<UFunction>(L"/Script/FortniteGame.FortPhysicsPawn.ServerUpdatePhysicsParams");
			Vehicle->ProcessEvent(ServerUpdatePhysicsParamsFn, &State);
		}
		else if (Command == "setspeed" || Command == "speed")
		{
			float Speed = 1.0f;

			if (Arguments.size() > 1 && Arguments[1] != " ")
			{
				try { Speed = std::stof(Arguments[1]); }
				catch (...) {}
			}

			auto Pawn = Cast<APawn>(ReceivingController->GetPawn());

			if (!Pawn)
			{
				SendMessageToConsole(PlayerController, L"No pawn to set speed!");
				return;
			}

			static auto SetMovementSpeedFn = FindObject<UFunction>(L"/Script/FortniteGame.FortPawn.SetMovementSpeed") ? FindObject<UFunction>(L"/Script/FortniteGame.FortPawn.SetMovementSpeed") :
				FindObject<UFunction>(L"/Script/FortniteGame.FortPawn.SetMovementSpeedMultiplier");

			if (!SetMovementSpeedFn)
			{
				SendMessageToConsole(PlayerController, L"SetMovementSpeed not found!");
				return;
			}

			Pawn->ProcessEvent(SetMovementSpeedFn, &Speed);

			std::wstringstream ss;
			ss << std::fixed << std::setprecision(0) << Speed;

			std::wstring Message = L"Speed set to " + ss.str() + L"!\n";
			SendMessageToConsole(PlayerController, Message.c_str());
		}
		else if (Command == "startcreativegame")
		{
			bool bCanEdit = false;
			TArray<FString> WhiteList;

			for (int i = 0; i < ClientConnections.Num(); i++)
			{
				auto ClientConnection = ClientConnections.at(i);

				auto PlayerController = ClientConnection->GetPlayerController();

				auto PlayerState = PlayerController->GetPlayerState();

				auto Name = PlayerState->GetPlayerName();

				WhiteList.Add(Name);

				LOG_INFO(LogDev, "Name added to whitelist: {}", Name.ToString());
			}

			static auto Server_SetCanEditCreativeIslandFn = FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerStateAthena.Server_SetCanEditCreativeIsland");

			struct
			{
				bool bCanEdit;
				TArray<FString> WhiteList;
			}params{ bCanEdit , WhiteList };

			ReceivingController->GetPlayerState()->ProcessEvent(Server_SetCanEditCreativeIslandFn, &params);

			SendMessageToConsole(ReceivingController, L"Started Game.");
		}
		else if (Command == "wipequickbar" || Command == "wipeall" || Command == "wipe" || Command == "clear" || Command == "clearall")
		{
			bool bWipePrimary = false;
			bool bWipeSecondary = false;
			bool bCheckShouldBeDropped = true;

			bool bWipeSingularQuickbar = Command != "wipeall" || Command != "clearall";

			if (bWipeSingularQuickbar)
			{
				if (Arguments.size() <= 1)
				{
					SendMessageToConsole(PlayerController, L"Please provide \"primary\" or \"secondary\"!\n");
					return;
				}

				std::string quickbarType = Arguments[1];
				std::transform(quickbarType.begin(), quickbarType.end(), quickbarType.begin(), ::tolower);

				if (quickbarType == "primary") bWipePrimary = true;
				if (quickbarType == "secondary") bWipeSecondary = true;
			}
			else
			{
				bWipePrimary = true;
				bWipeSecondary = true;
			}

			if (!bWipePrimary && !bWipeSecondary)
			{
				SendMessageToConsole(PlayerController, L"Please provide \"primary\" or \"secondary\"!\n");
				return;
			}

			if (Arguments.size() > 1 + bWipeSingularQuickbar)
			{
				std::string bypassCanBeDropped = Arguments[1 + bWipeSingularQuickbar];
				std::transform(bypassCanBeDropped.begin(), bypassCanBeDropped.end(), bypassCanBeDropped.begin(), ::tolower);

				if (bypassCanBeDropped == "true") bCheckShouldBeDropped = true;
				else if (bypassCanBeDropped == "false") bCheckShouldBeDropped = false;
			}

			auto WorldInventory = ReceivingController->GetWorldInventory();

			if (!WorldInventory)
			{
				SendMessageToConsole(PlayerController, L"Player does not have a WorldInventory!\n");
				return;
			}

			static auto FortEditToolItemDefinitionClass = FindObject<UClass>(L"/Script/FortniteGame.FortEditToolItemDefinition");
			static auto FortBuildingItemDefinitionClass = FindObject<UClass>(L"/Script/FortniteGame.FortBuildingItemDefinition");

			std::vector<std::pair<FGuid, int>> GuidsAndCountsToRemove;
			const auto& ItemInstances = WorldInventory->GetItemList().GetItemInstances();
			auto PickaxeInstance = WorldInventory->GetPickaxeInstance();

			for (int i = 0; i < ItemInstances.Num(); ++i)
			{
				auto ItemInstance = ItemInstances.at(i);
				const auto ItemDefinition = Cast<UFortWorldItemDefinition>(ItemInstance->GetItemEntry()->GetItemDefinition());

				if (bCheckShouldBeDropped
					? ItemDefinition->CanBeDropped()
					: !ItemDefinition->IsA(FortBuildingItemDefinitionClass)
					&& !ItemDefinition->IsA(FortEditToolItemDefinitionClass)
					&& ItemInstance != PickaxeInstance
					)
				{
					bool IsPrimary = IsPrimaryQuickbar(ItemDefinition);

					if ((bWipePrimary && IsPrimary) || (bWipeSecondary && !IsPrimary))
					{
						GuidsAndCountsToRemove.push_back({ ItemInstance->GetItemEntry()->GetItemGuid(), ItemInstance->GetItemEntry()->GetCount() });
					}
				}
			}

			for (auto& [Guid, Count] : GuidsAndCountsToRemove)
			{
				WorldInventory->RemoveItem(Guid, nullptr, Count, true);
			}

			WorldInventory->Update();

			SendMessageToConsole(PlayerController, L"Cleared!\n");
		}
		else if (Command == "destroytarget" || Command == "destroy")
		{
			UCheatManager*& CheatManager = ReceivingController->SpawnCheatManager(UCheatManager::StaticClass());

			if (!CheatManager)
			{
				SendMessageToConsole(PlayerController, L"Failed to spawn player's cheat manager!");
				return;
			}

			CheatManager->DestroyTarget();
			CheatManager = nullptr;
			SendMessageToConsole(PlayerController, L"Destroyed target!");
		}
		else if (Command == "buildfree" || Command == "infmats")
		{
			Globals::bInfiniteMaterials = !Globals::bInfiniteMaterials;

			if (Globals::bInfiniteMaterials == true)
			{
				SendMessageToConsole(PlayerController, L"Infinite Materials enabled!");
			}

			if (Globals::bInfiniteMaterials == false)
			{
				SendMessageToConsole(PlayerController, L"Infinite Materials disabled.");
			}
		}
		else if (Command == "infiniteammo" || Command == "infammo")
		{
			Globals::bInfiniteAmmo = !Globals::bInfiniteAmmo;

			if (Globals::bInfiniteAmmo == true)
			{
				SendMessageToConsole(PlayerController, L"Infinite Ammo enabled!");
			}

			if (Globals::bInfiniteAmmo == false)
			{
				SendMessageToConsole(PlayerController, L"Infinite Ammo disabled.");
			}
		}
		else if (Command == "bugitgo" || Command == "b")
		{
			if (Arguments.size() <= 3)
			{
				SendMessageToConsole(PlayerController, L"Please provide X, Y, and Z!\n");
				return;
			}

			float X{}, Y{}, Z{};

			try { X = std::stof(Arguments[1]); }
			catch (...) {}
			try { Y = std::stof(Arguments[2]); }
			catch (...) {}
			try { Z = std::stof(Arguments[3]); }
			catch (...) {}

			auto Pawn = Cast<APawn>(ReceivingController->GetPawn());

			if (!Pawn)
			{
				SendMessageToConsole(PlayerController, L"No pawn to teleport!");
				return;
			}

			Pawn->TeleportTo(FVector(X, Y, Z), Pawn->GetActorRotation());
			SendMessageToConsole(PlayerController, L"Teleported to Coordinates!");

			static auto LaunchCharacterFn = FindObject<UFunction>(L"/Script/Engine.Character.LaunchCharacter");

			struct
			{
				FVector LaunchVelocity;
				bool bXYOverride;
				bool bZOverride;
				bool bIgnoreFallDamage;
			} ACharacter_LaunchCharacter_Params{ FVector(0.0f, 0.0f, -10000000.0f), false, false, true }; // sort of works to stop momentum

			Pawn->ProcessEvent(LaunchCharacterFn, &ACharacter_LaunchCharacter_Params);
		}
		/*else if (Command == "tpto")
		{
			if (Arguments.size() < 2)
			{
				SendMessageToConsole(PlayerController, L"Please specify a player's name using \\PlayerName\\.");
				return;
			}

			FString TargetPlayerName = FString(Arguments[1]); // E0289

			APlayerController* TargetController = FindPlayerControllerByName(TargetPlayerName); // E0020

			if (!TargetController)
			{
				SendMessageToConsole(PlayerController, L"Player not found!");
				return;
			}

			auto TargetPawn = TargetController->GetPawn(); // E0135

			if (!TargetPawn)
			{
				SendMessageToConsole(PlayerController, L"Target player does not have a valid pawn.");
				return;
			}

			FVector TargetLocation = TargetPawn->GetActorLocation();

			auto ExecutingPawn = Cast<APawn>(ReceivingController->GetPawn());

			if (!ExecutingPawn)
			{
				SendMessageToConsole(PlayerController, L"You do not have a valid pawn to teleport.");
				return;
			}

			ExecutingPawn->TeleportTo(TargetLocation, ExecutingPawn->GetActorRotation());
			SendMessageToConsole(PlayerController, L"Teleported to Player!");
		}*/
		else if (Command == "healthall")
		{
			for (int i = 0; i < ClientConnections.Num(); i++)
			{
				auto PlayerController = Cast<AFortPlayerController>(ClientConnections.at(i)->GetPlayerController());

				if (!PlayerController->IsValidLowLevel())
					continue;

				auto Pawn = PlayerController->GetMyFortPawn();

				Pawn->SetHealth(100.f);
			}

			SendMessageToConsole(PlayerController, L"Healed all players health!\n");
		}
		else if (Command == "shieldall")
		{
			for (int i = 0; i < ClientConnections.Num(); i++)
			{
				auto PlayerController = Cast<AFortPlayerController>(ClientConnections.at(i)->GetPlayerController());

				if (!PlayerController->IsValidLowLevel())
					continue;

				auto Pawn = PlayerController->GetMyFortPawn();

				Pawn->SetShield(100.f);
			}

			SendMessageToConsole(PlayerController, L"Healed all players shield!\n");
		}
		else if (Command == "regenall")
		{
			for (int i = 0; i < ClientConnections.Num(); i++)
			{
				auto PlayerController = Cast<AFortPlayerController>(ClientConnections.at(i)->GetPlayerController());

				if (!PlayerController->IsValidLowLevel())
					continue;

				auto Pawn = PlayerController->GetMyFortPawn();

				Pawn->SetHealth(100.f);
				Pawn->SetShield(100.f);
			}

			SendMessageToConsole(PlayerController, L"Regenerated health and shield for all players!\n");
		}
		else if (Command == "giveall" || Command == "grantall")
		{
			static auto World_NetDriverOffset = GetWorld()->GetOffset("NetDriver");
			auto WorldNetDriver = GetWorld()->Get<UNetDriver*>(World_NetDriverOffset);
			auto& ClientConnections = WorldNetDriver->GetClientConnections();

			for (int z = 0; z < ClientConnections.Num(); z++)
			{
				auto ClientConnection = ClientConnections.at(z);
				auto FortPC = Cast<AFortPlayerController>(ClientConnection->GetPlayerController());

				if (!FortPC)
					continue;

				auto WorldInventory = FortPC->GetWorldInventory();

				if (!WorldInventory)
					continue;

				static auto WoodItemData = FindObject<UFortItemDefinition>(
					L"/Game/Items/ResourcePickups/WoodItemData.WoodItemData");
				static auto StoneItemData = FindObject<UFortItemDefinition>(
					L"/Game/Items/ResourcePickups/StoneItemData.StoneItemData");
				static auto MetalItemData = FindObject<UFortItemDefinition>(
					L"/Game/Items/ResourcePickups/MetalItemData.MetalItemData");
				static auto Gold = FindObject<UFortItemDefinition>(
					L"/Game/Items/ResourcePickups/Athena_WadsItemData.Athena_WadsItemData");

				static auto Sniper = FindObject<UFortItemDefinition>(
					L"");
				static auto Secondary = FindObject<UFortItemDefinition>(
					L"");
				static auto Tertiary = FindObject<UFortItemDefinition>(
					L"");
				static auto Consumable1 = FindObject<UFortItemDefinition>(
					L"");
				static auto Consumable2 = FindObject<UFortItemDefinition>(
					L"");

				static auto Bouncer = FindObject<UFortItemDefinition>(
					L"/Game/Athena/Items/Traps/TID_Context_BouncePad_Athena.TID_Context_BouncePad_Athena");
				static auto LaunchPad = FindObject<UFortItemDefinition>(
					L"/Game/Athena/Items/Traps/TID_Floor_Player_Launch_Pad_Athena.TID_Floor_Player_Launch_Pad_Athena");
				static auto DirBouncePad = FindObject<UFortItemDefinition>(
					L"/Game/Athena/Items/Traps/TID_Floor_Player_Jump_Pad_Free_Direction_Athena.TID_Floor_Player_Jump_Pad_Free_Direction_Athena");
				static auto FreezeTrap = FindObject<UFortItemDefinition>(
					L"/Game/Athena/Items/Traps/TID_Context_Freeze_Athena.TID_Context_Freeze_Athena");
				static auto SpeedBoost = FindObject<UFortItemDefinition>(
					L"/Game/Athena/Items/Traps/TID_Context_SpeedBoost.TID_Context_SpeedBoost");
				static auto Campfire = FindObject<UFortItemDefinition>(
					L"/Game/Athena/Items/Traps/TID_Floor_Player_Campfire_Athena.TID_Floor_Player_Campfire_Athena");

				static auto HeavyAmmo = FindObject<UFortItemDefinition>(
					L"/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsHeavy.AthenaAmmoDataBulletsHeavy");
				static auto ShellsAmmo = FindObject<UFortItemDefinition>(
					L"/Game/Athena/Items/Ammo/AthenaAmmoDataShells.AthenaAmmoDataShells");
				static auto MediumAmmo = FindObject<UFortItemDefinition>(
					L"/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsMedium.AthenaAmmoDataBulletsMedium");
				static auto LightAmmo = FindObject<UFortItemDefinition>(
					L"/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsLight.AthenaAmmoDataBulletsLight");
				static auto RocketAmmo = FindObject<UFortItemDefinition>(
					L"/Game/Athena/Items/Ammo/AmmoDataRockets.AmmoDataRockets");
				static auto ExplosiveAmmo = FindObject<UFortItemDefinition>(
					L"/Game/Items/Ammo/AmmoDataExplosive.AmmoDataExplosive");
				static auto EnergyCells = FindObject<UFortItemDefinition>(
					L"/Game/Items/Ammo/AmmoDataEnergyCell.AmmoDataEnergyCell");
				static auto Arrows = FindObject<UFortItemDefinition>(
					L"/PrimalGameplay/Items/Ammo/AthenaAmmoDataArrows.AthenaAmmoDataArrows");
				static auto ReconAmmo = FindObject<UFortItemDefinition>(
					L"/MotherGameplay/Items/Scooter/Ammo_Athena_Mother_Scooter.Ammo_Athena_Mother_Scooter");

				WorldInventory->AddItem(WoodItemData, nullptr, 999);
				WorldInventory->AddItem(StoneItemData, nullptr, 999);
				WorldInventory->AddItem(MetalItemData, nullptr, 999);
				WorldInventory->AddItem(Gold, nullptr, 10000);
				WorldInventory->AddItem(Sniper, nullptr, 1);
				WorldInventory->AddItem(Secondary, nullptr, 1);
				WorldInventory->AddItem(Tertiary, nullptr, 1);
				WorldInventory->AddItem(Consumable1, nullptr, 1);
				WorldInventory->AddItem(Consumable2, nullptr, 10);
				WorldInventory->AddItem(ShellsAmmo, nullptr, 999);
				WorldInventory->AddItem(HeavyAmmo, nullptr, 999);
				WorldInventory->AddItem(MediumAmmo, nullptr, 999);
				WorldInventory->AddItem(LightAmmo, nullptr, 999);
				WorldInventory->AddItem(RocketAmmo, nullptr, 999);
				WorldInventory->AddItem(ExplosiveAmmo, nullptr, 999);
				WorldInventory->AddItem(EnergyCells, nullptr, 999);
				WorldInventory->AddItem(Arrows, nullptr, 30);
				WorldInventory->AddItem(ReconAmmo, nullptr, 999);
				WorldInventory->AddItem(Bouncer, nullptr, 999);
				WorldInventory->AddItem(LaunchPad, nullptr, 999);
				WorldInventory->AddItem(DirBouncePad, nullptr, 999);
				WorldInventory->AddItem(FreezeTrap, nullptr, 999);
				WorldInventory->AddItem(SpeedBoost, nullptr, 999);
				WorldInventory->AddItem(Campfire, nullptr, 999);

				WorldInventory->Update();
			}

			SendMessageToConsole(PlayerController, L"Gave all players ammo, materials, and traps!\n");
		}
		else if (Command == "randomizeall")
		{
			static auto World_NetDriverOffset = GetWorld()->GetOffset("NetDriver");
			auto WorldNetDriver = GetWorld()->Get<UNetDriver*>(World_NetDriverOffset);
			auto& ClientConnections = WorldNetDriver->GetClientConnections();

			for (int z = 0; z < ClientConnections.Num(); z++)
			{
				auto ClientConnection = ClientConnections.at(z);
				auto FortPC = Cast<AFortPlayerController>(ClientConnection->GetPlayerController());

				if (!FortPC)
					continue;

				auto WorldInventory = FortPC->GetWorldInventory();

				if (!WorldInventory)
					continue;

				static auto WoodItemData = FindObject<UFortItemDefinition>(
					L"/Game/Items/ResourcePickups/WoodItemData.WoodItemData");
				static auto StoneItemData = FindObject<UFortItemDefinition>(
					L"/Game/Items/ResourcePickups/StoneItemData.StoneItemData");
				static auto MetalItemData = FindObject<UFortItemDefinition>(
					L"/Game/Items/ResourcePickups/MetalItemData.MetalItemData");
				static auto Gold = FindObject<UFortItemDefinition>(
					L"/Game/Items/ResourcePickups/Athena_WadsItemData.Athena_WadsItemData");

				static UFortItemDefinition* Primary = nullptr;

				do
				{
					Primary = FindObject<UFortItemDefinition>(GetRandomItem(Primaries, z), nullptr, ANY_PACKAGE);
				} while (!Primary);

				static UFortItemDefinition* Secondary = nullptr;

				do
				{
					Secondary = FindObject<UFortItemDefinition>(GetRandomItem(Secondaries, z), nullptr, ANY_PACKAGE);
				} while (!Secondary);

				static UFortItemDefinition* Tertiary = nullptr;

				do
				{
					Tertiary = FindObject<UFortItemDefinition>(GetRandomItem(Tertiaries, z), nullptr, ANY_PACKAGE);
				} while (!Tertiary);

				static UFortItemDefinition* Consumable1 = nullptr;

				do
				{
					Consumable1 = FindObject<UFortItemDefinition>(GetRandomItem(Consumables1, z), nullptr, ANY_PACKAGE);
				} while (!Consumable1);

				static UFortItemDefinition* Consumable2 = nullptr;

				do
				{
					Consumable2 = FindObject<UFortItemDefinition>(GetRandomItem(Consumables2, z), nullptr, ANY_PACKAGE);
				} while (!Consumable2);

				static UFortItemDefinition* Trap = nullptr;

				do
				{
					Trap = FindObject<UFortItemDefinition>(GetRandomItem(Traps, z), nullptr, ANY_PACKAGE);
				} while (!Trap);

				static auto HeavyAmmo = FindObject<UFortItemDefinition>(
					L"/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsHeavy.AthenaAmmoDataBulletsHeavy");
				static auto ShellsAmmo = FindObject<UFortItemDefinition>(
					L"/Game/Athena/Items/Ammo/AthenaAmmoDataShells.AthenaAmmoDataShells");
				static auto MediumAmmo = FindObject<UFortItemDefinition>(
					L"/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsMedium.AthenaAmmoDataBulletsMedium");
				static auto LightAmmo = FindObject<UFortItemDefinition>(
					L"/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsLight.AthenaAmmoDataBulletsLight");
				static auto RocketAmmo = FindObject<UFortItemDefinition>(
					L"/Game/Athena/Items/Ammo/AmmoDataRockets.AmmoDataRockets");
				static auto ExplosiveAmmo = FindObject<UFortItemDefinition>(
					L"/Game/Items/Ammo/AmmoDataExplosive.AmmoDataExplosive");
				static auto EnergyCells = FindObject<UFortItemDefinition>(
					L"/Game/Items/Ammo/AmmoDataEnergyCell.AmmoDataEnergyCell");
				static auto Arrows = FindObject<UFortItemDefinition>(
					L"/PrimalGameplay/Items/Ammo/AthenaAmmoDataArrows.AthenaAmmoDataArrows");
				static auto ReconAmmo = FindObject<UFortItemDefinition>(
					L"/MotherGameplay/Items/Scooter/Ammo_Athena_Mother_Scooter.Ammo_Athena_Mother_Scooter");
				static auto STWHeavy = FindObject<UFortItemDefinition>(
					L"/Game/Items/Ammo/AmmoDataBulletsHeavy.AmmoDataBulletsHeavy");
				static auto STWMedium = FindObject<UFortItemDefinition>(
					L"/Game/Items/Ammo/AmmoDataBulletsMedium.AmmoDataBulletsMedium");
				static auto STWLight = FindObject<UFortItemDefinition>(
					L"/Game/Items/Ammo/AmmoDataBulletsLight.AmmoDataBulletsLight");
				static auto STWShells = FindObject<UFortItemDefinition>(
					L"/Game/Items/Ammo/AmmoDataShells.AmmoDataShells");

				WorldInventory->AddItem(WoodItemData, nullptr, (std::rand() % 646) + 186);
				WorldInventory->AddItem(StoneItemData, nullptr, (std::rand() % 646) + 186);
				WorldInventory->AddItem(MetalItemData, nullptr, (std::rand() % 646) + 186);

				if (Fortnite_Version < 15)
				{
					WorldInventory->AddItem(Gold, nullptr, (std::rand() % 7500) + 1200);
				}
				else
				{
					WorldInventory->AddItem(Gold, nullptr, Gold->GetMaxStackSize());
				}

				WorldInventory->AddItem(Primary, nullptr, 1);
				WorldInventory->AddItem(Secondary, nullptr, 1);
				WorldInventory->AddItem(Tertiary, nullptr, 1);
				WorldInventory->AddItem(Consumable1, nullptr, Consumable1->GetMaxStackSize());
				WorldInventory->AddItem(Consumable2, nullptr, Consumable2->GetMaxStackSize());
				WorldInventory->AddItem(ShellsAmmo, nullptr, (std::rand() % 576) + 87);
				WorldInventory->AddItem(HeavyAmmo, nullptr, (std::rand() % 576) + 50);
				WorldInventory->AddItem(MediumAmmo, nullptr, (std::rand() % 824) + 186);
				WorldInventory->AddItem(LightAmmo, nullptr, (std::rand() % 824) + 124);
				WorldInventory->AddItem(RocketAmmo, nullptr, (std::rand() % 12) + 3);
				WorldInventory->AddItem(ExplosiveAmmo, nullptr, (std::rand() % 999) + 186);
				WorldInventory->AddItem(EnergyCells, nullptr, (std::rand() % 999) + 186);
				WorldInventory->AddItem(STWHeavy, nullptr, (std::rand() % 999) + 186);
				WorldInventory->AddItem(STWMedium, nullptr, (std::rand() % 999) + 186);
				WorldInventory->AddItem(STWLight, nullptr, (std::rand() % 999) + 186);
				WorldInventory->AddItem(STWShells, nullptr, (std::rand() % 999) + 186);
				WorldInventory->AddItem(Arrows, nullptr, (std::rand() % 30) + 12);
				WorldInventory->AddItem(ReconAmmo, nullptr, 999);

				WorldInventory->Update();
			}

			SendMessageToConsole(PlayerController, L"Randomized all player loadouts!\n");
		}
		else if (Command == "godall")
		{
			for (int i = 0; i < ClientConnections.Num(); i++)
			{
				auto Pawn = ReceivingController->GetMyFortPawn();

				if (!Pawn)
				{
					SendMessageToConsole(PlayerController, L"No pawn!");
					return;
				}

				float MaxHealth = Pawn->GetMaxHealth();

				auto HealthSet = Pawn->GetHealthSet();

				if (!HealthSet)
				{
					SendMessageToConsole(PlayerController, L"No HealthSet!");
					return;
				}

				static auto HealthOffset = HealthSet->GetOffset("Health");
				auto& Health = HealthSet->Get<FFortGameplayAttributeData>(HealthOffset);

				if (Health.GetMinimum() != MaxHealth)
				{
					Health.GetMinimum() = MaxHealth;
					SendMessageToConsole(PlayerController, L"God of all players ON.");
				}
				else
				{
					Health.GetMinimum() = 0;
					SendMessageToConsole(PlayerController, L"God of all players OFF.");
				}
			}
		}
		else if (Command == "tpalltomax" || Command == "allmax")
		{
			for (int i = 0; i < ClientConnections.Num(); i++)
			{
				auto PlayerController = Cast<AFortPlayerController>(ClientConnections.at(i)->GetPlayerController());
				auto Pawn = PlayerController->GetMyFortPawn();

				Pawn->TeleportTo(FVector{ 0,0,105000 }, FRotator{ 0,0,0 });

				float height = 2000;

				Pawn->ProcessEvent(Pawn->FindFunction("TeleportToSkyDive"), &height);
				SendMessageToConsole(PlayerController, L"Teleported all players to max height!");
			}
		}
		else if (Command == "oldgodall" || Command == "canbedamagedall")
		{
			for (int i = 0; i < ClientConnections.Num(); i++)
			{
				static auto GodFn = FindObject<UFunction>(L"/Script/Engine.CheatManager.God");

				auto PlayerController = Cast<AFortPlayerController>(ClientConnections.at(i)->GetPlayerController());

				if (!PlayerController->IsValidLowLevel())
					continue;

				if (GodFn)
				{
					auto CheatManager = PlayerController->SpawnCheatManager(UCheatManager::StaticClass());

					if (!CheatManager)
					{
						SendMessageToConsole(PlayerController, L"Failed to spawn player's cheat manager!");
						return;
					}

					CheatManager->God();
					CheatManager = nullptr;
				}
				else
				{
					auto Pawn = PlayerController->GetMyFortPawn();
					Pawn->SetCanBeDamaged(!Pawn->CanBeDamaged());
					SendMessageToConsole(PlayerController, std::wstring(L"God of all players set to " + std::to_wstring(!(bool)Pawn->CanBeDamaged())).c_str());
				}
			}
		}
		else if (Command == "givenames")
		{
			SendMessageToConsole(PlayerController, L"ar_uc || Uncommon Assalt Rifle");
			SendMessageToConsole(PlayerController, L"ar_r/ar || Rare Assalt Rifle");
			SendMessageToConsole(PlayerController, L"ar_vr/scar_vr || Epic Assalt Rifle");
			SendMessageToConsole(PlayerController, L"ar_sr/scar_sr || Legendary Assalt Rifle");
			SendMessageToConsole(PlayerController, L"ar_ur/scar_ur/skyesar || Mythic Skye's Assalt Rifle");
			SendMessageToConsole(PlayerController, L"minigun_vr || Epic Minigun");
			SendMessageToConsole(PlayerController, L"minigun_sr || Legendary Minigun");
			SendMessageToConsole(PlayerController, L"minigun_ur/brutus || Mythic Brutus' Minigun");
			SendMessageToConsole(PlayerController, L"pump_uc || Uncommon Pump Shotgun");
			SendMessageToConsole(PlayerController, L"pump_r || Rare Pump Shotgun");
			SendMessageToConsole(PlayerController, L"pump_vr/spaz_vr || Epic Pump Shotgun");
			SendMessageToConsole(PlayerController, L"pump_sr/spaz_sr/spaz/pump || Legendary Pump Shotgun");
			SendMessageToConsole(PlayerController, L"tac_uc || Uncommon Tactical Shotgun");
			SendMessageToConsole(PlayerController, L"tac_r || Rare Tactical Shotgun");
			SendMessageToConsole(PlayerController, L"tac_vr || Epic Tactical Shotgun");
			SendMessageToConsole(PlayerController, L"tac_sr/tac || Legendary Tactical Shotgun");
			SendMessageToConsole(PlayerController, L"doublebarrel_vr || Epic Double Barrel Shotgun");
			SendMessageToConsole(PlayerController, L"doublebarrel_sr/doublebarrel || Legendary Double Barrel Shotgun");
			SendMessageToConsole(PlayerController, L"flint_c || Common Flint Knock Pistol");
			SendMessageToConsole(PlayerController, L"flint_uc/flint || Uncommon Flint Knock Pistol");
			SendMessageToConsole(PlayerController, L"deagle_vr || Epic Hand Cannon");
			SendMessageToConsole(PlayerController, L"deagle_sr/deagle || Legendary Hand Cannon");
			SendMessageToConsole(PlayerController, L"heavy_r || Rare Heavy Sniper Rifle");
			SendMessageToConsole(PlayerController, L"heavy_vr || Epic Heavy Sniper Rifle");
			SendMessageToConsole(PlayerController, L"heavy_sr/heavy || Legendary Heavy Sniper Rifle");
			SendMessageToConsole(PlayerController, L"hunting_uc || Uncommon Hunting Rifle");
			SendMessageToConsole(PlayerController, L"hunting_r || Rare Hunting Rifle");
			SendMessageToConsole(PlayerController, L"hunting_vr || Epic Hunting Rifle");
			SendMessageToConsole(PlayerController, L"hunting_sr/hunting || Legendary Hunting Rifle");
			SendMessageToConsole(PlayerController, L"bolt_c || Common Bolt Action Sniper Rifle");
			SendMessageToConsole(PlayerController, L"bolt_uc || Uncommon Bolt Action Sniper Rifle");
			SendMessageToConsole(PlayerController, L"bolt_r/bolt || Rare Bolt Action Sniper Rifle");
			SendMessageToConsole(PlayerController, L"bolt_vr || Epic Bolt Action Sniper Rifle");
			SendMessageToConsole(PlayerController, L"bolt_sr || Legendary Bolt Action Sniper Rifle");
			SendMessageToConsole(PlayerController, L"suppressed_vr || Epic Suppressed Sniper Rifle");
			SendMessageToConsole(PlayerController, L"suppressed_sr/suppressed || Legendary Suppressed Sniper Rifle");
			SendMessageToConsole(PlayerController, L"semi_uc || Uncommon Semi-Auto Sniper Rifle");
			SendMessageToConsole(PlayerController, L"semi_r/semi || Rare Semi-Auto Sniper Rifle");
			SendMessageToConsole(PlayerController, L"stormscout_vr || Epic Storm Scout Sniper Rifle");
			SendMessageToConsole(PlayerController, L"stormscout_sr/stormscout || Legendary Storm Scout Sniper Rifle");
			SendMessageToConsole(PlayerController, L"lever_uc || Uncommon Lever Action Sniper Rifle");
			SendMessageToConsole(PlayerController, L"lever_r || Rare Lever Action Sniper Rifle");
			SendMessageToConsole(PlayerController, L"lever_vr/lever || Epic Lever Action Sniper Rifle");
			SendMessageToConsole(PlayerController, L"lever_sr || Legendary Lever Action Sniper Rifle");
			SendMessageToConsole(PlayerController, L"hunterbolt_uc || Uncommon Hunter Bolt Action Sniper Rifle");
			SendMessageToConsole(PlayerController, L"hunterbolt_r || Rare Hunter Bolt Action Sniper Rifle");
			SendMessageToConsole(PlayerController, L"hunterbolt_vr || Epic Hunter Bolt Action Sniper Rifle");
			SendMessageToConsole(PlayerController, L"hunterbolt_sr/hunterbolt || Legendary Hunter Bolt Action Sniper Rifle");
			SendMessageToConsole(PlayerController, L"firesniper/dragonsbreath || Exotic Dragon's Breath Sniper Rifle");
			SendMessageToConsole(PlayerController, L"exstormscout/exoticstormscout || Exotic Storm Scout");
			SendMessageToConsole(PlayerController, L"boom/boomsniper || Exotic Boom Sniper Rifle");
			SendMessageToConsole(PlayerController, L"rocket_r || Rare Rocket Launcher");
			SendMessageToConsole(PlayerController, L"rocket_vr || Epic Rocket Launcher");
			SendMessageToConsole(PlayerController, L"rocket_sr/rocket || Legendary Rocket Launcher");
			SendMessageToConsole(PlayerController, L"pumpkin_uc || Uncommon Pumpkin Launcher");
			SendMessageToConsole(PlayerController, L"pumpkin_r || Rare Pumpkin Launcher");
			SendMessageToConsole(PlayerController, L"pumpkin_vr || Epic Pumpkin Launcher");
			SendMessageToConsole(PlayerController, L"pumpkin_sr/pumpkin || Legendary Pumpkin Launcher");
			SendMessageToConsole(PlayerController, L"gl_r || Rare Grenade Launcher");
			SendMessageToConsole(PlayerController, L"gl_vr || Epic Grenade Launcher");
			SendMessageToConsole(PlayerController, L"gl_sr/gl || Legendary Grenade Launcher");
			SendMessageToConsole(PlayerController, L"quad_vr || Epic Quad Launcher");
			SendMessageToConsole(PlayerController, L"quad_sr/quad/quadlauncher || Legendary Quad Launcher");
			SendMessageToConsole(PlayerController, L"guidedmissile_vr/guided_vr/missile_vr || Epic Guided Missile");
			SendMessageToConsole(PlayerController, L"guidedmissile_sr/guided_sr_missile_sr/guided || Legendary Guided Missile");
			SendMessageToConsole(PlayerController, L"xenonbow/xenon/stwbow || Legendary Xenon Bow");
			SendMessageToConsole(PlayerController, L"kits/kitslauncher || Mythic Kits Shockwave Launcher");
			SendMessageToConsole(PlayerController, L"rift/rifts || Epic Rift-To-Go");
			SendMessageToConsole(PlayerController, L"crashpad/crashes/crash/crashpads || Rare Crashpads");
			SendMessageToConsole(PlayerController, L"chillers/chiller/chillergrenade || Common Chiller Grenades");
			SendMessageToConsole(PlayerController, L"can/rustycan/cans || Common Rusty Cans");
			SendMessageToConsole(PlayerController, L"mythicgoldfish/mythicfish/goldfish || Mythic Goldfish");
			SendMessageToConsole(PlayerController, L"batman || Batman's Grapnel Gun");
			SendMessageToConsole(PlayerController, L"spiderman || (infinite) Spider-Man's Webshooters");
			SendMessageToConsole(PlayerController, L"stink/stinkbomb/stinks || Rare Stink Bombs");
			SendMessageToConsole(PlayerController, L"shieldbubble || Rare Shield Bubbles");
			SendMessageToConsole(PlayerController, L"zaptrap || Epic Zaptraps");
			SendMessageToConsole(PlayerController, L"shockwave/shock/shockwavegrenade/shocks/shockwaves || Epic Shockwave Grenade");
			SendMessageToConsole(PlayerController, L"impulse/impulsegrenade/impulses || Rare Impulse Grenades");
			SendMessageToConsole(PlayerController, L"portafortress/fortress || Legendary Port-A-Fortress");
			SendMessageToConsole(PlayerController, L"hopflop/hopflopper || Epic Hop Flopper");
			SendMessageToConsole(PlayerController, L"slurpfish || Epic Slurpfish");
			SendMessageToConsole(PlayerController, L"zeropoint/zeropointfish || Rare Zero Point Fish");
			SendMessageToConsole(PlayerController, L"chugsplash/chugs || Rare Chug Splash");
			SendMessageToConsole(PlayerController, L"minis || Uncommon Small Shield Potions");
			SendMessageToConsole(PlayerController, L"bandage/bandages || Common Bandages");
			SendMessageToConsole(PlayerController, L"portafort/paf || Rare Port-A-Forts");
			SendMessageToConsole(PlayerController, L"c4 || Epic Remote Explosives");
			SendMessageToConsole(PlayerController, L"firefly/fireflies || Rare Firefly Grenade");
			SendMessageToConsole(PlayerController, L"tire/tires/tyre || Common Chonker Tires");
			SendMessageToConsole(PlayerController, L"doomgauntlets/doom || Mythic Dr. Doom's Arcane Gauntlets");
			SendMessageToConsole(PlayerController, L"dub || Exotic Dub Shotgun");
			SendMessageToConsole(PlayerController, L"hoprockdualies/dualies || Exotic Hop Rock Dualies");
			SendMessageToConsole(PlayerController, L"recon || Rare Recon Scanner");
			SendMessageToConsole(PlayerController, L"jules/julesgrappler/julesgrap || Mythic Jules' Grappler Gun");
			SendMessageToConsole(PlayerController, L"skye/skyesgrappler/skyegrap/skyesgrap || Mythic Skye's Grappler");
			SendMessageToConsole(PlayerController, L"captainamerica/shield/ca || Mythic Captain America's Shield");
			SendMessageToConsole(PlayerController, L"thorshammer/thor/thors || Mythic Thor's Hammer");
			SendMessageToConsole(PlayerController, L"batman/batgrap || Mythic Batman's Grapnel Gun");
			SendMessageToConsole(PlayerController, L"flare/flaregun || Rare Flare Gun");
			SendMessageToConsole(PlayerController, L"grabitron || Epic Grab-itron");
			SendMessageToConsole(PlayerController, L"grappler/grap/grapple || Epic Grappler");
			SendMessageToConsole(PlayerController, L"presents/present || Legendary Presents!");
			SendMessageToConsole(PlayerController, L"balloons/balloon || Rare/Epic Balloons");
			SendMessageToConsole(PlayerController, L"snowman/snowmen || Common/Rare Sneaky Snowman");
			SendMessageToConsole(PlayerController, L"ironman/iron-man || Mythic Iron-Man's Repulsor Gauntlets");
			SendMessageToConsole(PlayerController, L"god/godgun/testgod || Test God Gun");
			SendMessageToConsole(PlayerController, L"harpoon || Rare Harpoon Gun");
			SendMessageToConsole(PlayerController, L"bouncer/bouncers || Rare Bouncer");
			SendMessageToConsole(PlayerController, L"launchpad/launch/pad/launches || Epic Launch Pad");
			SendMessageToConsole(PlayerController, L"rocketammo/rockets || Rocket Ammo");
			SendMessageToConsole(PlayerController, L"heavyammo || Heavy Ammo");
			SendMessageToConsole(PlayerController, L"shells || Shells");
			SendMessageToConsole(PlayerController, L"medium/mediumammo || Medium Ammo");
			SendMessageToConsole(PlayerController, L"light/lightammo || Light Ammo");
		}
		else if (Command == "spawnnames")
		{
			SendMessageToConsole(PlayerController, L"driftboard/hoverboard");
			SendMessageToConsole(PlayerController, L"surfboard");
			SendMessageToConsole(PlayerController, L"quadcrasher/quad");
			SendMessageToConsole(PlayerController, L"baller");
			SendMessageToConsole(PlayerController, L"plane");
			SendMessageToConsole(PlayerController, L"golfcart/golf");
			SendMessageToConsole(PlayerController, L"cannon");
			SendMessageToConsole(PlayerController, L"shoppingcart/shopping");
			SendMessageToConsole(PlayerController, L"mech/brute");
			SendMessageToConsole(PlayerController, L"bear/truck");
			SendMessageToConsole(PlayerController, L"prevelant/car");
			SendMessageToConsole(PlayerController, L"whiplash/sportscar");
			SendMessageToConsole(PlayerController, L"taxi");
			SendMessageToConsole(PlayerController, L"mudflap");
			SendMessageToConsole(PlayerController, L"stark || Stark's Whiplash");
			SendMessageToConsole(PlayerController, L"boat");
			SendMessageToConsole(PlayerController, L"heli/helicopter");
			SendMessageToConsole(PlayerController, L"ufo");
			SendMessageToConsole(PlayerController, L"shark");
			SendMessageToConsole(PlayerController, L"klombo");
			SendMessageToConsole(PlayerController, L"umbrella");
			SendMessageToConsole(PlayerController, L"dumpster");
			SendMessageToConsole(PlayerController, L"tire");
			SendMessageToConsole(PlayerController, L"llama");
			SendMessageToConsole(PlayerController, L"airvent");
			SendMessageToConsole(PlayerController, L"geyser");
			SendMessageToConsole(PlayerController, L"nobuildzone");
			SendMessageToConsole(PlayerController, L"launch/launchpad");
			SendMessageToConsole(PlayerController, L"gascan/gas");
			SendMessageToConsole(PlayerController, L"supplydrop");
			SendMessageToConsole(PlayerController, L"zeropoint");
			SendMessageToConsole(PlayerController, L"lowgrav/lowgravzone");
		}

		else { bSendHelpMessage = true; };
	}
	else { bSendHelpMessage = true; };

	if (bSendHelpMessage)
	{
		FString HelpMessage = LR"(
- cheat ban - Permanently bans the player from the game. (IP Ban)
- cheat bot {#} - Spawns a bot at the player (experimental).
- cheat buildfree - Toggles Infinite Materials.
- cheat bugitgo {X Y Z} - Teleport to a location.
- cheat damagetarget {#} - Damages the Actor in front of you by the specified amount.
- cheat demospeed - Speeds up/slows down the speed of the game.
- cheat destroy - Destroys the actor that the player is looking at.
- cheat destroyall {ClassPathName} - Destroys every actor of a given class. Useful for removing all floorloot for example.
- cheat falldamage - Permanently turn off fall damage.
- cheat fly - Toggles flight.
- cheat ghost - Toggles flight and disables collision.
- cheat give {NAMEOFITEM_RARITY || Use cheat givenames} - Gives a weapon using a shortcut name, without ID.
- cheat giveall - Gives all players Ammo, Materials, and Traps maxed out.
- cheat giveitem {ID} - Gives a weapon to the executing player, if inventory is full drops a pickup on the player.
- cheat givenames - Sends a message to the console of all of the names that work with the "cheat give" command.
- cheat godall - Gods all players.
- cheat health {#} - Sets executing player's health.
- cheat healthall - Heals all players health.
- cheat infammo - Toggles Infinite Ammo.
- cheat kick - Kicks the player from the game.
- cheat killserver - Ends the running task of the hosting window (host only).
- cheat launch/fling {X Y Z} - Launches a player.
- cheat listplayers - Gives you all players names.
- cheat maxhealth {#} - Sets the maximum health of the player.
- cheat maxshield {#} - Sets the maximum shield of the player.
- cheat pausesafezone - Pauses the zone.
- cheat pausetimeofday - Pauses the current time of day cycle.
- cheat pickaxe {ID} - Set player's pickaxe. Can be either the PID or WID.
- cheat pickaxeall {ID} - Gives all players a specified pickaxe.
- cheat regen - Regenerates the players health and shield to their max.
- cheat regenall - Heals all players health and shield.
- cheat rift - Rifts the player into the air.
- cheat savewaypoint {phrase/number} - Gets the location of where you are standing and saves it as a waypoint.
- cheat settimeofday {1-23} - Changes the time of day in game to a 24H time period.
- cheat shield {#} - Sets executing player's shield.
- cheat shieldall - Heals all players shield.
- cheat siphon {#} - Changes the amount of health and shield a player gets for killing someone.
- cheat skin {CID} - Sets a player's skin.
- cheat skinall {CID} - Gives all players a certain skin.
- cheat spawn {name of object, use cheat spawnnames} {optional: X, Y, Z} - Spawns a blueprint actor on player using a shorter name.
- cheat spawnnames - Sends a message to the console of all of the names that work with the "cheat spawn" command.
- cheat spawnpickup {WID} {#} - Spawns a pickup at specified player.
- cheat speed - Changes player's movement speed (buggy running but works with cheat fly).
- cheat startaircraft - Starts the bus.
- cheat summon {full path of object} {optional: X, Y, Z} - Summons the specified blueprint class at the executing player's location. Note: There is a limit on the count.
- cheat suicide - Insta-kills player.
- cheat togglesnowmap - Toggles the map to have snow or not. (7.10, 7.30, 11.31, 15.10, 19.01, & 19.10 ONLY).
- cheat tp - Teleports to what the player is looking at.
- cheat tpalltomax - Teleports everyone to max height, in the middle of the map.
- cheat tptomax - Teleports player to max height, in the middle of the map.
- cheat tutorial - Opens the Project Reboot V3 Tutorial (host only).
- cheat waypoint {saved phrase/number} - Teleports the player to the selected existing waypoint.
- cheat wipe/clear {Primary = Guns || Secondary = Ammo & Mats} - Removes the specified quickbar.
- cheat wipeall/clearall - Removes the player's entire inventory.

- NOTE: If you want to execute a command on a player, quote their name after the command. Example: cheat op "Ralzify")";

		SendMessageToConsole(PlayerController, HelpMessage);
	}
}