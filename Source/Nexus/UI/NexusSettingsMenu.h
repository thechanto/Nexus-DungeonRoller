// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/SlateEnums.h" // ESelectInfo, used in the combo-box handler signatures below
#include "NexusSettingsMenu.generated.h"

class UButton;
class UComboBoxString;
class USlider;

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
	/** Removes the menu, leaving whatever opened it (the pause menu) on screen underneath. */
	UFUNCTION(BlueprintCallable, Category = "Nexus|Settings")
	void CloseSettingsMenu();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> Slider_MasterVolume;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UComboBoxString> Combo_Resolution;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UComboBoxString> Combo_WindowMode;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Back;

private:
	UFUNCTION()
	void HandleMasterVolumeChanged(float Value);

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
};
