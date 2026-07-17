// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Styling/SlateTypes.h"
#include "NexusStashView.generated.h"

class UButton;

/** Owned here (StashView logs to it most); NexusMainMenu.cpp logs to it too via this extern. */
NEXUS_API DECLARE_LOG_CATEGORY_EXTERN(LogStashView, Log, All);

/**
 * Read-only stash browser for the main menu. The widget tree is built in C++ at runtime
 * (RebuildWidget) because scripting cannot give a factory-created WidgetBlueprint a root
 * widget — W_StashView is deliberately an EMPTY blueprint whose CDO only carries the
 * pane class, fonts and button style (native UPROPERTYs, so they survive recompiles).
 *
 * Hosts an instance of the plugin's WBP_Loot_TheirInventory (StashPanelClass) and points
 * it at the PlayerState's "Stash" inventory component on construct. The pane's
 * "Initialize From Inventory" is a Blueprint function on plugin content, so it is invoked
 * by reflection; the filter param stays null — the stock looting menu leaves it
 * unconnected too.
 */
UCLASS()
class NEXUS_API UNexusStashView : public UUserWidget
{
	GENERATED_BODY()

public:
	/** The plugin pane shown inside the view (set to WBP_Loot_TheirInventory_C on the CDO). */
	UPROPERTY(EditDefaultsOnly, Category = "Stash")
	TSubclassOf<UUserWidget> StashPanelClass;

	/** Cloned off the W_MainMenu title/label fonts by script; default engine font if unset. */
	UPROPERTY(EditDefaultsOnly, Category = "Stash")
	FSlateFontInfo TitleFont;

	UPROPERTY(EditDefaultsOnly, Category = "Stash")
	FSlateFontInfo LabelFont;

	/** Cloned off the W_MainMenu SkillTree button style by script. */
	UPROPERTY(EditDefaultsOnly, Category = "Stash")
	FButtonStyle BackButtonStyle;

	UFUNCTION(BlueprintCallable, Category = "Stash")
	void CloseStashView();

	/** Points StashPanel at the stash. BlueprintCallable so headless tests can re-drive it. */
	UFUNCTION(BlueprintCallable, Category = "Stash")
	bool InitializeStashPanel();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;

	/** Runtime-built children (no BindWidget — the BP tree is empty on purpose). */
	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> StashPanel;

	UPROPERTY(Transient)
	TObjectPtr<UButton> Button_Back;

private:
	void BuildTree();

	UFUNCTION()
	void HandleBackClicked();
};
