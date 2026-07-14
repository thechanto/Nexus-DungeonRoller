// Fill out your copyright notice in the Description page of Project Settings.

#include "Nexus/Settings/NexusGameInstance.h"

#include "Nexus/Settings/NexusGameUserSettings.h"

void UNexusGameInstance::Init()
{
	Super::Init();

	const UNexusGameUserSettings* Settings = UNexusGameUserSettings::GetNexusGameUserSettings();
	if (!Settings)
	{
		UE_LOG(LogNexusSettings, Warning,
			TEXT("Init: engine settings object is not a UNexusGameUserSettings. Check GameUserSettingsClassName "
				 "in DefaultEngine.ini -- master volume will not persist."));
		return;
	}

	Settings->ApplyMasterVolume();
	UE_LOG(LogNexusSettings, Log, TEXT("Init: applied saved master volume %.2f"), Settings->GetMasterVolume());
}

void UNexusGameInstance::OnStart()
{
	Super::OnStart();

	// Re-apply. On some boots the main audio device is not up yet at Init time, and pushing the volume
	// at a null device is a silent no-op -- which would leave the player's saved volume unapplied for
	// the whole session. Idempotent, so paying for it twice costs nothing.
	if (const UNexusGameUserSettings* Settings = UNexusGameUserSettings::GetNexusGameUserSettings())
	{
		Settings->ApplyMasterVolume();
	}
}
