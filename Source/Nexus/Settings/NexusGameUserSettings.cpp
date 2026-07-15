// Fill out your copyright notice in the Description page of Project Settings.

#include "Nexus/Settings/NexusGameUserSettings.h"

#include "AudioDevice.h"
#include "AudioDeviceManager.h"
#include "Engine/Engine.h"

DEFINE_LOG_CATEGORY(LogNexusSettings);

UNexusGameUserSettings::UNexusGameUserSettings()
{
	MasterVolume = 1.0f;
	MouseSensitivity = 1.0f;
}

UNexusGameUserSettings* UNexusGameUserSettings::GetNexusGameUserSettings()
{
	return Cast<UNexusGameUserSettings>(UGameUserSettings::GetGameUserSettings());
}

void UNexusGameUserSettings::SetMasterVolume(float InMasterVolume)
{
	MasterVolume = FMath::Clamp(InMasterVolume, 0.0f, 1.0f);
	ApplyMasterVolume();
}

void UNexusGameUserSettings::ApplyMasterVolume() const
{
	if (!GEngine)
	{
		return;
	}

	const float Volume = FMath::Clamp(MasterVolume, 0.0f, 1.0f);

	// Push to EVERY live audio device, not just the editor's main one. In-editor,
	// bCreateNewAudioDeviceForPlayInEditor spins up a separate device for PIE, so targeting only the
	// main device left PIE audio untouched -- the slider "did nothing" in-editor. In a packaged build
	// there is a single shared device and this iterates over just that one, so it is correct in both.
	// GEngine is not const here, so this selects the non-const overload and yields a mutable device.
	if (FAudioDeviceManager* DeviceManager = GEngine->GetAudioDeviceManager())
	{
		DeviceManager->IterateOverAllDevices(
			[Volume](Audio::FDeviceId /*DeviceId*/, FAudioDevice* Device)
			{
				if (Device)
				{
					Device->SetTransientPrimaryVolume(Volume);
				}
			});
		return;
	}

	// No device manager (commandlets, -nosound): fall back to the main device if there is one.
	if (FAudioDevice* AudioDevice = GEngine->GetMainAudioDeviceRaw())
	{
		AudioDevice->SetTransientPrimaryVolume(Volume);
	}
}

void UNexusGameUserSettings::SetMouseSensitivity(float InMouseSensitivity)
{
	// Floor above zero so the look input can never be scaled to a dead stop; the slider's own min/max
	// (set in the designer) is what actually bounds the UI.
	MouseSensitivity = FMath::Clamp(InMouseSensitivity, 0.05f, 10.0f);
}

float UNexusGameUserSettings::GetNexusMouseSensitivity()
{
	if (const UNexusGameUserSettings* Settings = GetNexusGameUserSettings())
	{
		return Settings->GetMouseSensitivity();
	}

	return 1.0f;
}

void UNexusGameUserSettings::SetToDefaults()
{
	Super::SetToDefaults();

	MasterVolume = 1.0f;
	MouseSensitivity = 1.0f;
}

void UNexusGameUserSettings::ApplyNonResolutionSettings()
{
	Super::ApplyNonResolutionSettings();

	ApplyMasterVolume();
}
