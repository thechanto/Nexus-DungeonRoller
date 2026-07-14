// Fill out your copyright notice in the Description page of Project Settings.

#include "Nexus/UI/NexusPauseMenu.h"

#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Nexus/Settings/NexusGameUserSettings.h"
#include "Nexus/UI/NexusSettingsMenu.h"

void UNexusPauseMenu::NativeConstruct()
{
	Super::NativeConstruct();

	if (!Button_Settings)
	{
		// Widget tree has no Settings button yet. W_DeathScreen keeps working exactly as before.
		return;
	}

	// The same condition the Blueprint's Construct uses to reveal the PAUSED title. On death the game
	// is not paused, so the button stays collapsed and this widget is a pure death screen.
	const bool bPaused = UGameplayStatics::IsGamePaused(this);

	Button_Settings->SetVisibility(bPaused ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	if (bPaused)
	{
		Button_Settings->OnClicked.AddDynamic(this, &UNexusPauseMenu::HandleSettingsClicked);
	}
}

void UNexusPauseMenu::HandleSettingsClicked()
{
	OpenSettingsMenu();
}

void UNexusPauseMenu::OpenSettingsMenu()
{
	if (!SettingsMenuClass)
	{
		UE_LOG(LogNexusSettings, Warning,
			TEXT("OpenSettingsMenu: SettingsMenuClass is unset on %s. Set it on the W_DeathScreen CDO."), *GetName());
		return;
	}

	if (SettingsMenu && SettingsMenu->IsInViewport())
	{
		return;
	}

	SettingsMenu = CreateWidget<UNexusSettingsMenu>(GetOwningPlayer(), SettingsMenuClass);
	if (!SettingsMenu)
	{
		UE_LOG(LogNexusSettings, Warning, TEXT("OpenSettingsMenu: CreateWidget failed."));
		return;
	}

	// Above the pause menu, which the controller adds at the default ZOrder.
	SettingsMenu->AddToViewport(20);
}
