// Fill out your copyright notice in the Description page of Project Settings.

#include "Nexus/UI/NexusSettingsMenu.h"

#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/Slider.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Nexus/Settings/NexusGameUserSettings.h"

namespace NexusSettingsMenu
{
	static const FString OptionFullscreen = TEXT("Fullscreen");
	static const FString OptionBorderless = TEXT("Borderless");
	static const FString OptionWindowed   = TEXT("Windowed");

	static FString ResolutionToString(const FIntPoint& Resolution)
	{
		return FString::Printf(TEXT("%d x %d"), Resolution.X, Resolution.Y);
	}

	static bool StringToResolution(const FString& Text, FIntPoint& OutResolution)
	{
		FString Left;
		FString Right;
		if (!Text.Split(TEXT("x"), &Left, &Right))
		{
			return false;
		}

		OutResolution.X = FCString::Atoi(*Left.TrimStartAndEnd());
		OutResolution.Y = FCString::Atoi(*Right.TrimStartAndEnd());

		return OutResolution.X > 0 && OutResolution.Y > 0;
	}
}

void UNexusSettingsMenu::NativeConstruct()
{
	Super::NativeConstruct();

	const UNexusGameUserSettings* Settings = UNexusGameUserSettings::GetNexusGameUserSettings();

	// Seed every control BEFORE binding any delegate. Seeding a combo fires OnSelectionChanged
	// (with ESelectInfo::Direct) and seeding the slider fires OnValueChanged, so binding first
	// would make construction look like user input and write settings the player never touched.
	if (Slider_MasterVolume)
	{
		Slider_MasterVolume->SetMinValue(0.0f);
		Slider_MasterVolume->SetMaxValue(1.0f);
		Slider_MasterVolume->SetValue(Settings ? Settings->GetMasterVolume() : 1.0f);
	}

	PopulateResolutions();
	PopulateWindowModes();

	if (Slider_MasterVolume)
	{
		Slider_MasterVolume->OnValueChanged.AddDynamic(this, &UNexusSettingsMenu::HandleMasterVolumeChanged);
	}

	if (Combo_Resolution)
	{
		Combo_Resolution->OnSelectionChanged.AddDynamic(this, &UNexusSettingsMenu::HandleResolutionChanged);
	}

	if (Combo_WindowMode)
	{
		Combo_WindowMode->OnSelectionChanged.AddDynamic(this, &UNexusSettingsMenu::HandleWindowModeChanged);
	}

	if (Button_Back)
	{
		Button_Back->OnClicked.AddDynamic(this, &UNexusSettingsMenu::HandleBackClicked);
	}
}

void UNexusSettingsMenu::NativeDestruct()
{
	// Volume is applied live as the slider moves but only written to disk here, so a drag doesn't
	// rewrite GameUserSettings.ini once per frame. The video handlers persist themselves via
	// ApplySettings, so this is only load-bearing for volume.
	if (UNexusGameUserSettings* Settings = UNexusGameUserSettings::GetNexusGameUserSettings())
	{
		Settings->SaveSettings();
	}

	Super::NativeDestruct();
}

void UNexusSettingsMenu::PopulateResolutions()
{
	if (!Combo_Resolution)
	{
		return;
	}

	Combo_Resolution->ClearOptions();

	// Ask the display rather than hardcoding a list: a hardcoded list offers modes this monitor may
	// not have and misses the ones it does.
	TArray<FIntPoint> Resolutions;
	if (!UKismetSystemLibrary::GetSupportedFullscreenResolutions(Resolutions) || Resolutions.Num() == 0)
	{
		UKismetSystemLibrary::GetConvenientWindowedResolutions(Resolutions);
	}

	for (const FIntPoint& Resolution : Resolutions)
	{
		const FString Option = NexusSettingsMenu::ResolutionToString(Resolution);
		if (Combo_Resolution->FindOptionIndex(Option) == INDEX_NONE)
		{
			Combo_Resolution->AddOption(Option);
		}
	}

	const UNexusGameUserSettings* Settings = UNexusGameUserSettings::GetNexusGameUserSettings();
	if (!Settings)
	{
		UE_LOG(LogNexusSettings, Warning, TEXT("PopulateResolutions: no UNexusGameUserSettings, cannot preselect."));
		return;
	}

	// The current mode is often absent from the supported list (a resized windowed session, PIE).
	// Add it so the combo can show what the player is actually running rather than a wrong entry.
	const FString Current = NexusSettingsMenu::ResolutionToString(Settings->GetScreenResolution());
	if (Combo_Resolution->FindOptionIndex(Current) == INDEX_NONE)
	{
		Combo_Resolution->AddOption(Current);
	}

	Combo_Resolution->SetSelectedOption(Current);
}

void UNexusSettingsMenu::PopulateWindowModes()
{
	if (!Combo_WindowMode)
	{
		return;
	}

	Combo_WindowMode->ClearOptions();
	Combo_WindowMode->AddOption(NexusSettingsMenu::OptionFullscreen);
	Combo_WindowMode->AddOption(NexusSettingsMenu::OptionBorderless);
	Combo_WindowMode->AddOption(NexusSettingsMenu::OptionWindowed);

	const UNexusGameUserSettings* Settings = UNexusGameUserSettings::GetNexusGameUserSettings();
	const EWindowMode::Type Mode = Settings ? Settings->GetFullscreenMode() : EWindowMode::Windowed;

	switch (Mode)
	{
	case EWindowMode::Fullscreen:
		Combo_WindowMode->SetSelectedOption(NexusSettingsMenu::OptionFullscreen);
		break;

	case EWindowMode::WindowedFullscreen:
		Combo_WindowMode->SetSelectedOption(NexusSettingsMenu::OptionBorderless);
		break;

	default:
		Combo_WindowMode->SetSelectedOption(NexusSettingsMenu::OptionWindowed);
		break;
	}
}

void UNexusSettingsMenu::HandleMasterVolumeChanged(float Value)
{
	if (UNexusGameUserSettings* Settings = UNexusGameUserSettings::GetNexusGameUserSettings())
	{
		// Applied to the audio device immediately so the player hears the slider; written to disk
		// in NativeDestruct.
		Settings->SetMasterVolume(Value);
	}
}

void UNexusSettingsMenu::HandleResolutionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	// Direct means SetSelectedOption, i.e. our own seeding rather than the player picking something.
	if (SelectionType == ESelectInfo::Direct)
	{
		return;
	}

	FIntPoint Resolution;
	if (!NexusSettingsMenu::StringToResolution(SelectedItem, Resolution))
	{
		UE_LOG(LogNexusSettings, Warning, TEXT("HandleResolutionChanged: cannot parse '%s'."), *SelectedItem);
		return;
	}

	UNexusGameUserSettings* Settings = UNexusGameUserSettings::GetNexusGameUserSettings();
	if (!Settings)
	{
		return;
	}

	Settings->SetScreenResolution(Resolution);
	Settings->ApplySettings(false);

	// Without this the engine treats the mode as unconfirmed and can revert it on the next boot.
	Settings->ConfirmVideoMode();

	UE_LOG(LogNexusSettings, Log, TEXT("Resolution set to %dx%d."), Resolution.X, Resolution.Y);
}

void UNexusSettingsMenu::HandleWindowModeChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (SelectionType == ESelectInfo::Direct)
	{
		return;
	}

	UNexusGameUserSettings* Settings = UNexusGameUserSettings::GetNexusGameUserSettings();
	if (!Settings)
	{
		return;
	}

	EWindowMode::Type Mode;
	if (SelectedItem == NexusSettingsMenu::OptionFullscreen)
	{
		Mode = EWindowMode::Fullscreen;
	}
	else if (SelectedItem == NexusSettingsMenu::OptionBorderless)
	{
		Mode = EWindowMode::WindowedFullscreen;
	}
	else if (SelectedItem == NexusSettingsMenu::OptionWindowed)
	{
		Mode = EWindowMode::Windowed;
	}
	else
	{
		UE_LOG(LogNexusSettings, Warning, TEXT("HandleWindowModeChanged: unknown option '%s'."), *SelectedItem);
		return;
	}

	Settings->SetFullscreenMode(Mode);
	Settings->ApplySettings(false);
	Settings->ConfirmVideoMode();

	UE_LOG(LogNexusSettings, Log, TEXT("Window mode set to '%s'."), *SelectedItem);
}

void UNexusSettingsMenu::HandleBackClicked()
{
	CloseSettingsMenu();
}

void UNexusSettingsMenu::CloseSettingsMenu()
{
	RemoveFromParent();
}
