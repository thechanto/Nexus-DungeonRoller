// Fill out your copyright notice in the Description page of Project Settings.


#include "BasicAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

UBasicAttributeSet::UBasicAttributeSet()
{
	Health = 100.f;
	MaxHealth = 100.f;
	Stamina = 100.f;
	MaxStamina = 100.f;
	Damage = 0.f;
	Shield = 0.f;
	MaxShield = 5.f;

	// RPG Stats - start at 1, StatPoints start at 0
	Strength = 50.f;
	Dexterity = 1.f;
	Intelligence = 50.f;
	Faith = 1.f;
	Level = 1.f;
	StatPoints = 10.f;

	// Mana
	Mana = 100.f;
	MaxMana = 100.f;
	
	ShieldBuffTags.AddTag(FGameplayTag::RequestGameplayTag("Status.Buff.Shield"));
}

void UBasicAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION_NOTIFY(UBasicAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBasicAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBasicAttributeSet, Stamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBasicAttributeSet, MaxStamina, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBasicAttributeSet, Strength, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBasicAttributeSet, Dexterity, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBasicAttributeSet, Intelligence, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBasicAttributeSet, Faith, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBasicAttributeSet, Level, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBasicAttributeSet, StatPoints, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBasicAttributeSet, Mana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBasicAttributeSet, MaxMana, COND_None, REPNOTIFY_Always);
}

void UBasicAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	} else if (Attribute == GetStaminaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxStamina());	
	} else if (Attribute == GetShieldAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxShield());
	}else if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxMana());
	}
	else if (Attribute == GetStrengthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 1.f, 99.f);
	}
	else if (Attribute == GetDexterityAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 1.f, 99.f);
	}
	else if (Attribute == GetIntelligenceAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 1.f, 99.f);
	}
	else if (Attribute == GetFaithAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 1.f, 99.f);
	}
	else if (Attribute == GetLevelAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 1.f, 10.f);
	}
	else if (Attribute == GetStatPointsAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, 99.f);
	} 
}

void UBasicAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		float TotalDamage = GetDamage();
		/*
		float StrengthMultiplier = 1.0f + (GetStrength() * 0.05f);
		TotalDamage *= StrengthMultiplier;
		*/
		SetDamage(0.f);

		const float CurrentShield = GetShield();
		if (CurrentShield > 0.f)
		{
			SetShield(CurrentShield - TotalDamage);
			float RemainingDamage = TotalDamage - CurrentShield;

			if (RemainingDamage > 0.f)
			{
				SetHealth(GetHealth() - RemainingDamage);
			}
		} else
		{
			SetHealth(GetHealth() - TotalDamage);	
		}

		if (Data.EffectSpec.Def->GetAssetTags().HasTag(FGameplayTag::RequestGameplayTag("Effects.HitReaction"))
			&& Data.EvaluatedData.Magnitude != 0.f)
		{
			FGameplayTagContainer HitReactionTagContainer;
			HitReactionTagContainer.AddTag(FGameplayTag::RequestGameplayTag("GameplayAbility.HitReaction"));
			GetOwningAbilitySystemComponent()->TryActivateAbilitiesByTag(HitReactionTagContainer);	
		}
	}

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(GetHealth());
	} 
	else if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
		SetStamina(GetStamina());
	}
	else if (Data.EvaluatedData.Attribute == GetManaAttribute())  
	{
		SetMana(GetMana());
	}
}

void UBasicAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	if (Attribute == GetMaxHealthAttribute())
	{
		SetHealth(GetMaxHealth());
	}
	if (Attribute == GetMaxStaminaAttribute())
	{
		SetStamina(GetMaxStamina());
	}
	if (Attribute == GetMaxManaAttribute())
	{
		SetMana(GetMaxMana());
	}
	
	if (Attribute == GetHealthAttribute() && NewValue <= 0.f)
	{
		FGameplayTagContainer DeathAbilityTagContainer;
		DeathAbilityTagContainer.AddTag(FGameplayTag::RequestGameplayTag("GameplayAbility.Death"));
		GetOwningAbilitySystemComponent()->TryActivateAbilitiesByTag(DeathAbilityTagContainer);
	}
	
	
	if (Attribute == GetShieldAttribute())
	{
		if (NewValue > 0.f && OldValue <= 0.f)
		{
			GetOwningAbilitySystemComponent()->AddGameplayCue(FGameplayTag::RequestGameplayTag("GameplayCue.ShieldUp"));
		} else if (NewValue <= 0.f && OldValue > 0.f)
		{
			GetOwningAbilitySystemComponent()->RemoveGameplayCue(FGameplayTag::RequestGameplayTag("GameplayCue.ShieldUp"));
			GetOwningAbilitySystemComponent()->ExecuteGameplayCue(FGameplayTag::RequestGameplayTag("GameplayCue.ShieldDown"));
		}
	}
}
