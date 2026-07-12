#pragma once

#include "CoreMinimal.h"
#include "NexusGameplayAbility.h"
#include "NexusAbility_UpgradeStat.generated.h"

UENUM(BlueprintType)
enum class ENexusStatType : uint8
{
	Strength      UMETA(DisplayName = "Strength"),
	Dexterity     UMETA(DisplayName = "Dexterity"),
	Intelligence  UMETA(DisplayName = "Intelligence"),
	Faith         UMETA(DisplayName = "Faith"),
	Vitality      UMETA(DisplayName = "Vitality"),
	Endurance     UMETA(DisplayName = "Endurance"), //controls MaxStamina
	Mana          UMETA(DisplayName = "Mana")
};

UCLASS()
class NEXUS_API UNexusAbility_UpgradeStat : public UNexusGameplayAbility
{
	GENERATED_BODY()

public:

	UNexusAbility_UpgradeStat();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat Upgrade")
	ENexusStatType StatToUpgrade = ENexusStatType::Strength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat Upgrade")
	TSubclassOf<UGameplayEffect> StrengthUpgradeEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat Upgrade")
	TSubclassOf<UGameplayEffect> DexterityUpgradeEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat Upgrade")
	TSubclassOf<UGameplayEffect> IntelligenceUpgradeEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat Upgrade")
	TSubclassOf<UGameplayEffect> FaithUpgradeEffect;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat Upgrade")
	TSubclassOf<UGameplayEffect> VitalityUpgradeEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat Upgrade")
	TSubclassOf<UGameplayEffect> EnduranceUpgradeEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat Upgrade")
	TSubclassOf<UGameplayEffect> ManaUpgradeEffect;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

private:
	TSubclassOf<UGameplayEffect> GetEffectForStat() const;
};