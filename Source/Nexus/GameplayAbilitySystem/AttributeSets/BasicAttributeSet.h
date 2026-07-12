// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "BasicAttributeSet.generated.h"

/**
 * 
 */
UCLASS()
class NEXUS_API UBasicAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:

	UBasicAttributeSet();
	
	// Health Attributes
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing=OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, Health);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing=OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, MaxHealth);

	// Stamina Attributes
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing=OnRep_Stamina)
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, Stamina);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing=OnRep_MaxStamina)
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, MaxStamina);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, Damage);

	// Shield Attributes
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing=OnRep_Shield)
	FGameplayAttributeData Shield;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, Shield);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing=OnRep_MaxShield)
	FGameplayAttributeData MaxShield;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, MaxShield);
	
	UFUNCTION()
	void OnRep_Strength(const FGameplayAttributeData& OldValue) const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UBasicAttributeSet, Strength, OldValue);
	}

	UFUNCTION()
	void OnRep_Dexterity(const FGameplayAttributeData& OldValue) const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UBasicAttributeSet, Dexterity, OldValue);
	}

	UFUNCTION()
	void OnRep_Intelligence(const FGameplayAttributeData& OldValue) const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UBasicAttributeSet, Intelligence, OldValue);
	}

	UFUNCTION()
	void OnRep_Faith(const FGameplayAttributeData& OldValue) const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UBasicAttributeSet, Faith, OldValue);
	}

	UFUNCTION()
	void OnRep_Level(const FGameplayAttributeData& OldValue) const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UBasicAttributeSet, Level, OldValue);
	}

	UFUNCTION()
	void OnRep_StatPoints(const FGameplayAttributeData& OldValue) const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UBasicAttributeSet, StatPoints, OldValue);
	}

	UFUNCTION()
	void OnRep_Mana(const FGameplayAttributeData& OldValue) const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UBasicAttributeSet, Mana, OldValue);
	}

	UFUNCTION()
	void OnRep_MaxMana(const FGameplayAttributeData& OldValue) const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UBasicAttributeSet, MaxMana, OldValue);
	}

public:
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue) const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UBasicAttributeSet, Health, OldValue);
	}

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue) const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UBasicAttributeSet, MaxHealth, OldValue);
	}

	UFUNCTION()
	void OnRep_Stamina(const FGameplayAttributeData& OldValue) const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UBasicAttributeSet, Stamina, OldValue);
	}

	UFUNCTION()
	void OnRep_MaxStamina(const FGameplayAttributeData& OldValue) const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UBasicAttributeSet, MaxStamina, OldValue);
	}

	UFUNCTION()
	void OnRep_Shield(const FGameplayAttributeData& OldValue) const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UBasicAttributeSet, Shield, OldValue);
	}

	UFUNCTION()
	void OnRep_MaxShield(const FGameplayAttributeData& OldValue) const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UBasicAttributeSet, MaxShield, OldValue);
	}

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	// RPG Stats
	UPROPERTY(BlueprintReadOnly, Category = "Stats", ReplicatedUsing=OnRep_Strength)
	FGameplayAttributeData Strength;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, Strength);

	UPROPERTY(BlueprintReadOnly, Category = "Stats", ReplicatedUsing=OnRep_Dexterity)
	FGameplayAttributeData Dexterity;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, Dexterity);

	UPROPERTY(BlueprintReadOnly, Category = "Stats", ReplicatedUsing=OnRep_Intelligence)
	FGameplayAttributeData Intelligence;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, Intelligence);

	UPROPERTY(BlueprintReadOnly, Category = "Stats", ReplicatedUsing=OnRep_Faith)
	FGameplayAttributeData Faith;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, Faith);

	UPROPERTY(BlueprintReadOnly, Category = "Stats", ReplicatedUsing=OnRep_Level)
	FGameplayAttributeData Level;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, Level);

	UPROPERTY(BlueprintReadOnly, Category = "Stats", ReplicatedUsing=OnRep_StatPoints)
	FGameplayAttributeData StatPoints;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, StatPoints);

	// Mana
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing=OnRep_Mana)
	FGameplayAttributeData Mana;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, Mana);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing=OnRep_MaxMana)
	FGameplayAttributeData MaxMana;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, MaxMana);
	
	FGameplayTagContainer ShieldBuffTags;
};
