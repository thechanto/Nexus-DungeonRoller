// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "NexusCharacterBase.generated.h"

UCLASS()
class NEXUS_API ANexusCharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ANexusCharacterBase();

	// Ability System Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitySystem")
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitySystem")
	class UBasicAttributeSet* BasicAttributeSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitySystem")
	class UCombatAttributeSet* CombatAttributeSet;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
	EGameplayEffectReplicationMode AscReplicationMode = EGameplayEffectReplicationMode::Mixed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitySystem")
	TArray<TSubclassOf<UGameplayAbility>> StartingAbilities;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void PossessedBy(AController* NewController) override;

	virtual void OnRep_PlayerState() override;

	virtual void OnDeadTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Damage")
	void HandleDeath();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintCallable, Category = "AbilitySystem")
	TArray<FGameplayAbilitySpecHandle> GrantAbilities(TArray<TSubclassOf<UGameplayAbility>> AbilitiesToGrant, int32 Level = 1);

	/**
	 * Grants a single ability bound to the passed InputID instead of the ability
	 * CDO's AbilityInputID, so the same ability class can be bound to any keybind
	 * slot at runtime. Authority-only; returns an invalid handle otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "AbilitySystem")
	FGameplayAbilitySpecHandle GrantAbilityWithInputID(TSubclassOf<UGameplayAbility> AbilityClass, int32 InputID, int32 Level = 1);

	UFUNCTION(BlueprintCallable, Category = "AbilitySystem")
	void RemoveAbilities(TArray<FGameplayAbilitySpecHandle> AbilityHandlesToRemove);

	UFUNCTION(BlueprintCallable, Category = "AbilitySystem")
	void SendAbilitiesChangedEvent();

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "AbilitySystem")
	void ServerSendGameplayEventToSelf(FGameplayEventData EventData);

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "AbilitySystem")
	void MultiSendGameplayEventToActor(AActor* TargetActor, FGameplayEventData EventData);
};
