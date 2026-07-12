// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NexusLockOnComponent.generated.h"

class ACharacter;
class APlayerController;
class UInputAction;
class UUserWidget;
class UWidgetComponent;
class ANexusEnemyBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLockOnChanged, bool, bLocked, AActor*, Target);

/**
 * Souls-style lock-on targeting. Added to the player character; drives control
 * rotation toward the locked enemy each tick, switches the character to strafe
 * movement, and attaches a screen-space indicator widget to the target.
 * Binds its own toggle input (LockOnAction) on the owner's EnhancedInputComponent.
 */
UCLASS(ClassGroup = (Nexus), meta = (BlueprintSpawnableComponent))
class NEXUS_API UNexusLockOnComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNexusLockOnComponent();

	/** Toggle input (e.g. Tab / middle mouse). Bound automatically once the owner has input. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn|Input")
	TObjectPtr<UInputAction> LockOnAction;

	/** Max distance to acquire a target. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn")
	float LockOnRange = 2000.f;

	/** Distance at which an existing lock breaks. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn")
	float BreakDistance = 2500.f;

	/** Interp speed for the camera rotation while locked. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn")
	float CameraInterpSpeed = 9.f;

	/** Extra downward pitch (degrees) applied to the framed view. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn")
	float ExtraPitchDown = 10.f;

	/** Bone/socket on the target mesh the indicator attaches to and the camera frames. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn")
	FName TargetSocketName = TEXT("head");

	/** Widget shown on the locked target (screen space). Optional. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn|Indicator")
	TSubclassOf<UUserWidget> IndicatorWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LockOn|Indicator")
	FVector2D IndicatorDrawSize = FVector2D(70.f, 70.f);

	/** Fires on lock and on break (Target is null on break). */
	UPROPERTY(BlueprintAssignable, Category = "LockOn")
	FOnLockOnChanged OnLockOnChanged;

	UFUNCTION(BlueprintCallable, Category = "LockOn")
	void ToggleLockOn();

	UFUNCTION(BlueprintCallable, Category = "LockOn")
	void BreakLock();

	UFUNCTION(BlueprintPure, Category = "LockOn")
	bool IsLocked() const { return LockedTarget != nullptr; }

	UFUNCTION(BlueprintPure, Category = "LockOn")
	AActor* GetLockedTarget() const { return LockedTarget; }

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY()
	TObjectPtr<AActor> LockedTarget;

	UPROPERTY()
	TObjectPtr<UWidgetComponent> IndicatorComponent;

	UPROPERTY()
	TObjectPtr<ACharacter> OwnerCharacter;

	bool bInputBound = false;
	bool bSavedOrientRotationToMovement = true;
	bool bSavedUseControllerDesiredRotation = false;

	void TryBindInput();
	AActor* FindBestTarget() const;
	void LockOn(AActor* Target);

	/** Locked target died: switch to the best remaining enemy, or clear the lock if there is none. */
	void HandleTargetDeath(float DeltaTime);

	bool IsTargetStillValid() const;
	void SteerCamera(float DeltaTime);
	FVector GetTargetFramePoint(const AActor* Target) const;
	APlayerController* GetPlayerController() const;
};
