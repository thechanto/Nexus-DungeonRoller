// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "NexusGameUserSettings.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogNexusSettings, Log, All);

/**
 * The project's settings object, registered through GameUserSettingsClassName in DefaultEngine.ini.
 *
 * Video (resolution + window mode) is inherited wholesale: UGameUserSettings already persists it to
 * Saved/Config/<Platform>/GameUserSettings.ini, so there is nothing to write for it. All this class
 * adds is master volume.
 *
 * Volume is deliberately NOT kept in BP_SaveGame. That save is wiped routinely during testing, and a
 * wipe would silently reset the player's audio. A UPROPERTY(config) lands in GameUserSettings.ini,
 * which lives under Saved/Config and survives a save-game wipe.
 */
UCLASS(config = GameUserSettings, configdonotcheckdefaults)
class NEXUS_API UNexusGameUserSettings : public UGameUserSettings
{
	GENERATED_BODY()

public:
	UNexusGameUserSettings();

	/** Typed accessor for the engine's settings singleton. Null if GameUserSettingsClassName is wrong. */
	UFUNCTION(BlueprintPure, Category = "Nexus|Settings")
	static UNexusGameUserSettings* GetNexusGameUserSettings();

	/** Clamped to 0..1 and applied to the audio device immediately. Persisted by SaveSettings. */
	UFUNCTION(BlueprintCallable, Category = "Nexus|Settings")
	void SetMasterVolume(float InMasterVolume);

	UFUNCTION(BlueprintPure, Category = "Nexus|Settings")
	float GetMasterVolume() const { return MasterVolume; }

	/**
	 * Mouse look sensitivity, persisted to GameUserSettings.ini alongside master volume.
	 *
	 * This used to live only as an in-memory variable on BP_MyGameInstance, so it never survived a
	 * relaunch (it merely persisted across level loads within one session). It now has a real home.
	 */
	UFUNCTION(BlueprintCallable, Category = "Nexus|Settings")
	void SetMouseSensitivity(float InMouseSensitivity);

	UFUNCTION(BlueprintPure, Category = "Nexus|Settings")
	float GetMouseSensitivity() const { return MouseSensitivity; }

	/**
	 * Static convenience for the gameplay look/turn path (BP_NexusPlayer scales yaw + pitch by this).
	 * Resolves the settings singleton and returns 1.0 if it is somehow unavailable, so the input path
	 * degrades to unscaled rather than to zero (an unreadable, look-is-dead value).
	 */
	UFUNCTION(BlueprintPure, Category = "Nexus|Settings")
	static float GetNexusMouseSensitivity();

	/**
	 * Pushes MasterVolume into the audio device as its transient primary volume.
	 *
	 * The engine does not persist that value, so it has to be re-pushed on every launch --
	 * UNexusGameInstance::Init does exactly that. Safe to call from the game thread:
	 * FAudioDevice::SetTransientPrimaryVolume marshals itself onto the audio thread.
	 *
	 * This is a device-wide multiplier, which is why the project needs no SoundClass/SoundMix graph
	 * (it has neither) and why no cue can be missed by it.
	 */
	void ApplyMasterVolume() const;

	virtual void SetToDefaults() override;
	virtual void ApplyNonResolutionSettings() override;

protected:
	/** Written to [/Script/Nexus.NexusGameUserSettings] in GameUserSettings.ini. */
	UPROPERTY(config)
	float MasterVolume;

	/** Written to the same section. Read live by the gameplay look path via GetNexusMouseSensitivity. */
	UPROPERTY(config)
	float MouseSensitivity;
};
