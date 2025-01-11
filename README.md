![Banner](https://fortnite.gg/img/items/1154/icon.png?2)

# What is Reboot Ultimate?

Reboot Ultimate is a rewrite of the original [Project Reboot 3.0](https://github.com/Milxnor/Project-Reboot-3.0). This version of Reboot strives for adding new things to Reboot, while keeping it all stable.
Alot of these updates wouldn't be possible without [Max's](https://github.com/max8447) help. I will not take all of the credit for this.

# What does this version have?

## In-Game:

- [x] Siphon (Animation, Health, and Materials)
- [x] Spawning Quaternion
- [x] Fixed Foundations (minus maybe some stuff on s17)
- [x] Fixed S18+ Win Screen
- [x] Fixed S19 Traps (s/o sweefy)

## UI:

- [x] Recolored to be dark mode.
- [x] You can now choose playlists in the Pre-Game tabs, instead of pasting the ID.
- [x] New Pregame Bots Tab, with configurable settings, like changing the bot's name, cosmetics, and functionality.
- [x] Added Players Tab
- [x] Toggle Victory Crown Slomo
- [x] Added more indicators (Uptime, Started, Ended, Gamemode, Players)
- [x] Zone Tab now shows a status message when you click to pause/skip zone.

## Commands (compared to old reboot ultimate):

- [x] cheat savewaypoint {phrase} - Will get the players current location, and save the coordinates based on the phrase.
- [x] cheat waypoint {phrase} - (based on the last command) Will teleport the player to a waypoint using the specified phrase.
- [x] cheat rift - Rifts the player.
- [x] cheat infammo - Toggles Infinite Ammo.
- [x] cheat buildfree - Toggles Infinite Materials.
- [x] cheat launch - (reworked) ; If you want to get flung directly up, you now only need to type "cheat launch 8000" instead of "cheat launch 0 0 8000".
- [x] cheat ghost - Toggles flight and no-clip, allowing you to phase through everything.
- [x] cheat demospeed {#} - Changes the overral speed of the game.
- [x] cheat siphon {#} - Changes the amount of health/shield the player gets per elimination.
- [x] cheat regen - Regenerates all of the players health.
- [x] cheat tptomax - Teleports the player to max height.
- [x] cheat tpalltomax - Teleports all players to max height.
- [x] cheat pausetime - Pauses the current time of day cycle.
- [x] cheat getscript - Fixed so that non-host players cannot run this command.
- [x] cheat tutorial - Fixed so that non-host players cannot run this command.
- [x] cheat spawn - Allows for spawning at a location (ex: cheat spawn quad -120397.375000, 4183.438477, 7969.918457), as well as quaternation spawning.
- [x] cheat skin - Similar to cheat give, you can now type a phrase (ex: cheat skin renegaderaider) and it will put on Renegade Raider.
- [x] cheat pickaxe - Similar to cheat give, you can now type a phrase (ex: cheat pickaxe raidersrevenge) and it will put on the Raider's Revenge pickaxe.

## To Do List:
- [ ] Fix Whirlpools, Witch Brooms (Silver Surfer's Board)
- [ ] Fix PC Bot Siphon Animation

# Installation

1. Click the green "<> Code 🔽" button, and click "Download ZIP". Extract it using [WinRAR](https://www.rarlab.com/rar/winrar-x64-701.exe).
2. Open the project using your preferred build tools. (Recommended: Visual Studio Community 2022, C++). 
3. At the top, click the green "▶ Local Windows Debugger" Button. This will start to compile the code into a suitable .dll file. If you get a popup/"error", it's just because it can't run it as a .dll (you need to inject it into your og build to run it)
4. Go to C:\Users\YourPCUser\Downloads\Reboot-Ultimate-main\x64\Release, there will be a "Reboot Ultimate.dll file there."
5. Run the server using the Reboot Launcher/another launcher of choice.
5 (extended/explained). If you are using Reboot Launcher, Go to settings, and find either "Reboot DLL", or "Gameserver DLL". Put the dll there. 

# Links

- [Original Project Reboot 3.0 Github](https://github.com/Milxnor/Project-Reboot-3.0)

- [Reboot TS Discord Server](https://discord.gg/invite/rPc5t4usPe)

