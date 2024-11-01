#pragma once

#include <atomic>

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
	extern inline int AmountOfHealthSiphon = 0;
	extern inline bool bShouldUseReplicationGraph = false;

	extern inline bool bBotPC = false;

	extern inline bool bCustomLootpool = false;

	extern inline bool bCrownSlowmo = true;

	extern inline bool bHitReadyToStartMatch = false;
	extern inline bool bInitializedPlaylist = false;
	extern inline bool bStartedListening = false;
	extern inline bool bAutoRestart = false;
	extern inline bool bFillVendingMachines = true;
	extern inline bool bPrivateIPsAreOperator = true;
	extern inline int AmountOfListens = 0;
}

extern inline int NumToSubtractFromSquadId = 0;

extern inline std::string PlaylistShortName = "Solos";

extern inline std::string PlaylistName = "/Game/Athena/Playlists/Playlist_DefaultSolo.Playlist_DefaultSolo";