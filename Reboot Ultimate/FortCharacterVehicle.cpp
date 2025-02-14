#include "FortCharacterVehicle.h"

void AFortCharacterVehicle::FortGameplayAbility_OstrichJump()
{
	static auto FortGameplayAbility_OstrichJumpFn = FindObject<UFunction>(L"/Script/FortniteGame.FortGameplayAbility_OstrichJump");
	this->ProcessEvent(FortGameplayAbility_OstrichJumpFn);
}

void AFortCharacterVehicle::AirDash()
{
	static auto AirDashFn = FindObject<UFunction>(L"/Script/FortniteGame.FortCharacterVehicle_Ostrich.AirDash");
	this->ProcessEvent(AirDashFn);
}