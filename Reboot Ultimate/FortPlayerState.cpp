#include "FortPlayerState.h"

#include "KismetStringLibrary.h"

void AFortPlayerState::UpdateScoreStatChanged()
{
	static auto fn = FindObject<UFunction>(L"/Script/FortniteGame.FortPlayerState.UpdateScoreStatChanged");
	this->ProcessEvent(fn);
}

void AFortPlayerState::EndDBNOAbilities()
{
	static auto GAB_AthenaDBNOClass = FindObject<UClass>(L"/Game/Abilities/NPC/Generic/GAB_AthenaDBNO.Default__GAB_AthenaDBNO_C");

	auto ASC = this->GetAbilitySystemComponent();

	if (!ASC)
		return;

	FGameplayAbilitySpec* DBNOSpec = nullptr;

	UObject* ClassToFind = GAB_AthenaDBNOClass->ClassPrivate;

	auto compareAbilities = [&DBNOSpec, &ClassToFind](FGameplayAbilitySpec* Spec) {
		auto CurrentAbility = Spec->GetAbility();

		if (CurrentAbility->ClassPrivate == ClassToFind)
		{
			DBNOSpec = Spec;
			return;
		}
	};

	LoopSpecs(ASC, compareAbilities);

	if (!DBNOSpec)
		return;

	ASC->ClientCancelAbility(DBNOSpec->GetHandle(), DBNOSpec->GetActivationInfo());
	ASC->ClientEndAbility(DBNOSpec->GetHandle(), DBNOSpec->GetActivationInfo());
	ASC->ServerEndAbility(DBNOSpec->GetHandle(), DBNOSpec->GetActivationInfo(), nullptr);
}

void AFortPlayerState::ApplySiphonEffect()
{
	if (!GetAbilitySystemComponent())
		return;

	static FGameplayTag GameplayCueTag = FGameplayTag(UKismetStringLibrary::Conv_StringToName(L"GameplayCue.Shield.PotionConsumed"));

	GetAbilitySystemComponent()->NetMulticast_InvokeGameplayCueAdded(GameplayCueTag);
	GetAbilitySystemComponent()->NetMulticast_InvokeGameplayCueExecuted(GameplayCueTag);
}

void AFortPlayerState::DisableFallDamage()
{
	if (!GetAbilitySystemComponent())
		return;

	static auto BGAClass = FindObject<UClass>(L"/Script/Engine.BlueprintGeneratedClass");
	UClass* GE_Class = LoadObject<UClass>(L"/Game/Athena/Items/Gameplay/BackPacks/Ashton/GE_AshtonPack_FallDamageImmune.GE_AshtonPack_FallDamageImmune_C", BGAClass);

	GetAbilitySystemComponent()->ApplyGameplayEffectToSelf(GE_Class, -1.f);
}

void AFortPlayerState::ApplyHealthEffect()
{
	if (!GetAbilitySystemComponent())
		return;

	static FGameplayTag GameplayCueTag = FGameplayTag(UKismetStringLibrary::Conv_StringToName(L"GameplayCue.Athena.Health.HealUsed"));

	GetAbilitySystemComponent()->NetMulticast_InvokeGameplayCueAdded(GameplayCueTag);
	GetAbilitySystemComponent()->NetMulticast_InvokeGameplayCueExecuted(GameplayCueTag);
}

void AFortPlayerState::ApplyCrownEffect()
{
	if (!GetAbilitySystemComponent())
		return;

	static auto BGAClass = FindObject<UClass>(L"/Script/Engine.BlueprintGeneratedClass");
	UClass* GE_Class = LoadObject<UClass>(L"/VictoryCrownsGameplay/Items/GE_CrownVisuals.GE_CrownVisuals_C", BGAClass);

	GetAbilitySystemComponent()->ApplyGameplayEffectToSelf(GE_Class, -1.f);
}

bool AFortPlayerState::AreUniqueIDsIdentical(FUniqueNetIdRepl* A, FUniqueNetIdRepl* B)
{
	return A->IsIdentical(B);
}