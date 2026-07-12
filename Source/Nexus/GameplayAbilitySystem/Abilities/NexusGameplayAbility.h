// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "NexusGameplayAbility.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class EAbilityInputID : uint8
{
	None             UMETA(DisplayName = "None"),              // 0
	PrimaryAbility   UMETA(DisplayName = "Primary Ability"),   // 1
	SecondaryAbility UMETA(DisplayName = "Secondary Ability"), // 2
	DefensiveAbility UMETA(DisplayName = "Defensive Ability"), // 3
	MovementAbility  UMETA(DisplayName = "Movement Ability"),  // 4
	Ability1         UMETA(DisplayName = "Ability 1"),         // 5
	Ability2         UMETA(DisplayName = "Ability 2"),         // 6
	Ability3         UMETA(DisplayName = "Ability 3"),         // 7
	Ability4         UMETA(DisplayName = "Ability 4"),         // 8
};

UCLASS()
class NEXUS_API UNexusGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:

	UNexusGameplayAbility();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	bool ShouldShowInAbilitiesBar = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activation")
	bool AutoActivateWhenGranted = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	EAbilityInputID AbilityInputID = EAbilityInputID::None;

	UFUNCTION(BlueprintCallable, Category = "Ability")
	void SetAbilityLevel(int32 NewLevel);
	
private:
	UFUNCTION(BlueprintCallable, Category = "Helpers")
	bool HasPC() const;
};
