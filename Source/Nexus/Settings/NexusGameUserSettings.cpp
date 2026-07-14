// Fill out your copyright notice in the Description page of Project Settings.

#include "Nexus/Settings/NexusGameUserSettings.h"

#include "AudioDevice.h"
#include "Engine/Engine.h"

DEFINE_LOG_CATEGORY(LogNexusSettings);

UNexusGameUserSettings::UNexusGameUserSettings()
{
	MasterVolume = 1.0f;
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

	FAudioDevice* AudioDevice = GEngine->GetMainAudioDeviceRaw();
	if (!AudioDevice)
	{
		// Boots with no audio device (commandlets, -nosound) land here. Not an error.
		UE_LOG(LogNexusSettings, Verbose, TEXT("ApplyMasterVolume: no main audio device, skipping."));
		return;
	}

	AudioDevice->SetTransientPrimaryVolume(FMath::Clamp(MasterVolume, 0.0f, 1.0f));
}

void UNexusGameUserSettings::SetToDefaults()
{
	Super::SetToDefaults();

	MasterVolume = 1.0f;
}

void UNexusGameUserSettings::ApplyNonResolutionSettings()
{
	Super::ApplyNonResolutionSettings();

	ApplyMasterVolume();
}
