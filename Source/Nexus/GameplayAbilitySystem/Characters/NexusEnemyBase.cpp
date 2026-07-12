// Fill out your copyright notice in the Description page of Project Settings.

#include "NexusEnemyBase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Nexus/GameplayAbilitySystem/NexusAbilitySystemComponent.h"
#include "Nexus/GameplayAbilitySystem/Abilities/NexusGameplayAbility.h"
#include "Nexus/GameplayAbilitySystem/AttributeSets/BasicAttributeSet.h"
#include "Nexus/GameplayAbilitySystem/AttributeSets/CombatAttributeSet.h"

ANexusEnemyBase::ANexusEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;

	AbilitySystemComponent = CreateDefaultSubobject<UNexusAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(AscReplicationMode);

	GetCapsuleComponent()->InitCapsuleSize(35.0f, 90.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	BasicAttributeSet = CreateDefaultSubobject<UBasicAttributeSet>(TEXT("BasicAttributeSet"));
	CombatAttributeSet = CreateDefaultSubobject<UCombatAttributeSet>(TEXT("CombatAttributeSet"));
}

void ANexusEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	AbilitySystemComponent->RegisterGameplayTagEvent(FGameplayTag::RequestGameplayTag("State.Dead"))
		.AddUObject(this, &ANexusEnemyBase::OnDeadTagChanged);
}

void ANexusEnemyBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (AbilitySystemComponent)
	{
		// Enemies own their ASC directly — owner and avatar are both this actor
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		GrantAbilities(StartingAbilities);
	}
}

UAbilitySystemComponent* ANexusEnemyBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

TArray<FGameplayAbilitySpecHandle> ANexusEnemyBase::GrantAbilities(
	TArray<TSubclassOf<UGameplayAbility>> AbilitiesToGrant, const int32 Level)
{
	if (!AbilitySystemComponent || !HasAuthority())
	{
		return TArray<FGameplayAbilitySpecHandle>();
	}

	TArray<FGameplayAbilitySpecHandle> AbilityHandles;
	for (TSubclassOf<UGameplayAbility> Ability : AbilitiesToGrant)
	{
		int32 InputID = -1;
		bool ShouldActivate = false;
		if (const UNexusGameplayAbility* NexusAbilityCDO = GetDefault<UNexusGameplayAbility>(Ability))
		{
			InputID = static_cast<int32>(NexusAbilityCDO->AbilityInputID);
			ShouldActivate = NexusAbilityCDO->AutoActivateWhenGranted;
		}

		FGameplayAbilitySpecHandle SpecHandle = AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(
			Ability, Level, InputID, this
		));
		AbilityHandles.Add(SpecHandle);

		if (ShouldActivate)
		{
			AbilitySystemComponent->TryActivateAbility(SpecHandle);
		}
	}

	return AbilityHandles;
}

void ANexusEnemyBase::RemoveAbilities(TArray<FGameplayAbilitySpecHandle> AbilityHandlesToRemove)
{
	if (!AbilitySystemComponent || !HasAuthority())
	{
		return;
	}

	for (FGameplayAbilitySpecHandle AbilityHandle : AbilityHandlesToRemove)
	{
		AbilitySystemComponent->ClearAbility(AbilityHandle);
	}
}

void ANexusEnemyBase::HandleDeath_Implementation()
{
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->DisableMovement();

	FVector Impulse = GetActorForwardVector() * -20000;
	Impulse.Z = 15000;
	GetMesh()->AddImpulseAtLocation(Impulse, GetActorLocation());
}

void ANexusEnemyBase::OnDeadTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	if (NewCount > 0)
	{
		HandleDeath();
	}
}
