#pragma once

#include "Object.h"
#include "GameplayAbilitySpec.h"
#include "AttributeSet.h"
#include "GameplayTagContainer.h"
#include "AbilitySystemComponent.h"

// using FPredictionKey = PadHex18;
// using FGameplayEventData = PadHexA8;

// using FPredictionKey = PadHex10; 

struct FGameplayEventData
{
	PadHexB0 Pad;

	static UStruct* GetStruct()
	{
		static auto Struct = FindObject<UStruct>(L"/Script/GameplayAbilities.GameplayEventData");
		return Struct;
	}

	static int GetStructSize() { return GetStruct()->GetPropertiesSize(); }
};

// using FGameplayEventData = __int64;

struct FGameplayEffectContextHandle
{
	unsigned char                                      UnknownData00[0x18];                                      // 0x0000(0x0018) MISSED OFFSET
};

struct FActiveGameplayEffectHandle
{
	int                                                Handle;                                                   // 0x0000(0x0004) (ZeroConstructor, IsPlainOldData)
	bool                                               bPassedFiltersAndWasExecuted;                             // 0x0004(0x0001) (ZeroConstructor, IsPlainOldData)
	unsigned char                                      UnknownData00[0x3];                                       // 0x0005(0x0003) MISSED OFFSET
};

struct FGameplayAbilitySpecContainer : public FFastArraySerializer
{
	TArray<FGameplayAbilitySpec>& GetItems()
	{
		static auto ItemsOffset = FindOffsetStruct("/Script/GameplayAbilities.GameplayAbilitySpecContainer", "Items");
		return *(TArray<FGameplayAbilitySpec>*)(__int64(this) + ItemsOffset);
	}
};

class UAbilitySystemComponent : public UObject
{
public:
	static inline FGameplayAbilitySpecHandle* (*GiveAbilityOriginal)(UAbilitySystemComponent*, FGameplayAbilitySpecHandle*, __int64 inSpec);
	static inline bool (*InternalTryActivateAbilityOriginal)(UAbilitySystemComponent*, FGameplayAbilitySpecHandle Handle, PadHex10 InPredictionKey, UObject** OutInstancedAbility, void* OnGameplayAbilityEndedDelegate, const FGameplayEventData* TriggerEventData);
	static inline bool (*InternalTryActivateAbilityOriginal2)(UAbilitySystemComponent*, FGameplayAbilitySpecHandle Handle, PadHex18 InPredictionKey, UObject** OutInstancedAbility, void* OnGameplayAbilityEndedDelegate, const FGameplayEventData* TriggerEventData);

	void ClientActivateAbilityFailed(FGameplayAbilitySpecHandle AbilityToActivate, int16_t PredictionKey)
	{
		struct { FGameplayAbilitySpecHandle AbilityToActivate; int16_t PredictionKey; } UAbilitySystemComponent_ClientActivateAbilityFailed_Params{ AbilityToActivate, PredictionKey };
		static auto fn = FindObject<UFunction>(L"/Script/GameplayAbilities.AbilitySystemComponent.ClientActivateAbilityFailed");

		this->ProcessEvent(fn, &UAbilitySystemComponent_ClientActivateAbilityFailed_Params);
	}

	void NetMulticast_InvokeGameplayCueAdded(FGameplayTag GameplayCueTag)
	{
		static UFunction* NetMulticast_InvokeGameplayCueAddedFn = FindObject<UFunction>(L"/Script/GameplayAbilities.AbilitySystemComponent.NetMulticast_InvokeGameplayCueAdded");

		if (!NetMulticast_InvokeGameplayCueAddedFn)
			return;

		static int NetMulticast_InvokeGameplayCueExecutedParamSize = NetMulticast_InvokeGameplayCueAddedFn->GetPropertiesSize();

		auto NetMulticast_InvokeGameplayCueAddedParams = Alloc(NetMulticast_InvokeGameplayCueExecutedParamSize);

		if (!NetMulticast_InvokeGameplayCueAddedParams)
			return;

		static auto GameplayCueTagOffset = FindOffsetStruct("/Script/GameplayAbilities.AbilitySystemComponent.NetMulticast_InvokeGameplayCueAdded", "GameplayCueTag");
		*(FGameplayTag*)(__int64(NetMulticast_InvokeGameplayCueAddedParams) + GameplayCueTagOffset) = GameplayCueTag;

		this->ProcessEvent(NetMulticast_InvokeGameplayCueAddedFn, NetMulticast_InvokeGameplayCueAddedParams);
	}

	void NetMulticast_InvokeGameplayCueExecuted(FGameplayTag GameplayCueTag)
	{
		static UFunction* NetMulticast_InvokeGameplayCueExecutedFn = FindObject<UFunction>(L"/Script/GameplayAbilities.AbilitySystemComponent.NetMulticast_InvokeGameplayCueExecuted");

		if (!NetMulticast_InvokeGameplayCueExecutedFn)
			return;

		static int NetMulticast_InvokeGameplayCueExecutedParamSize = NetMulticast_InvokeGameplayCueExecutedFn->GetPropertiesSize();

		auto NetMulticast_InvokeGameplayCueExecutedParams = Alloc(NetMulticast_InvokeGameplayCueExecutedParamSize);

		if (!NetMulticast_InvokeGameplayCueExecutedParams)
			return;

		static auto GameplayCueTagOffset = FindOffsetStruct("/Script/GameplayAbilities.AbilitySystemComponent.NetMulticast_InvokeGameplayCueExecuted", "GameplayCueTag");

		*(FGameplayTag*)(__int64(NetMulticast_InvokeGameplayCueExecutedParams) + GameplayCueTagOffset) = GameplayCueTag;

		this->ProcessEvent(NetMulticast_InvokeGameplayCueExecutedFn, NetMulticast_InvokeGameplayCueExecutedParams);
	}

	TArray<UAttributeSet*>& GetSpawnedAttributes()
	{
		static auto SpawnedAttributesOffset = GetOffset("SpawnedAttributes");
		return Get<TArray<UAttributeSet*>>(SpawnedAttributesOffset);
	}

	FGameplayAbilitySpecContainer* GetActivatableAbilities()
	{
		static auto ActivatableAbilitiesOffset = this->GetOffset("ActivatableAbilities");
		return GetPtr<FGameplayAbilitySpecContainer>(ActivatableAbilitiesOffset);
	}

	UAttributeSet* AddDefaultSubobjectSet(UAttributeSet* Subobject)
	{
		GetSpawnedAttributes().Add(Subobject);
		return Subobject;
	}

	void ServerEndAbility(FGameplayAbilitySpecHandle AbilityToEnd, FGameplayAbilityActivationInfo* ActivationInfo, FPredictionKey* PredictionKey);
	void ClientEndAbility(FGameplayAbilitySpecHandle AbilityToEnd, FGameplayAbilityActivationInfo* ActivationInfo);
	void ClientCancelAbility(FGameplayAbilitySpecHandle AbilityToCancel, FGameplayAbilityActivationInfo* ActivationInfo);
	bool HasAbility(UObject* DefaultAbility);
	FActiveGameplayEffectHandle ApplyGameplayEffectToSelf(UClass* GameplayEffectClass, float Level, const FGameplayEffectContextHandle& EffectContext = FGameplayEffectContextHandle());
	FGameplayEffectContextHandle MakeEffectContext();
	void RemoveActiveGameplayEffectBySourceEffect(UClass* GEClass, int StacksToRemove, UAbilitySystemComponent* Instigator);
	void ConsumeAllReplicatedData(FGameplayAbilitySpecHandle AbilityHandle, FPredictionKey* AbilityOriginalPredictionKey);
	FGameplayAbilitySpecHandle GiveAbilityEasy(UClass* AbilityClass, UObject* SourceObject = nullptr, bool bDoNotRegive = true);
	FGameplayAbilitySpec* FindAbilitySpecFromHandle(FGameplayAbilitySpecHandle Handle);
	void RemoveActiveGameplayEffectBySourceEffect(UClass* GameplayEffect, UAbilitySystemComponent* InstigatorAbilitySystemComponent, int StacksToRemove);
	void ClearAbility(const FGameplayAbilitySpecHandle& Handle);

	static void InternalServerTryActivateAbilityHook(UAbilitySystemComponent* AbilitySystemComponent, FGameplayAbilitySpecHandle Handle, bool InputPressed, const FPredictionKey* PredictionKey, const FGameplayEventData* TriggerEventData);
};

void LoopSpecs(UAbilitySystemComponent* AbilitySystemComponent, std::function<void(FGameplayAbilitySpec*)> func);