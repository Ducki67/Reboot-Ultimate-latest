#pragma once

#include <atomic>
#include <map>
#include <string>

#include "inc.h"

namespace Globals
{
	extern inline int UPTime = 0;
	extern inline int tickTime = 0;

	extern inline bool bIsTickTiming = false;
	extern inline bool UPTimeStarted = false;

	extern inline bool bCreative = false;
	extern inline bool bGoingToPlayEvent = false;
	extern inline bool bEnableAGIDs = true;
	extern inline bool bNoMCP = false;
	extern inline bool bLogProcessEvent = false;
	extern inline std::atomic<bool> bLateGame(true);

	extern inline bool bInfiniteMaterials = true;
	extern inline bool bInfiniteAmmo = true;
	extern inline int AmountOfHealthSiphon = 50;
	extern inline bool bUseUnrealisticSiphon = false;
	extern inline bool bShouldUseReplicationGraph = false;

	extern inline bool bBotInvincible = false;
	extern inline bool bBotPC = false;
	extern inline bool bBotNames = false;
	extern inline bool bUseRandomSkins = true;
	extern inline int bBotHealth = 21;
	extern inline int bBotShield = 21;
	extern inline std::string BotPickaxeID = "WID_Harvest_HalloweenScythe_Athena_C_T01";
	extern inline std::string BotName = "";
	extern inline std::string BotSkin = "";

	// extern inline std::map<int, std::string> CustomBotNames;
	// extern inline int CurrentBotID = 1;

	extern inline bool bCustomLootpool = false;

	extern inline bool bAutoPauseZone = false;

	extern inline bool bCrownSlowmo = true;

	extern inline bool bBugitgo = false;

	extern inline bool bHitReadyToStartMatch = false;
	extern inline bool bInitializedPlaylist = false;
	extern inline bool bStartedListening = false;
	extern inline bool bAutoRestart = false;
	extern inline bool bFillVendingMachines = true;
	extern inline bool bPrivateIPsAreOperator = true;
	extern inline bool bUseWhitelist = true;
	extern inline int AmountOfListens = 0;
}

extern inline int NumToSubtractFromSquadId = 0;

extern inline std::string PlaylistShortName = "Solos";

extern inline std::string PlaylistName = "/Game/Athena/Playlists/Playlist_DefaultSolo.Playlist_DefaultSolo";