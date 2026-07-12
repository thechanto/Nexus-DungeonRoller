// Fill out your copyright notice in the Description page of Project Settings.


#include "NexusCharacterBase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Nexus/GameplayAbilitySystem/NexusAbilitySystemComponent.h"
#include "Nexus/GameplayAbilitySystem/Abilities/NexusGameplayAbility.h"
#include "Nexus/GameplayAbilitySystem/AttributeSets/BasicAttributeSet.h"
#include "Nexus/GameplayAbilitySystem/AttributeSets/CombatAttributeSet.h"
#include "Nexus/UI/NexusAbilityUILibrary.h"
#include "TimerManager.h"

// Sets default values
ANexusCharacterBase::ANexusCharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Add the ability system component
	AbilitySystemComponent = CreateDefaultSubobject<UNexusAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(AscReplicationMode);

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(35.0f, 90.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Add the attribute sets
	BasicAttributeSet = CreateDefaultSubobject<UBasicAttributeSet>(TEXT("BasicAttributeSet"));
	CombatAttributeSet = CreateDefaultSubobject<UCombatAttributeSet>(TEXT("CombatAttributeSet"));

}

// Called when the game starts or when spawned
void ANexusCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	AbilitySystemComponent->RegisterGameplayTagEvent(FGameplayTag::RequestGameplayTag("State.Dead"))
    		.AddUObject(this, &ANexusCharacterBase::OnDeadTagChanged);
}

// Called every frame
void ANexusCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ANexusCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ANexusCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		GrantAbilities(StartingAbilities);

		// Keybind-slot abilities come from the save game, not StartingAbilities.
		if (NewController && NewController->IsPlayerController())
		{
			UNexusAbilityUILibrary::GrantAssignedAbilities(this);

			// The HUD widget is created after possession, so fill the gold counter
			// once it exists; AddGold keeps it current from then on.
			FTimerHandle GoldDisplayTimer;
			GetWorldTimerManager().SetTimer(GoldDisplayTimer,
				FTimerDelegate::CreateWeakLambda(this, [this]()
				{
					UNexusAbilityUILibrary::UpdateGoldDisplay(this);
				}),
				1.0f, false);
		}
	}
}

void ANexusCharacterBase::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}

UAbilitySystemComponent* ANexusCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

TArray<FGameplayAbilitySpecHandle> ANexusCharacterBase::GrantAbilities(
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

	SendAbilitiesChangedEvent();
	return AbilityHandles;
}

FGameplayAbilitySpecHandle ANexusCharacterBase::GrantAbilityWithInputID(
	TSubclassOf<UGameplayAbility> AbilityClass, const int32 InputID, const int32 Level)
{
	if (!AbilitySystemComponent || !HasAuthority() || !AbilityClass)
	{
		return FGameplayAbilitySpecHandle();
	}

	bool ShouldActivate = false;
	if (const UNexusGameplayAbility* NexusAbilityCDO = Cast<UNexusGameplayAbility>(AbilityClass->GetDefaultObject()))
	{
		ShouldActivate = NexusAbilityCDO->AutoActivateWhenGranted;
	}

	const FGameplayAbilitySpecHandle SpecHandle = AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(
		AbilityClass, Level, InputID, this
	));

	if (ShouldActivate)
	{
		AbilitySystemComponent->TryActivateAbility(SpecHandle);
	}

	SendAbilitiesChangedEvent();
	return SpecHandle;
}

void ANexusCharacterBase::RemoveAbilities(TArray<FGameplayAbilitySpecHandle> AbilityHandlesToRemove)
{
	if (!AbilitySystemComponent || !HasAuthority())
	{
		return;
	}

	for (FGameplayAbilitySpecHandle AbilityHandle : AbilityHandlesToRemove)
	{
		AbilitySystemComponent->ClearAbility(AbilityHandle);
	}

	SendAbilitiesChangedEvent();
}

void ANexusCharacterBase::SendAbilitiesChangedEvent()
{
	FGameplayEventData EventData;
	EventData.EventTag = FGameplayTag::RequestGameplayTag(FName("Event.Abilities.Changed"));
	EventData.Instigator = this;
	EventData.Target = this;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, EventData.EventTag, EventData);
}

void ANexusCharacterBase::MultiSendGameplayEventToActor_Implementation(AActor* TargetActor,
	FGameplayEventData EventData)
{
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(TargetActor, EventData.EventTag, EventData);
}

void ANexusCharacterBase::ServerSendGameplayEventToSelf_Implementation(FGameplayEventData EventData)
{
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, EventData.EventTag, EventData);
}

void ANexusCharacterBase::HandleDeath_Implementation()
{
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->DisableMovement();

	FVector Impulse = GetActorForwardVector() * -20000;
	Impulse.Z = 15000;
	GetMesh()->AddImpulseAtLocation(Impulse, GetActorLocation());
}


void ANexusCharacterBase::OnDeadTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	if (NewCount > 0)
	{
		HandleDeath();
	}
}