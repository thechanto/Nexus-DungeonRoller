// Fill out your copyright notice in the Description page of Project Settings.

#include "NexusLockOnComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Blueprint/UserWidget.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "EngineUtils.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "Kismet/KismetMathLibrary.h"
#include "NexusEnemyBase.h"
#include "Nexus/GameplayAbilitySystem/AttributeSets/BasicAttributeSet.h"

DEFINE_LOG_CATEGORY_STATIC(LogLockOn, Log, All);

namespace
{
	/**
	 * A target counts as dead on any one of three tells. State.Dead only ever lands on the
	 * player/BP_NexusEnemy_Base hierarchy, so it cannot stand alone: the shipping enemies under
	 * /Game/Enemies never receive it. GAS Health is what actually reaches zero on them, and the
	 * Blueprint death pipeline ragdolls the corpse in place rather than destroying the actor --
	 * so a simulating mesh is a death too.
	 */
	bool IsTargetDead(const AActor* Target)
	{
		if (!IsValid(Target))
		{
			return true;
		}

		if (const IAbilitySystemInterface* AscInterface = Cast<IAbilitySystemInterface>(Target))
		{
			if (const UAbilitySystemComponent* ASC = AscInterface->GetAbilitySystemComponent())
			{
				if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName(TEXT("State.Dead")))))
				{
					return true;
				}

				if (const UBasicAttributeSet* Attributes = ASC->GetSet<UBasicAttributeSet>())
				{
					if (Attributes->GetHealth() <= 0.f)
					{
						return true;
					}
				}
			}
		}

		if (const USkeletalMeshComponent* Mesh = Target->FindComponentByClass<USkeletalMeshComponent>())
		{
			if (Mesh->IsSimulatingPhysics())
			{
				return true;
			}
		}

		return false;
	}
}

UNexusLockOnComponent::UNexusLockOnComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	// Steer after movement/input have run so the lock rotation wins the frame
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void UNexusLockOnComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerCharacter = Cast<ACharacter>(GetOwner());

	// Project defaults; resolved here so the component works with no per-instance setup
	if (!LockOnAction)
	{
		LockOnAction = LoadObject<UInputAction>(nullptr, TEXT("/Game/Input/Actions/IA_LockOn.IA_LockOn"));
	}
	if (!IndicatorWidgetClass)
	{
		IndicatorWidgetClass = LoadClass<UUserWidget>(nullptr, TEXT("/Game/Widgets/W_LockOnIndicator.W_LockOnIndicator_C"));
	}
}

void UNexusLockOnComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	BreakLock();
	Super::EndPlay(EndPlayReason);
}

void UNexusLockOnComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bInputBound)
	{
		TryBindInput();
	}

	if (!LockedTarget)
	{
		return;
	}

	if (IsTargetDead(LockedTarget))
	{
		HandleTargetDeath(DeltaTime);
		return;
	}

	// Anything else that invalidates the lock (walked out of BreakDistance) is a plain break:
	// leaving a live target must not silently snap the lock onto a different enemy.
	if (!IsTargetStillValid())
	{
		BreakLock();
		return;
	}

	SteerCamera(DeltaTime);
}

void UNexusLockOnComponent::HandleTargetDeath(float DeltaTime)
{
	// While a cinematic owns the view (finisher shot), sit on the dead lock: acquisition scores
	// against the active camera, so retargeting here would both pick from the cine camera's
	// viewpoint and pop a fresh indicator into the middle of the shot. Steering is skipped too --
	// the victim's ragdoll is being thrown, and framing a flying corpse would whip the control
	// rotation the player gets handed back.
	const APlayerController* PC = GetPlayerController();
	if (PC && PC->GetViewTarget() != PC->GetPawn())
	{
		return;
	}

	// FindBestTarget rejects the dead, so the corpse we are still holding cannot be re-picked.
	AActor* NextTarget = FindBestTarget();
	BreakLock();

	if (NextTarget)
	{
		LockOn(NextTarget);
		SteerCamera(DeltaTime);
		UE_LOG(LogLockOn, Log, TEXT("Target died; lock switched to %s"), *NextTarget->GetName());
	}
	else
	{
		UE_LOG(LogLockOn, Log, TEXT("Target died; no valid enemy in range, lock cleared"));
	}
}

void UNexusLockOnComponent::TryBindInput()
{
	if (!OwnerCharacter || !LockOnAction)
	{
		return;
	}

	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(OwnerCharacter->InputComponent);
	if (!Input)
	{
		return;
	}

	Input->BindAction(LockOnAction, ETriggerEvent::Started, this, &UNexusLockOnComponent::ToggleLockOn);
	bInputBound = true;
}

void UNexusLockOnComponent::ToggleLockOn()
{
	if (LockedTarget)
	{
		BreakLock();
		return;
	}

	if (AActor* Target = FindBestTarget())
	{
		LockOn(Target);
	}
}

AActor* UNexusLockOnComponent::FindBestTarget() const
{
	if (!OwnerCharacter)
	{
		return nullptr;
	}

	const APlayerController* PC = GetPlayerController();
	if (!PC || !PC->PlayerCameraManager)
	{
		return nullptr;
	}

	const FVector CameraLocation = PC->PlayerCameraManager->GetCameraLocation();
	const FVector CameraForward = PC->PlayerCameraManager->GetCameraRotation().Vector();
	const FVector OwnerLocation = OwnerCharacter->GetActorLocation();

	AActor* Best = nullptr;
	float BestScore = -FLT_MAX;

	// Corpses keep QueryAndPhysics collision (the Blueprint death pipeline only disables the
	// capsule), so a ragdoll blocks ECC_Visibility and would veto every enemy standing behind it --
	// and they accumulate over a fight. Gather the dead once and ignore them all for every trace.
	TArray<AActor*> Corpses;
	for (TActorIterator<ANexusEnemyBase> It(GetWorld()); It; ++It)
	{
		if (IsTargetDead(*It))
		{
			Corpses.Add(*It);
		}
	}

	for (TActorIterator<ANexusEnemyBase> It(GetWorld()); It; ++It)
	{
		ANexusEnemyBase* Enemy = *It;
		if (IsTargetDead(Enemy))
		{
			continue;
		}

		const float Distance = FVector::Dist(OwnerLocation, Enemy->GetActorLocation());
		if (Distance > LockOnRange)
		{
			UE_LOG(LogLockOn, Log, TEXT("  candidate %s REJECTED range (dist=%.0f > %.0f)"),
				*Enemy->GetName(), Distance, LockOnRange);
			continue;
		}

		const FVector FramePoint = GetTargetFramePoint(Enemy);
		const FVector ToEnemy = (FramePoint - CameraLocation).GetSafeNormal();
		const float Dot = FVector::DotProduct(CameraForward, ToEnemy);
		if (Dot < 0.f) // behind the camera
		{
			UE_LOG(LogLockOn, Log, TEXT("  candidate %s REJECTED behind-camera (dist=%.0f, dot=%.2f)"),
				*Enemy->GetName(), Distance, Dot);
			continue;
		}

		// Line of sight from the camera; anything solid in between disqualifies
		FHitResult Hit;
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(NexusLockOnLOS), false, OwnerCharacter);
		QueryParams.AddIgnoredActor(Enemy);
		QueryParams.AddIgnoredActors(Corpses);
		const bool bBlocked = GetWorld()->LineTraceSingleByChannel(Hit, CameraLocation, FramePoint, ECC_Visibility, QueryParams);
		if (bBlocked)
		{
			// Name the blocker: this is what tells corpse-in-the-way apart from cave geometry.
			const AActor* Blocker = Hit.GetActor();
			UE_LOG(LogLockOn, Log, TEXT("  candidate %s REJECTED line-of-sight (dist=%.0f, dot=%.2f, blocked by %s / %s)"),
				*Enemy->GetName(), Distance, Dot,
				Blocker ? *Blocker->GetName() : TEXT("<none>"),
				Hit.GetComponent() ? *Hit.GetComponent()->GetName() : TEXT("<none>"));
			continue;
		}

		// Screen-center weighting first, distance as tiebreaker
		const float Score = Dot - 0.5f * (Distance / LockOnRange);
		UE_LOG(LogLockOn, Log, TEXT("  candidate %s ACCEPTED (dist=%.0f, dot=%.2f, score=%.3f)"),
			*Enemy->GetName(), Distance, Dot, Score);

		if (Score > BestScore)
		{
			BestScore = Score;
			Best = Enemy;
		}
	}

	UE_LOG(LogLockOn, Log, TEXT("FindBestTarget: %d corpse(s) ignored, picked %s"),
		Corpses.Num(), Best ? *Best->GetName() : TEXT("<none>"));

	return Best;
}

void UNexusLockOnComponent::LockOn(AActor* Target)
{
	LockedTarget = Target;

	if (OwnerCharacter)
	{
		if (UCharacterMovementComponent* Movement = OwnerCharacter->GetCharacterMovement())
		{
			bSavedOrientRotationToMovement = Movement->bOrientRotationToMovement;
			bSavedUseControllerDesiredRotation = Movement->bUseControllerDesiredRotation;
			Movement->bOrientRotationToMovement = false;
			Movement->bUseControllerDesiredRotation = true;
		}
	}

	if (APlayerController* PC = GetPlayerController())
	{
		PC->SetIgnoreLookInput(true);
	}

	if (IndicatorWidgetClass)
	{
		USkeletalMeshComponent* TargetMesh = Target->FindComponentByClass<USkeletalMeshComponent>();
		IndicatorComponent = NewObject<UWidgetComponent>(Target);
		IndicatorComponent->SetWidgetSpace(EWidgetSpace::Screen);
		IndicatorComponent->SetWidgetClass(IndicatorWidgetClass);
		IndicatorComponent->SetDrawSize(IndicatorDrawSize);
		IndicatorComponent->SetupAttachment(TargetMesh ? Cast<USceneComponent>(TargetMesh) : Target->GetRootComponent(),
			TargetMesh && TargetMesh->DoesSocketExist(TargetSocketName) ? TargetSocketName : NAME_None);
		IndicatorComponent->RegisterComponent();
	}

	OnLockOnChanged.Broadcast(true, Target);
}

void UNexusLockOnComponent::BreakLock()
{
	if (!LockedTarget)
	{
		return;
	}

	LockedTarget = nullptr;

	if (OwnerCharacter)
	{
		if (UCharacterMovementComponent* Movement = OwnerCharacter->GetCharacterMovement())
		{
			Movement->bOrientRotationToMovement = bSavedOrientRotationToMovement;
			Movement->bUseControllerDesiredRotation = bSavedUseControllerDesiredRotation;
		}
	}

	if (APlayerController* PC = GetPlayerController())
	{
		PC->SetIgnoreLookInput(false);
	}

	if (IndicatorComponent)
	{
		IndicatorComponent->DestroyComponent();
		IndicatorComponent = nullptr;
	}

	OnLockOnChanged.Broadcast(false, nullptr);
}

bool UNexusLockOnComponent::IsTargetStillValid() const
{
	if (!IsValid(LockedTarget))
	{
		return false;
	}

	if (const IAbilitySystemInterface* AscInterface = Cast<IAbilitySystemInterface>(LockedTarget.Get()))
	{
		if (const UAbilitySystemComponent* ASC = AscInterface->GetAbilitySystemComponent())
		{
			if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName(TEXT("State.Dead")))))
			{
				return false;
			}
		}
	}

	if (OwnerCharacter && FVector::Dist(OwnerCharacter->GetActorLocation(), LockedTarget->GetActorLocation()) > BreakDistance)
	{
		return false;
	}

	return true;
}

void UNexusLockOnComponent::SteerCamera(float DeltaTime)
{
	APlayerController* PC = GetPlayerController();
	if (!PC || !PC->PlayerCameraManager)
	{
		return;
	}

	const FVector CameraLocation = PC->PlayerCameraManager->GetCameraLocation();
	FRotator Desired = UKismetMathLibrary::FindLookAtRotation(CameraLocation, GetTargetFramePoint(LockedTarget));
	Desired.Pitch = FMath::ClampAngle(Desired.Pitch - ExtraPitchDown, -80.f, 80.f);
	Desired.Roll = 0.f;

	const FRotator Smoothed = FMath::RInterpTo(PC->GetControlRotation(), Desired, DeltaTime, CameraInterpSpeed);
	PC->SetControlRotation(Smoothed);
}

FVector UNexusLockOnComponent::GetTargetFramePoint(const AActor* Target) const
{
	if (const USkeletalMeshComponent* Mesh = Target->FindComponentByClass<USkeletalMeshComponent>())
	{
		if (Mesh->DoesSocketExist(TargetSocketName))
		{
			return Mesh->GetSocketLocation(TargetSocketName);
		}
	}
	return Target->GetActorLocation() + FVector(0.f, 0.f, 40.f);
}

APlayerController* UNexusLockOnComponent::GetPlayerController() const
{
	return OwnerCharacter ? Cast<APlayerController>(OwnerCharacter->GetController()) : nullptr;
}
