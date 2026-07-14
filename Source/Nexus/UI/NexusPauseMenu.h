// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Templates/SubclassOf.h"
#include "NexusPauseMenu.generated.h"

class UButton;
class UNexusSettingsMenu;

/**
 * Native parent for W_DeathScreen, which does double duty as the death screen AND the pause menu:
 * BP_ThirdPersonPlayerController pauses the game BEFORE creating the widget, and the Blueprint's
 * Event Construct reads IsGamePaused to swap the title between "YOU DIED" and "PAUSED".
 *
 * The only thing this parent adds is the Settings button. It is revealed ONLY in the paused state,
 * reusing that same IsGamePaused condition rather than inventing a second one, so it can never
 * appear on the death screen.
 *
 * The Blueprint's existing graph is untouched: NativeConstruct runs before the Blueprint's
 * Event Construct, and this class touches nothing that graph touches.
 */
UCLASS()
class NEXUS_API UNexusPauseMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Creates SettingsMenuClass and adds it above the pause menu. */
	UFUNCTION(BlueprintCallable, Category = "Nexus|Settings")
	void OpenSettingsMenu();

protected:
	virtual void NativeConstruct() override;

	/** W_SettingsMenu. Set on the W_DeathScreen CDO. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Nexus|Settings")
	TSubclassOf<UNexusSettingsMenu> SettingsMenuClass;

	/**
	 * Optional rather than a hard BindWidget so that reparenting W_DeathScreen does not break it
	 * before the button has been added to the widget tree. If the button is absent the widget simply
	 * behaves exactly as it does today.
	 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_Settings;

private:
	UFUNCTION()
	void HandleSettingsClicked();

	UPROPERTY()
	TObjectPtr<UNexusSettingsMenu> SettingsMenu;
};
