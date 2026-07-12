// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "UObject/Object.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"
#include "NexusFinisherDirector.generated.h"

class ACameraActor;
class ACharacter;
class APlayerController;
class UAbilitySystemComponent;
class UAnimInstance;
class UAnimMontage;
class UGameplayAbility;
class USkeletalMeshComponent;
struct FGameplayEventData;

/**
 * A finisher's killing blow. Instant, with a single Override modifier on the Damage meta-attribute
 * whose magnitude is a plain SetByCaller (Data.Damage).
 *
 * Deliberately NOT the MMC that GE_Damage_Instant uses: that one treats the SetByCaller value as an
 * input to a Strength/Armor formula, so an armored target could survive a magnitude that was computed
 * to be lethal. Here the caller's number lands verbatim. It still writes the Damage meta-attribute, so
 * the kill flows through the normal death pipeline (ragdoll/loot/XP/token release) rather than
 * replacing it.
 *
 * Carries no Effects.HitReaction tag: the killing blow must not stagger the victim out of its death.
 */
UCLASS()
class NEXUS_API UNexusFinisherEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UNexusFinisherEffect();
};

/**
 * Everything that differs between one finisher and the next. The director itself is generic; a profile
 * is the only thing that makes it Sky Crusher rather than Burden.
 *
 * Every beat is expressed in the montage's OWN clock, never in seconds. Finishers run at non-unit play
 * rates under global time dilation, so any beat expressed in wall-clock time would desync the moment
 * either is tuned.
 */
struct FNexusFinisherProfile
{
	// -- identity ------------------------------------------------------------------------------
	/** Substring the active montage's name must contain before the director adopts it. */
	FString MontageToken;
	float MontagePlayRate = 1.f;

	// -- beats, in montage time ----------------------------------------------------------------
	/** The cut. Nothing before this touches the view target, so a cancelled channel keeps its camera. */
	float CameraCutTime = 0.f;
	/** < 0: this finisher never slows time. */
	float DilationTime = -1.f;
	/** < 0: hold the slow motion until the ability ends, rather than snapping back on a beat. */
	float DilationRestoreTime = -1.f;
	/** < 0: no gap-close; the caster kills from where it stands. */
	float RelocateTime = -1.f;
	/** < 0: impact is driven by ImpactEventTag instead of the montage clock. */
	float ImpactTime = -1.f;

	/**
	 * Impact event, for finishers whose montage already sends one at exactly the right frame.
	 * NAME_None: fall back to ImpactTime. Whichever arrives first fires the impact, once.
	 */
	FName ImpactEventTag = NAME_None;

	// -- time dilation -------------------------------------------------------------------------
	float GlobalDilation = 0.4f;
	/**
	 * 1.0 leaves the avatar in slow motion along with the world — the montage itself stretches, so the
	 * recovery plays out over GlobalDilation-scaled real seconds. Above 1.0 it counters the global
	 * dilation and the avatar keeps animating at full speed while the world crawls.
	 */
	float AvatarCounterDilation = 1.f;

	// -- gap-close -----------------------------------------------------------------------------
	/**
	 * Horizontal clearance from the victim. Landing exactly on its origin puts two character capsules
	 * inside each other, and CMC depenetration then shoves the caster sideways for the whole descent.
	 */
	float RelocateStandoff = 90.f;
	float RelocateZLift = 10.f;

	// -- camera --------------------------------------------------------------------------------
	/**
	 * true: the rig rides the victim, holding it pinned in the same part of frame however it moves.
	 * false: the rig stays anchored on the caster and pans to follow the victim — the shot for a
	 * finisher that throws the body away from a caster who never moves.
	 */
	bool bCameraFollowsVictim = true;
	float CameraSideOffset = 520.f;
	float CameraBackOffset = 180.f;
	float CameraHeight = 260.f;
	float CameraLookAtLift = 140.f;
	float CameraFOV = 80.f;
	/** How far the aim drags off the victim toward the caster. 0 = victim dead centre. */
	float CameraPlayerBias = 0.18f;
	/** Aim smoothing, in real seconds — undilated, so the pan reads the same in slow motion. */
	float CameraTrackInterpSpeed = 7.f;

	// -- force push ----------------------------------------------------------------------------
	/** Throw the corpse once the death pipeline ragdolls it. Cosmetic; the death itself is untouched. */
	bool bLaunchVictim = false;
	float VictimLaunchSpeed = 1800.f;
	float VictimLaunchUpSpeed = 700.f;
	float VictimTumbleDegrees = 180.f;
	/** How long to wait for the death pipeline to ragdoll the body before forcing physics on ourselves. */
	float CorpseThrowWindow = 0.5f;

	/** Shove every other living non-boss enemy in a cone. No damage, no kill — the wave. */
	bool bRadialPush = false;
	float PushRadius = 700.f;
	float PushConeHalfAngleDeg = 60.f;
	float PushBackSpeed = 1200.f;
	float PushUpSpeed = 500.f;

	static const FNexusFinisherProfile& SkyCrusher();
	static const FNexusFinisherProfile& Burden();
};

/**
 * Drives everything a cinematic finisher does that a Blueprint ability graph cannot: the camera cut and
 * its tracking, the time-dilation beats, the optional gap-close teleport, the guaranteed-lethal blow,
 * and the physics of the force push.
 *
 * The camera is NOT taken at activation. The whole channel plays on the normal gameplay camera and the
 * cut happens on the camera beat, so a channel cancelled before that beat never touches the view target,
 * and Shutdown only ever restores a view target it actually took.
 */
UCLASS()
class NEXUS_API UNexusFinisherDirector : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Arms the director for one activation. Must run BEFORE Parent::ActivateAbility (which is what
	 * starts the montage) so the play rate is applied on the montage's first frame.
	 * Target is the validated victim from CanSkyCrusherExecute.
	 */
	static void Arm(UGameplayAbility* Ability, AActor* Target, const FNexusFinisherProfile& Profile);

	/**
	 * Unwinds the cinematic for Ability: restores both dilations unconditionally, restores the view
	 * target ONLY if the camera beat actually took it, retires the spawned camera, drops the event
	 * binding. Safe on every exit path (end/cancel/death/interrupt), safe before any beat has run, and
	 * safe to call twice.
	 */
	static void Restore(UGameplayAbility* Ability);

private:
	void Begin(UGameplayAbility* InAbility, AActor* InTarget, const FNexusFinisherProfile& InProfile);
	void Shutdown();

	/** Timer-driven: watches the montage clock and fires each position beat exactly once. */
	void PollMontage();

	/** Camera beat: spawns the camera and cuts to it. Nothing before this touches the view target. */
	void TakeCinematicCamera();

	/** Core-ticker driven, once per frame while the shot is live: keeps the victim framed as it moves. */
	bool TickCamera(float DeltaSeconds);

	/** Core-ticker driven: waits for the death pipeline to ragdoll the victim, then throws the corpse. */
	bool TickCorpseThrow(float DeltaSeconds);

	void ApplyDilation();
	void RestoreDilation();
	void RelocateToTarget();
	void DestroyCinematicCamera();

	/** Bound to the avatar ASC's ImpactEventTag, for profiles that drive impact off an event. */
	void HandleImpactEvent(FGameplayTag MatchingTag, const FGameplayEventData* Payload);

	/** The killing blow and everything that rides on it. Runs once, from whichever beat gets there first. */
	void FireImpact();
	void ApplyFinisher();
	void ApplyForcePush();
	void ThrowCorpse(USkeletalMeshComponent* Mesh);

	/** False when the target is gone or already dead; fills current Health/Shield when true. */
	bool GetTargetVitals(float& OutHealth, float& OutShield) const;

	/**
	 * Where to point the camera. A ragdolled victim leaves its capsule behind at the death spot while the
	 * mesh flies, so GetActorLocation would aim the shot at an empty patch of floor for the whole throw.
	 * Once physics owns the body, follow the mesh's bounds instead.
	 */
	FVector GetVictimFocus() const;

	FNexusFinisherProfile Profile;

	TWeakObjectPtr<UGameplayAbility> Ability;
	TWeakObjectPtr<AActor> Target;
	TWeakObjectPtr<ACharacter> Avatar;
	TWeakObjectPtr<APlayerController> Controller;
	TWeakObjectPtr<UAbilitySystemComponent> SourceASC;
	TWeakObjectPtr<UAnimInstance> AnimInstance;
	TWeakObjectPtr<UAnimMontage> Montage;
	TWeakObjectPtr<AActor> PreviousViewTarget;

	UPROPERTY()
	TObjectPtr<ACameraActor> CinematicCamera;

	/**
	 * The camera's world offset from its anchor (victim or caster, per the profile), captured once at the
	 * cut. Tracking re-anchors this offset every frame instead of recomputing the angle, so a strafing
	 * enemy stays pinned in the same part of the frame and the shot never orbits or swims.
	 */
	FVector CameraOffset = FVector::ZeroVector;

	/** Captured at impact, spent when the body finally ragdolls — the caster may be dead by then. */
	FVector CorpseThrowVelocity = FVector::ZeroVector;
	FVector CorpseTumbleAxis = FVector::ZeroVector;
	float CorpseThrowElapsed = 0.f;

	FTimerHandle PollHandle;
	FDelegateHandle ImpactEventHandle;
	FTSTicker::FDelegateHandle CameraTickHandle;
	FTSTicker::FDelegateHandle CorpseTickHandle;

	bool bMontageStarted = false;
	bool bCameraCut = false;      // the camera beat has fired (whether or not it got a camera)
	bool bCameraTaken = false;    // we actually called SetViewTarget, so we owe a restore
	bool bDilationApplied = false;
	bool bDilationRestored = false;
	bool bRelocated = false;
	bool bImpactFired = false;
	bool bCorpseThrown = false;
	bool bShutDown = false;
};
