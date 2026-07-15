// Fill out your copyright notice in the Description page of Project Settings.

#include "Nexus/UI/NexusSettingsMenu.h"

#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "GenericPlatform/GenericApplication.h" // FDisplayMetrics, for the fullscreen resolution snap
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Nexus/Settings/NexusGameUserSettings.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"

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

TWeakObjectPtr<UNexusSettingsMenu> UNexusSettingsMenu::ActiveInstance;

UNexusSettingsMenu::UNexusSettingsMenu(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// A short existing cue for the volume-release feedback. EditAnywhere, so it can be swapped in the
	// widget's defaults; this just spares a manual wire-up and keeps the cue referenced for cooking.
	static ConstructorHelpers::FObjectFinder<USoundBase> CueFinder(TEXT("/Game/Audio/swoosh_low.swoosh_low"));
	if (CueFinder.Succeeded())
	{
		VolumeFeedbackCue = CueFinder.Object;
	}
}

void UNexusSettingsMenu::NativeConstruct()
{
	Super::NativeConstruct();

	// Register as the on-screen settings menu so the pause toggle can find and close it (see
	// CloseOpenSettingsMenu). Only one exists at a time; last constructed wins.
	ActiveInstance = this;

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
	UpdateVolumeReadout(Settings ? Settings->GetMasterVolume() : 1.0f);

	// Sensitivity: seed value only. Its min/max were set in the designer and "worked before"; leaving
	// them untouched preserves the existing feel rather than silently re-ranging the slider.
	if (SensitivitySlider)
	{
		SensitivitySlider->SetValue(Settings ? Settings->GetMouseSensitivity() : 1.0f);
	}

	PopulateResolutions();
	PopulateWindowModes();

	if (Slider_MasterVolume)
	{
		Slider_MasterVolume->OnValueChanged.AddDynamic(this, &UNexusSettingsMenu::HandleMasterVolumeChanged);
		// Commit-time feedback, not per-tick: play the cue once when the handle is released.
		Slider_MasterVolume->OnMouseCaptureEnd.AddDynamic(this, &UNexusSettingsMenu::HandleVolumeCommitted);
	}

	if (SensitivitySlider)
	{
		SensitivitySlider->OnValueChanged.AddDynamic(this, &UNexusSettingsMenu::HandleSensitivityChanged);
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

	// Clear the registry only if we are still the active one -- guards against a newer menu having
	// already claimed it before this one tears down.
	if (ActiveInstance == this)
	{
		ActiveInstance.Reset();
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

	UpdateVolumeReadout(Value);
}

void UNexusSettingsMenu::HandleSensitivityChanged(float Value)
{
	if (UNexusGameUserSettings* Settings = UNexusGameUserSettings::GetNexusGameUserSettings())
	{
		// Read live by the gameplay look path via GetNexusMouseSensitivity; written to disk in
		// NativeDestruct alongside master volume.
		Settings->SetMouseSensitivity(Value);
	}
}

void UNexusSettingsMenu::HandleVolumeCommitted()
{
	if (VolumeFeedbackCue)
	{
		UGameplayStatics::PlaySound2D(this, VolumeFeedbackCue);
	}
}

void UNexusSettingsMenu::UpdateVolumeReadout(float Value)
{
	if (Text_VolumeValue)
	{
		const int32 Percent = FMath::RoundToInt(FMath::Clamp(Value, 0.0f, 1.0f) * 100.0f);
		Text_VolumeValue->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), Percent)));
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

	// Exclusive fullscreen cannot be driven from a PIE window -- attempting it shrinks the window and
	// then hangs on the OS maximize. In-editor, degrade it to borderless (WindowedFullscreen), which
	// looks identical and is safe. A packaged build (GIsEditor false) still gets true exclusive.
	if (GIsEditor && Mode == EWindowMode::Fullscreen)
	{
		UE_LOG(LogNexusSettings, Log,
			TEXT("HandleWindowModeChanged: in-editor, degrading exclusive Fullscreen to Borderless."));
		Mode = EWindowMode::WindowedFullscreen;
	}

	Settings->SetFullscreenMode(Mode);

	// Snap resolution to the desktop's native size when entering a fullscreen mode, and apply mode +
	// resolution together via a single ApplySettings. Without this, going fullscreen inherits a leftover
	// windowed size and produces a tiny fullscreen at the wrong resolution.
	if (Mode == EWindowMode::Fullscreen || Mode == EWindowMode::WindowedFullscreen)
	{
		FDisplayMetrics DisplayMetrics;
		FDisplayMetrics::RebuildDisplayMetrics(DisplayMetrics);
		const FIntPoint Desktop(DisplayMetrics.PrimaryDisplayWidth, DisplayMetrics.PrimaryDisplayHeight);
		if (Desktop.X > 0 && Desktop.Y > 0)
		{
			Settings->SetScreenResolution(Desktop);

			// Keep the resolution combo showing what we actually applied. SetSelectedOption fires
			// OnSelectionChanged with ESelectInfo::Direct, which HandleResolutionChanged ignores.
			if (Combo_Resolution)
			{
				const FString DesktopOption = NexusSettingsMenu::ResolutionToString(Desktop);
				if (Combo_Resolution->FindOptionIndex(DesktopOption) == INDEX_NONE)
				{
					Combo_Resolution->AddOption(DesktopOption);
				}
				Combo_Resolution->SetSelectedOption(DesktopOption);
			}
		}
	}

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

bool UNexusSettingsMenu::CloseOpenSettingsMenu()
{
	if (UNexusSettingsMenu* Menu = ActiveInstance.Get())
	{
		// Same teardown the Back button uses: removes only the settings menu and leaves the paused
		// pause menu on screen underneath. NativeDestruct clears ActiveInstance.
		Menu->CloseSettingsMenu();
		return true;
	}

	return false;
}
