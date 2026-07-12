// Fill out your copyright notice in the Description page of Project Settings.


#include "NexusGameplayAbility.h"

UNexusGameplayAbility::UNexusGameplayAbility()
{
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("GameplayAbility.Active")));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag("State.Dead"));
}

void UNexusGameplayAbility::SetAbilityLevel(int32 NewLevel)
{
	if (FGameplayAbilitySpec* AbilitySpec = GetCurrentAbilitySpec())
	{
		AbilitySpec->Level = NewLevel;
	}
}

bool UNexusGameplayAbility::HasPC() const
{
	const APawn* PawnObject = Cast<APawn>(GetAvatarActorFromActorInfo());
	if (!PawnObject)
	{
		return false;
	}
	return PawnObject->GetController()->IsA<APlayerController>();
}
