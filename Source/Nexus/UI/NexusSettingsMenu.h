// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/SlateEnums.h" // ESelectInfo, used in the combo-box handler signatures below
#include "NexusSettingsMenu.generated.h"

class UButton;
class UComboBoxString;
class USlider;
class UTextBlock;
class USoundBase;

/**
 * Native parent for W_SettingsMenu.
 *
 * Every control is bound by name and every delegate is bound here in C++, so the widget Blueprint
 * needs no event graph whatsoever -- its widget tree just has to contain children with these exact
 * names. BindWidget is a hard compile error when one is missing, which is the failure mode we want:
 * loud at compile time rather than a silent null at runtime.
 */
UCLASS()
class NEXUS_API UNexusSettingsMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	UNexusSettingsMenu(const FObjectInitializer& ObjectInitializer);

	/** Removes the menu, leaving whatever opened it (the pause menu) on screen underneath. */
	UFUNCTION(BlueprintCallable, Category = "Nexus|Settings")
	void CloseSettingsMenu();

	/**
	 * If a settings menu is currently on screen, close only it and return true; otherwise return false.
	 * Called from the controller's TogglePauseMenu so Esc inside Settings closes just Settings (back to
	 * the still-paused pause menu) instead of routing to the resume path and orphaning the panel over
	 * live gameplay. O(1) via the ActiveInstance registry rather than a viewport widget scan.
	 */
	UFUNCTION(BlueprintCallable, Category = "Nexus|Settings")
	static bool CloseOpenSettingsMenu();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> Slider_MasterVolume;

	/** The existing sensitivity slider, now driven from C++ (its old Blueprint chains are removed). */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> SensitivitySlider;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UComboBoxString> Combo_Resolution;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UComboBoxString> Combo_WindowMode;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Back;

	/** Optional percent readout beside the volume slider, added in the widget pass. Bound if present. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_VolumeValue;

	/**
	 * Played once when the volume slider is released (OnMouseCaptureEnd), so the player hears the level
	 * they just set even in a silent context (paused game / main menu, SFX-only project). Because the
	 * cue plays through the same audio device the master volume scales, it audibly reflects the setting.
	 * Defaulted to a short existing cue in the constructor; swappable in the widget's defaults.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nexus|Settings")
	TObjectPtr<USoundBase> VolumeFeedbackCue;

private:
	UFUNCTION()
	void HandleMasterVolumeChanged(float Value);

	UFUNCTION()
	void HandleSensitivityChanged(float Value);

	/** Plays VolumeFeedbackCue on slider release so the level is testable by ear in any context. */
	UFUNCTION()
	void HandleVolumeCommitted();

	/** Sets Text_VolumeValue to a whole-percent string if the label is bound. */
	void UpdateVolumeReadout(float Value);

	UFUNCTION()
	void HandleResolutionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void HandleWindowModeChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void HandleBackClicked();

	/** Fills Combo_Resolution from the monitor's actual supported modes and selects the current one. */
	void PopulateResolutions();

	/** Fills Combo_WindowMode with the three window modes and selects the current one. */
	void PopulateWindowModes();

	/**
	 * The settings menu currently on screen, if any. Set in NativeConstruct, cleared in NativeDestruct.
	 * Weak so a menu torn down by any path (Back button, RemoveFromParent, GC) never leaves a dangling
	 * pointer that CloseOpenSettingsMenu could act on. Single-instance by construction -- the pause menu
	 * only ever spawns one at a time.
	 */
	static TWeakObjectPtr<UNexusSettingsMenu> ActiveInstance;
};
