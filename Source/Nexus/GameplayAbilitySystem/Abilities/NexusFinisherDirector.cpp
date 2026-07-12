// Fill out your copyright notice in the Description page of Project Settings.

#include "Nexus/GameplayAbilitySystem/Abilities/NexusFinisherDirector.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Abilities/GameplayAbility.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Camera/CameraShakeBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "UObject/ObjectKey.h"
#include "UObject/SoftObjectPath.h"

#include "Nexus/GameplayAbilitySystem/AttributeSets/BasicAttributeSet.h"

DEFINE_LOG_CATEGORY_STATIC(LogFinisher, Log, All);

namespace FinisherCine
{
	static const FName DamageTagName(TEXT("Data.Damage"));

	static const FSoftClassPath ShakeClassPath(
		TEXT("/Game/VFX/CameraShakes/BP_CameraShake_SkyCrusher.BP_CameraShake_SkyCrusher_C"));
	static const FSoftClassPath BossClassPath(
		TEXT("/Game/Enemies/BossEnemy/BP_Enemy_Boss.BP_Enemy_Boss_C"));

	static const float PollInterval = 0.01f;
	static const float ShakeScale = 1.0f;
	static const float CameraBlendIn = 0.2f;
	static const float CameraBlendOut = 0.35f;

	/** One live director per activation. The director itself is rooted while it runs. */
	static TMap<FObjectKey, TWeakObjectPtr<UNexusFinisherDirector>> ActiveDirectors;
}

// ---------------------------------------------------------------------------------------------
// Profiles
// ---------------------------------------------------------------------------------------------

/**
 * Warrior ultimate. AM_SkyCrusher (length 4.3667s at rate 1.0) carries NO root motion, so the capsule
 * never leaves the ground even though the mesh renders 36 m up. That is why the montage whiffs on a
 * distant target, and why relocating the capsule mid-flight is safe: there is no root-motion delta to
 * fight, and CMC stays in Walking the whole time. Verified in PIE — the relocate frame reports
 * HasAnimRootMotion() = 0 and zero root motion sources, and the capsule holds the spot with no drift.
 *
 * The avatar counter-dilates at the apex, so the world crawls while the slam itself keeps its snap.
 */
const FNexusFinisherProfile& FNexusFinisherProfile::SkyCrusher()
{
	static const FNexusFinisherProfile Profile = []
	{
		FNexusFinisherProfile P;
		P.MontageToken = TEXT("SkyCrusher");
		P.MontagePlayRate = 1.4f;

		P.CameraCutTime = 1.525f;        // feet leave the ground
		P.DilationTime = 2.382f;         // root Z peaks at 3664
		P.RelocateTime = 2.45f;          // just past apex, 0.8s before the slam
		P.DilationRestoreTime = 3.25f;   // the slam itself, at full speed
		P.ImpactEventTag = FName(TEXT("Event.HitScan.Start"));  // the parent's own weapon trace

		P.GlobalDilation = 0.4f;
		P.AvatarCounterDilation = 2.0f;

		P.RelocateStandoff = 90.f;
		P.RelocateZLift = 10.f;

		P.bCameraFollowsVictim = true;   // the player lands on the victim, so the victim is the shot
		P.CameraSideOffset = 520.f;
		P.CameraBackOffset = 180.f;
		P.CameraHeight = 260.f;
		P.CameraLookAtLift = 140.f;
		P.CameraFOV = 80.f;
		P.CameraPlayerBias = 0.18f;
		P.CameraTrackInterpSpeed = 7.f;

		return P;
	}();
	return Profile;
}

/**
 * Mage ultimate: a ranged force push. AM_Burden is 1.4333s at rate 1.0 with no root motion on either the
 * montage or its source sequence (Burden_slow, bEnableRootMotion=False), so the "step forward" is mesh
 * only — the capsule never moves and the mage kills from where he stands. No relocate.
 *
 * The cast lands at 0.60 (where the montage's own AN_SendGameplayEvent sits) and takes the world down to
 * 0.4 with NO avatar counter-dilation: the mage slows along with everything else, which stretches the
 * remaining 0.83s of montage into ~2.1 real seconds of watching the body tumble away. The dilation is
 * therefore never restored on a beat — it holds until the montage ends and the ability unwinds.
 */
const FNexusFinisherProfile& FNexusFinisherProfile::Burden()
{
	static const FNexusFinisherProfile Profile = []
	{
		FNexusFinisherProfile P;
		P.MontageToken = TEXT("Burden");
		P.MontagePlayRate = 1.0f;

		P.CameraCutTime = 0.30f;         // he commits to the cast
		P.ImpactTime = 0.60f;            // AN_SendGameplayEvent (Event.ShootProjectile) sits here
		P.DilationTime = 0.60f;          // slow motion starts on the same beat as the push
		P.DilationRestoreTime = -1.f;    // hold it: the slow tumble is the whole point of the shot
		P.RelocateTime = -1.f;           // ranged: no gap-close

		P.GlobalDilation = 0.4f;
		P.AvatarCounterDilation = 1.f;   // the mage slows too, so his recovery stretches with the world

		// Anchored on the caster, not the victim: the corpse flies away at 1800 cm/s, and a rig that rode
		// it would drag the shot downrange and leave the mage behind. A fixed camera that pans to follow
		// the body past a stationary caster is the shot.
		P.bCameraFollowsVictim = false;
		P.CameraSideOffset = 420.f;
		P.CameraBackOffset = 320.f;
		P.CameraHeight = 220.f;
		P.CameraLookAtLift = 100.f;
		P.CameraFOV = 85.f;
		P.CameraPlayerBias = 0.35f;      // keeps the mage in frame while the body travels
		P.CameraTrackInterpSpeed = 6.f;

		P.bLaunchVictim = true;
		P.VictimLaunchSpeed = 1800.f;
		P.VictimLaunchUpSpeed = 700.f;
		P.VictimTumbleDegrees = 180.f;
		P.CorpseThrowWindow = 0.5f;

		P.bRadialPush = true;
		P.PushRadius = 700.f;
		P.PushConeHalfAngleDeg = 60.f;
		P.PushBackSpeed = 1200.f;
		P.PushUpSpeed = 500.f;

		return P;
	}();
	return Profile;
}

// ---------------------------------------------------------------------------------------------
// UNexusFinisherEffect
// ---------------------------------------------------------------------------------------------

UNexusFinisherEffect::UNexusFinisherEffect()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayModifierInfo Mod;
	Mod.Attribute = UBasicAttributeSet::GetDamageAttribute();
	Mod.ModifierOp = EGameplayModOp::Override;

	FSetByCallerFloat SetByCaller;
	SetByCaller.DataTag = FGameplayTag::RequestGameplayTag(FinisherCine::DamageTagName, /*ErrorIfNotFound*/ false);
	Mod.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);

	Modifiers.Add(Mod);
}

// ---------------------------------------------------------------------------------------------
// Lifetime
// ---------------------------------------------------------------------------------------------

void UNexusFinisherDirector::Arm(UGameplayAbility* InAbility, AActor* InTarget, const FNexusFinisherProfile& InProfile)
{
	if (!InAbility)
	{
		UE_LOG(LogFinisher, Warning, TEXT("Arm: no ability"));
		return;
	}

	// A previous activation that never unwound would otherwise leave the world dilated.
	Restore(InAbility);

	UNexusFinisherDirector* Director = NewObject<UNexusFinisherDirector>(GetTransientPackage());
	Director->AddToRoot();
	FinisherCine::ActiveDirectors.Add(FObjectKey(InAbility), Director);
	Director->Begin(InAbility, InTarget, InProfile);
}

void UNexusFinisherDirector::Restore(UGameplayAbility* InAbility)
{
	if (!InAbility)
	{
		return;
	}

	TWeakObjectPtr<UNexusFinisherDirector> Found;
	if (FinisherCine::ActiveDirectors.RemoveAndCopyValue(FObjectKey(InAbility), Found))
	{
		if (UNexusFinisherDirector* Director = Found.Get())
		{
			Director->Shutdown();
		}
	}
}

void UNexusFinisherDirector::Begin(UGameplayAbility* InAbility, AActor* InTarget, const FNexusFinisherProfile& InProfile)
{
	Profile = InProfile;
	Ability = InAbility;
	Target = InTarget;

	Avatar = Cast<ACharacter>(InAbility->GetAvatarActorFromActorInfo());
	SourceASC = InAbility->GetAbilitySystemComponentFromActorInfo();

	ACharacter* Player = Avatar.Get();
	if (!Player)
	{
		UE_LOG(LogFinisher, Warning, TEXT("Arm: avatar is not a Character, cinematic skipped"));
		return;
	}

	Controller = Cast<APlayerController>(Player->GetController());
	AnimInstance = Player->GetMesh() ? Player->GetMesh()->GetAnimInstance() : nullptr;

	// For profiles whose montage already sends an impact event on exactly the right frame, ride it rather
	// than racing it from the montage clock. Bound in C++ because the ability graphs cannot host a latent
	// WaitGameplayEvent node and the shared parent abilities are off-limits.
	if (Profile.ImpactEventTag != NAME_None)
	{
		if (UAbilitySystemComponent* ASC = SourceASC.Get())
		{
			const FGameplayTag ImpactTag =
				FGameplayTag::RequestGameplayTag(Profile.ImpactEventTag, /*ErrorIfNotFound*/ false);
			if (ImpactTag.IsValid())
			{
				ImpactEventHandle = ASC->AddGameplayEventTagContainerDelegate(
					FGameplayTagContainer(ImpactTag),
					FGameplayEventTagMulticastDelegate::FDelegate::CreateUObject(
						this, &UNexusFinisherDirector::HandleImpactEvent));
			}
			else
			{
				UE_LOG(LogFinisher, Warning, TEXT("Arm: tag %s not found, finisher disabled"),
					*Profile.ImpactEventTag.ToString());
			}
		}
	}

	// No camera here on purpose. The channel plays on the normal gameplay camera; the cut happens on the
	// camera beat, so an interrupted channel never steals the view.
	if (UWorld* World = Player->GetWorld())
	{
		World->GetTimerManager().SetTimer(PollHandle, this, &UNexusFinisherDirector::PollMontage,
			FinisherCine::PollInterval, /*bLoop*/ true);
	}

	UE_LOG(LogFinisher, Log, TEXT("Armed [%s] on %s, target %s"),
		*Profile.MontageToken, *Player->GetName(), IsValid(InTarget) ? *InTarget->GetName() : TEXT("none"));
}

void UNexusFinisherDirector::Shutdown()
{
	if (bShutDown)
	{
		return;
	}
	bShutDown = true;

	UWorld* World = Avatar.IsValid() ? Avatar->GetWorld() : nullptr;

	if (World)
	{
		World->GetTimerManager().ClearTimer(PollHandle);
	}

	// Unconditional: EndAbility can fire mid-flight, before any beat ran, and the world must never be
	// left in slow motion.
	RestoreDilation();

	if (UAbilitySystemComponent* ASC = SourceASC.Get())
	{
		if (ImpactEventHandle.IsValid())
		{
			const FGameplayTag ImpactTag =
				FGameplayTag::RequestGameplayTag(Profile.ImpactEventTag, /*ErrorIfNotFound*/ false);
			if (ImpactTag.IsValid())
			{
				ASC->RemoveGameplayEventTagContainerDelegate(FGameplayTagContainer(ImpactTag), ImpactEventHandle);
			}
			ImpactEventHandle.Reset();
		}
	}

	// The corpse-throw ticker holds a raw UObject delegate on this director, which is about to leave the
	// root set. Drop it before that happens.
	if (CorpseTickHandle.IsValid())
	{
		FTSTicker::RemoveTicker(CorpseTickHandle);
		CorpseTickHandle.Reset();
	}

	// Only hand the view back if the camera beat actually took it. A channel cancelled before the cut
	// never called SetViewTarget, and "restoring" one we never set would yank the normal gameplay camera
	// onto a stale target and fight the player's own camera for the length of the blend.
	if (bCameraTaken)
	{
		if (APlayerController* PC = Controller.Get())
		{
			AActor* Back = PreviousViewTarget.IsValid() ? PreviousViewTarget.Get() : Cast<AActor>(Avatar.Get());
			if (Back)
			{
				PC->SetViewTargetWithBlend(Back, FinisherCine::CameraBlendOut, EViewTargetBlendFunction::VTBlend_Cubic);
			}
		}
	}

	DestroyCinematicCamera();

	if (IsRooted())
	{
		RemoveFromRoot();
	}

	UE_LOG(LogFinisher, Log, TEXT("Restored [%s] (camera=%d, relocated=%d, impact=%d, corpseThrown=%d)"),
		*Profile.MontageToken, bCameraTaken ? 1 : 0, bRelocated ? 1 : 0, bImpactFired ? 1 : 0, bCorpseThrown ? 1 : 0);
}

void UNexusFinisherDirector::DestroyCinematicCamera()
{
	if (CameraTickHandle.IsValid())
	{
		FTSTicker::RemoveTicker(CameraTickHandle);
		CameraTickHandle.Reset();
	}

	if (ACameraActor* Cam = CinematicCamera)
	{
		// Outlive the blend back to the pawn, then self-destruct. A lifespan rather than a timer, so world
		// teardown cleans it up even if this director is already gone.
		Cam->SetLifeSpan(FinisherCine::CameraBlendOut + 0.25f);
		CinematicCamera = nullptr;
	}
}

// ---------------------------------------------------------------------------------------------
// Montage clock
// ---------------------------------------------------------------------------------------------

void UNexusFinisherDirector::PollMontage()
{
	UAnimInstance* AnimInst = AnimInstance.Get();
	if (!AnimInst)
	{
		return;
	}

	if (!bMontageStarted)
	{
		// Parent::ActivateAbility starts the montage on the frame after Arm, so watch for it.
		UAnimMontage* Active = AnimInst->GetCurrentActiveMontage();
		if (Active && Active->GetName().Contains(Profile.MontageToken))
		{
			Montage = Active;
			bMontageStarted = true;
			AnimInst->Montage_SetPlayRate(Active, Profile.MontagePlayRate);
			UE_LOG(LogFinisher, Log, TEXT("Montage %s started, play rate %.2f"),
				*Active->GetName(), Profile.MontagePlayRate);
		}
		return;
	}

	UAnimMontage* Active = Montage.Get();
	if (!Active || !AnimInst->Montage_IsActive(Active))
	{
		// Ended or blended out early (interrupt, death). Never leave the world slowed.
		RestoreDilation();
		if (UWorld* World = Avatar.IsValid() ? Avatar->GetWorld() : nullptr)
		{
			World->GetTimerManager().ClearTimer(PollHandle);
		}
		return;
	}

	const float Position = AnimInst->Montage_GetPosition(Active);

	if (!bCameraCut && Position >= Profile.CameraCutTime)
	{
		TakeCinematicCamera();
	}

	if (!bDilationApplied && Profile.DilationTime >= 0.f && Position >= Profile.DilationTime)
	{
		ApplyDilation();
	}

	if (!bRelocated && Profile.RelocateTime >= 0.f && Position >= Profile.RelocateTime)
	{
		RelocateToTarget();
	}

	if (!bImpactFired && Profile.ImpactTime >= 0.f && Position >= Profile.ImpactTime)
	{
		FireImpact();
	}

	if (!bDilationRestored && Profile.DilationRestoreTime >= 0.f && Position >= Profile.DilationRestoreTime)
	{
		RestoreDilation();
	}
}

void UNexusFinisherDirector::ApplyDilation()
{
	bDilationApplied = true;

	ACharacter* Player = Avatar.Get();
	UWorld* World = Player ? Player->GetWorld() : nullptr;
	if (!World)
	{
		return;
	}

	UGameplayStatics::SetGlobalTimeDilation(World, Profile.GlobalDilation);
	Player->CustomTimeDilation = Profile.AvatarCounterDilation;

	UE_LOG(LogFinisher, Log, TEXT("Dilation: global %.2f, avatar CustomTimeDilation %.2f"),
		Profile.GlobalDilation, Profile.AvatarCounterDilation);
}

void UNexusFinisherDirector::RestoreDilation()
{
	bDilationRestored = true;

	ACharacter* Player = Avatar.Get();
	if (UWorld* World = Player ? Player->GetWorld() : nullptr)
	{
		UGameplayStatics::SetGlobalTimeDilation(World, 1.f);
	}
	if (Player)
	{
		Player->CustomTimeDilation = 1.f;
	}
}

// ---------------------------------------------------------------------------------------------
// Gap-close
// ---------------------------------------------------------------------------------------------

void UNexusFinisherDirector::RelocateToTarget()
{
	bRelocated = true;

	ACharacter* Player = Avatar.Get();
	if (!Player)
	{
		return;
	}

	float Health = 0.f;
	float Shield = 0.f;
	if (!GetTargetVitals(Health, Shield))
	{
		UE_LOG(LogFinisher, Log,
			TEXT("Relocate skipped: target dead or gone at descent, montage will whiff as intended"));
		return;
	}

	AActor* Victim = Target.Get();
	const FVector VictimLocation = Victim->GetActorLocation();

	// Snap to where the target is NOW, not where it was at activation.
	FVector Approach = Player->GetActorLocation() - VictimLocation;
	Approach.Z = 0.f;
	if (!Approach.Normalize())
	{
		Approach = -Player->GetActorForwardVector();
	}

	FVector Destination = VictimLocation + Approach * Profile.RelocateStandoff;

	// Match feet, not origins: the two capsules can have different half-heights.
	const float PlayerHalfHeight = Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	float VictimHalfHeight = 0.f;
	if (const ACharacter* VictimCharacter = Cast<ACharacter>(Victim))
	{
		VictimHalfHeight = VictimCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	}
	Destination.Z = VictimLocation.Z - VictimHalfHeight + PlayerHalfHeight + Profile.RelocateZLift;

	const FVector Before = Player->GetActorLocation();

	Player->SetActorLocation(Destination, /*bSweep*/ false, nullptr, ETeleportType::TeleportPhysics);

	FRotator Facing = (VictimLocation - Destination).Rotation();
	Facing.Pitch = 0.f;
	Facing.Roll = 0.f;
	Player->SetActorRotation(Facing);
	if (AController* PlayerController = Player->GetController())
	{
		PlayerController->SetControlRotation(Facing);
	}

	UE_LOG(LogFinisher, Log, TEXT("Relocated onto %s at %s (moved %.0f)"),
		*Victim->GetName(), *Destination.ToCompactString(), FVector::Dist(Before, Player->GetActorLocation()));
}

// ---------------------------------------------------------------------------------------------
// Impact
// ---------------------------------------------------------------------------------------------

void UNexusFinisherDirector::HandleImpactEvent(FGameplayTag MatchingTag, const FGameplayEventData* Payload)
{
	FireImpact();
}

void UNexusFinisherDirector::FireImpact()
{
	if (bImpactFired)
	{
		return;
	}
	bImpactFired = true;

	ApplyFinisher();

	if (Profile.bRadialPush)
	{
		ApplyForcePush();
	}

	// The victim is dead but not yet ragdolled: the death pipeline enables physics from Blueprint a frame
	// or two from now. Capture the throw here, while the caster is still around to define "away", and let
	// the ticker spend it the moment the body actually becomes physical.
	if (Profile.bLaunchVictim)
	{
		ACharacter* Player = Avatar.Get();
		AActor* Victim = Target.Get();
		if (Player && IsValid(Victim))
		{
			FVector Away = Victim->GetActorLocation() - Player->GetActorLocation();
			Away.Z = 0.f;
			if (!Away.Normalize())
			{
				Away = Player->GetActorForwardVector().GetSafeNormal2D();
			}

			CorpseThrowVelocity = Away * Profile.VictimLaunchSpeed
				+ FVector(0.f, 0.f, Profile.VictimLaunchUpSpeed);

			// Tumble over the side axis, so the body rotates the way it is being blown rather than spinning
			// around its own spine.
			CorpseTumbleAxis = FVector::CrossProduct(FVector::UpVector, Away).GetSafeNormal();

			CorpseThrowElapsed = 0.f;
			CorpseTickHandle = FTSTicker::GetCoreTicker().AddTicker(
				FTickerDelegate::CreateUObject(this, &UNexusFinisherDirector::TickCorpseThrow));
		}
	}

	if (APlayerController* PC = Controller.Get())
	{
		if (UClass* ShakeClass = FinisherCine::ShakeClassPath.TryLoadClass<UCameraShakeBase>())
		{
			PC->ClientStartCameraShake(ShakeClass, FinisherCine::ShakeScale);
		}
		else
		{
			UE_LOG(LogFinisher, Warning, TEXT("Could not load camera shake %s"),
				*FinisherCine::ShakeClassPath.ToString());
		}
	}

	// Profiles that snap back on a beat also snap back on impact, in case the blow lands before the clock
	// gets there. Profiles that hold their slow motion (DilationRestoreTime < 0) keep it — the slow tumble
	// is the shot, and restoring here would cut it dead.
	if (Profile.DilationRestoreTime >= 0.f)
	{
		RestoreDilation();
	}
}

void UNexusFinisherDirector::ApplyFinisher()
{
	UGameplayAbility* OwningAbility = Ability.Get();
	UAbilitySystemComponent* Source = SourceASC.Get();
	if (!OwningAbility || !Source)
	{
		UE_LOG(LogFinisher, Warning, TEXT("Finisher skipped: no ability or source ASC"));
		return;
	}

	float Health = 0.f;
	float Shield = 0.f;
	if (!GetTargetVitals(Health, Shield))
	{
		UE_LOG(LogFinisher, Log, TEXT("Finisher skipped: target already dead or gone at impact"));
		return;
	}

	AActor* Victim = Target.Get();
	const IAbilitySystemInterface* VictimASI = Cast<IAbilitySystemInterface>(Victim);
	UAbilitySystemComponent* VictimASC = VictimASI ? VictimASI->GetAbilitySystemComponent() : nullptr;
	if (!VictimASC)
	{
		UE_LOG(LogFinisher, Warning, TEXT("Finisher skipped: %s has no ability system"), *Victim->GetName());
		return;
	}

	const FGameplayTag DamageTag =
		FGameplayTag::RequestGameplayTag(FinisherCine::DamageTagName, /*ErrorIfNotFound*/ false);
	if (!DamageTag.IsValid())
	{
		UE_LOG(LogFinisher, Warning, TEXT("Finisher skipped: tag %s not found"),
			*FinisherCine::DamageTagName.ToString());
		return;
	}

	FGameplayEffectContextHandle Context = Source->MakeEffectContext();
	Context.SetAbility(OwningAbility);
	Context.AddSourceObject(Avatar.Get());

	FGameplayEffectSpecHandle Spec =
		Source->MakeOutgoingSpec(UNexusFinisherEffect::StaticClass(), 1.f, Context);
	if (!Spec.IsValid())
	{
		UE_LOG(LogFinisher, Warning, TEXT("Finisher skipped: could not build effect spec"));
		return;
	}

	// Damage is a meta-attribute that drains Shield before Health, so lethal is Health + Shield + 1.
	const float Lethal = Health + Shield + 1.f;
	Spec.Data->SetSetByCallerMagnitude(DamageTag, Lethal);

	Source->ApplyGameplayEffectSpecToTarget(*Spec.Data, VictimASC);

	UE_LOG(LogFinisher, Log, TEXT("Finisher: %s at health %.1f + shield %.1f, applied %.1f damage"),
		*Victim->GetName(), Health, Shield, Lethal);
}

// ---------------------------------------------------------------------------------------------
// Force push
// ---------------------------------------------------------------------------------------------

void UNexusFinisherDirector::ApplyForcePush()
{
	ACharacter* Player = Avatar.Get();
	UWorld* World = Player ? Player->GetWorld() : nullptr;
	if (!World)
	{
		return;
	}

	const FVector Origin = Player->GetActorLocation();

	// Aim the wave down the line to the victim when there is one: the capsule's yaw can lag the shot by a
	// few degrees, and a 60 degree cone is narrow enough for that to clip enemies off the edge.
	FVector Forward = Player->GetActorForwardVector().GetSafeNormal2D();
	if (const AActor* Victim = Target.Get())
	{
		FVector ToVictim = Victim->GetActorLocation() - Origin;
		ToVictim.Z = 0.f;
		if (ToVictim.Normalize())
		{
			Forward = ToVictim;
		}
	}
	if (Forward.IsNearlyZero())
	{
		return;
	}

	const float CosHalfAngle = FMath::Cos(FMath::DegreesToRadians(Profile.PushConeHalfAngleDeg));
	const UClass* BossClass = FinisherCine::BossClassPath.TryLoadClass<AActor>();

	TArray<AActor*> Characters;
	UGameplayStatics::GetAllActorsOfClass(World, ACharacter::StaticClass(), Characters);

	int32 PushedCount = 0;
	for (AActor* Actor : Characters)
	{
		ACharacter* Other = Cast<ACharacter>(Actor);
		if (!IsValid(Other) || Other == Player || Other == Target.Get())
		{
			continue;
		}

		if (BossClass && Other->IsA(BossClass))
		{
			continue;
		}

		// "Enemy" here means anything with an ASC that is still alive. The caster is already excluded, and
		// this keeps the wave independent of which Blueprint hierarchy an enemy happens to come from.
		const IAbilitySystemInterface* OtherASI = Cast<IAbilitySystemInterface>(Other);
		const UAbilitySystemComponent* OtherASC = OtherASI ? OtherASI->GetAbilitySystemComponent() : nullptr;
		const UBasicAttributeSet* Attributes = OtherASC ? OtherASC->GetSet<UBasicAttributeSet>() : nullptr;
		if (!Attributes || Attributes->GetHealth() <= 0.f)
		{
			continue;
		}

		// A corpse mid-ragdoll ignores LaunchCharacter: CMC no longer owns the body.
		if (const USkeletalMeshComponent* OtherMesh = Other->GetMesh())
		{
			if (OtherMesh->IsSimulatingPhysics())
			{
				continue;
			}
		}

		FVector ToOther = Other->GetActorLocation() - Origin;
		ToOther.Z = 0.f;
		if (ToOther.Size() > Profile.PushRadius)
		{
			continue;
		}

		FVector PushDir = ToOther;
		if (!PushDir.Normalize())
		{
			PushDir = Forward;  // standing inside the caster: shove it straight downrange
		}
		else if (FVector::DotProduct(Forward, PushDir) < CosHalfAngle)
		{
			continue;  // outside the cone
		}

		Other->LaunchCharacter(
			PushDir * Profile.PushBackSpeed + FVector(0.f, 0.f, Profile.PushUpSpeed),
			/*bXYOverride*/ true, /*bZOverride*/ true);
		++PushedCount;
	}

	UE_LOG(LogFinisher, Log, TEXT("Force push: %d enemies shoved (radius %.0f, cone %.0f deg)"),
		PushedCount, Profile.PushRadius, Profile.PushConeHalfAngleDeg);
}

bool UNexusFinisherDirector::TickCorpseThrow(float DeltaSeconds)
{
	if (bCorpseThrown)
	{
		return false;
	}

	CorpseThrowElapsed += DeltaSeconds;

	ACharacter* Victim = Cast<ACharacter>(Target.Get());
	USkeletalMeshComponent* Mesh = IsValid(Victim) ? Victim->GetMesh() : nullptr;
	if (!Mesh)
	{
		UE_LOG(LogFinisher, Log, TEXT("Corpse throw abandoned: victim gone before it ragdolled"));
		return false;
	}

	// The death pipeline ragdolls from Blueprint a frame or two after the kill lands. Waiting for the flag
	// rather than guessing a delay means we cannot race it.
	if (Mesh->IsSimulatingPhysics())
	{
		ThrowCorpse(Mesh);
		return false;
	}

	if (CorpseThrowElapsed >= Profile.CorpseThrowWindow)
	{
		UE_LOG(LogFinisher, Warning,
			TEXT("Corpse throw: %s never simulated physics within %.2fs, forcing ragdoll"),
			*Victim->GetName(), Profile.CorpseThrowWindow);
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Mesh->SetSimulatePhysics(true);
		ThrowCorpse(Mesh);
		return false;
	}

	return true;
}

void UNexusFinisherDirector::ThrowCorpse(USkeletalMeshComponent* Mesh)
{
	bCorpseThrown = true;

	// The ticker unregisters itself by returning false, so the handle is already spent — clear it so
	// Shutdown does not try to remove it again.
	CorpseTickHandle.Reset();

	// Velocity, not impulse: an impulse is mass-scaled and only lands on the one body it is applied to,
	// which is what makes ragdoll impulses so fiddly to tune. Setting velocity across every body moves the
	// whole corpse as one, mass-independently, so the numbers in the profile mean what they say.
	Mesh->SetAllPhysicsLinearVelocity(CorpseThrowVelocity);
	if (!CorpseTumbleAxis.IsNearlyZero())
	{
		Mesh->SetAllPhysicsAngularVelocityInDegrees(CorpseTumbleAxis * Profile.VictimTumbleDegrees);
	}

	UE_LOG(LogFinisher, Log, TEXT("Corpse thrown: %s at %.0f cm/s, %.2fs after impact"),
		*GetNameSafe(Mesh->GetOwner()), CorpseThrowVelocity.Size(), CorpseThrowElapsed);
}

bool UNexusFinisherDirector::GetTargetVitals(float& OutHealth, float& OutShield) const
{
	OutHealth = 0.f;
	OutShield = 0.f;

	AActor* Victim = Target.Get();
	if (!IsValid(Victim))
	{
		return false;
	}

	const IAbilitySystemInterface* VictimASI = Cast<IAbilitySystemInterface>(Victim);
	const UAbilitySystemComponent* VictimASC = VictimASI ? VictimASI->GetAbilitySystemComponent() : nullptr;
	if (!VictimASC)
	{
		return false;
	}

	const UBasicAttributeSet* Attributes = VictimASC->GetSet<UBasicAttributeSet>();
	if (!Attributes)
	{
		return false;
	}

	OutHealth = Attributes->GetHealth();
	OutShield = Attributes->GetShield();
	return OutHealth > 0.f;
}

// ---------------------------------------------------------------------------------------------
// Camera
// ---------------------------------------------------------------------------------------------

FVector UNexusFinisherDirector::GetVictimFocus() const
{
	const AActor* Victim = Target.Get();
	if (!IsValid(Victim))
	{
		return FVector::ZeroVector;
	}

	if (const ACharacter* VictimCharacter = Cast<ACharacter>(Victim))
	{
		if (const USkeletalMeshComponent* Mesh = VictimCharacter->GetMesh())
		{
			// Once the body is physical the capsule stays behind at the death spot while the mesh flies, so
			// the actor's location would aim the shot at an empty patch of floor for the whole throw. The
			// mesh bounds follow the ragdoll and need no bone names to do it.
			if (Mesh->IsSimulatingPhysics())
			{
				return Mesh->Bounds.Origin;
			}
		}
	}

	return Victim->GetActorLocation();
}

void UNexusFinisherDirector::TakeCinematicCamera()
{
	// Fires once, at the camera beat, whether or not it manages to get a camera.
	bCameraCut = true;

	APlayerController* PC = Controller.Get();
	ACharacter* Player = Avatar.Get();
	AActor* Victim = Target.Get();
	if (!PC || !Player || !IsValid(Victim))
	{
		UE_LOG(LogFinisher, Log, TEXT("Camera cut skipped: no target at the cut, staying on gameplay camera"));
		return;
	}

	UWorld* World = Player->GetWorld();
	if (!World)
	{
		return;
	}

	const FVector VictimLocation = Victim->GetActorLocation();
	const FVector PlayerLocation = Player->GetActorLocation();

	FVector ToVictim = VictimLocation - PlayerLocation;
	ToVictim.Z = 0.f;
	if (!ToVictim.Normalize())
	{
		ToVictim = Player->GetActorForwardVector().GetSafeNormal2D();
	}
	const FVector Side = FVector::CrossProduct(FVector::UpVector, ToVictim).GetSafeNormal();

	// Captured once, then carried with the anchor every frame. Recomputing the basis per frame would let
	// the shot swing around as the enemy turns; freezing it makes the camera a rigid rig.
	CameraOffset = Side * Profile.CameraSideOffset
		- ToVictim * Profile.CameraBackOffset
		+ FVector(0.f, 0.f, Profile.CameraHeight);

	const FVector Anchor = Profile.bCameraFollowsVictim ? VictimLocation : PlayerLocation;
	const FVector CameraLocation = Anchor + CameraOffset;
	const FVector LookAt = VictimLocation + FVector(0.f, 0.f, Profile.CameraLookAtLift);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.ObjectFlags |= RF_Transient;

	CinematicCamera = World->SpawnActor<ACameraActor>(
		ACameraActor::StaticClass(), CameraLocation, (LookAt - CameraLocation).Rotation(), SpawnParams);
	if (!CinematicCamera)
	{
		UE_LOG(LogFinisher, Warning, TEXT("Could not spawn cinematic camera"));
		return;
	}

	if (UCameraComponent* CameraComponent = CinematicCamera->GetCameraComponent())
	{
		CameraComponent->SetFieldOfView(Profile.CameraFOV);
	}

	PreviousViewTarget = PC->GetViewTarget();
	PC->SetViewTargetWithBlend(CinematicCamera, FinisherCine::CameraBlendIn, EViewTargetBlendFunction::VTBlend_Cubic);
	bCameraTaken = true;

	// Tracked on the core ticker rather than the montage poll timer: the poll runs on dilated world time,
	// so under slow motion it would only land every other frame and the pan would judder exactly during the
	// beat that most needs to be smooth.
	CameraTickHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &UNexusFinisherDirector::TickCamera));

	UE_LOG(LogFinisher, Log, TEXT("Camera cut [%s], tracking %s"), *Profile.MontageToken, *Victim->GetName());
}

bool UNexusFinisherDirector::TickCamera(float DeltaSeconds)
{
	ACameraActor* Cam = CinematicCamera;
	if (!IsValid(Cam))
	{
		return false;  // camera gone: unregister the ticker
	}

	AActor* Victim = Target.Get();
	if (!IsValid(Victim))
	{
		// Target died and despawned mid-shot. Hold the last framing rather than snapping to the world
		// origin, and keep ticking — Shutdown owns the teardown.
		return true;
	}

	// Position rides the actor, which is stable. Aim rides the focus, which follows the ragdoll: a
	// jittering physics body is fine to look at but would shake the whole shot if the rig sat on it.
	if (Profile.bCameraFollowsVictim)
	{
		Cam->SetActorLocation(Victim->GetActorLocation() + CameraOffset);
	}

	FVector Focus = GetVictimFocus() + FVector(0.f, 0.f, Profile.CameraLookAtLift);
	if (const ACharacter* Player = Avatar.Get())
	{
		Focus = FMath::Lerp(Focus, Player->GetActorLocation(), Profile.CameraPlayerBias);
	}

	const FRotator Desired = (Focus - Cam->GetActorLocation()).Rotation();
	Cam->SetActorRotation(
		FMath::RInterpTo(Cam->GetActorRotation(), Desired, DeltaSeconds, Profile.CameraTrackInterpSpeed));

	return true;
}
