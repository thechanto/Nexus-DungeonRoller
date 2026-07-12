#include "NexusAbility_UpgradeStat.h"
#include "AbilitySystemComponent.h"
#include "../AttributeSets/BasicAttributeSet.h"

UNexusAbility_UpgradeStat::UNexusAbility_UpgradeStat()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UNexusAbility_UpgradeStat::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
    if (!ASC)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    const UBasicAttributeSet* Attrs = ASC->GetSet<UBasicAttributeSet>();
    if (!Attrs || Attrs->GetStatPoints() < 1.f)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    TSubclassOf<UGameplayEffect> EffectToApply = GetEffectForStat();
    if (!EffectToApply)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
    FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(
        EffectToApply, 1.f, EffectContext);

    if (SpecHandle.IsValid())
    {
        ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
    }

    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

TSubclassOf<UGameplayEffect> UNexusAbility_UpgradeStat::GetEffectForStat() const
{
    switch (StatToUpgrade)
    {
        case ENexusStatType::Strength:     return StrengthUpgradeEffect;
        case ENexusStatType::Dexterity:    return DexterityUpgradeEffect;
        case ENexusStatType::Intelligence: return IntelligenceUpgradeEffect;
        case ENexusStatType::Faith:        return FaithUpgradeEffect;
        case ENexusStatType::Vitality:     return VitalityUpgradeEffect;
        case ENexusStatType::Endurance:    return EnduranceUpgradeEffect;
        case ENexusStatType::Mana:         return ManaUpgradeEffect;
        default:                           return nullptr;
    }
}