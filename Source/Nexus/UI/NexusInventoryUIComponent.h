// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Templates/SubclassOf.h"
#include "NexusInventoryUIComponent.generated.h"

class UCommonActivatableWidget;
class UInputAction;
class UUserWidget;

/**
 * Lives on the player controller. At BeginPlay it creates the Narrative HUD wrapper
 * widget (W_NexusNarrativeHUD), adds it to the viewport above W_PlayerHUD, and binds
 * ToggleAction (IA_Inventory) on the controller's EnhancedInputComponent.
 *
 * The toggle calls the Blueprint-defined OpenMenu(ActivatableWidgetClass) on the
 * WBP_NarrativeHUD child via reflection, since that function only exists on the
 * widget Blueprint, not on the native NarrativeCommonHUD parent.
 */
UCLASS(ClassGroup = (Nexus), meta = (BlueprintSpawnableComponent))
class NEXUS_API UNexusInventoryUIComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNexusInventoryUIComponent();

	/** Wrapper widget created at BeginPlay. Must contain a WBP_NarrativeHUD child
	 * named NarrativeHUDWidgetName, or itself derive from WBP_NarrativeHUD. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Nexus|UI")
	TSubclassOf<UUserWidget> HUDWidgetClass;

	/** Menu class passed to WBP_NarrativeHUD.OpenMenu (W_NarrativeMenu_Inventory). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Nexus|UI")
	TSubclassOf<UUserWidget> InventoryMenuClass;

	/** Menu class opened when looting a container (W_NarrativeMenu_Looting). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Nexus|UI")
	TSubclassOf<UUserWidget> LootMenuClass;

	/** Input action that toggles the inventory menu (IA_Inventory). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Nexus|UI")
	TObjectPtr<UInputAction> ToggleAction;

	/** Name of the WBP_NarrativeHUD child inside the wrapper widget's tree. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Nexus|UI")
	FName NarrativeHUDWidgetName = TEXT("NarrativeHUD");

	/** Viewport ZOrder for the wrapper. W_PlayerHUD is added at the default (0). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Nexus|UI")
	int32 ViewportZOrder = 10;

	/** Opens the inventory menu, or closes it when the menu we opened is still active. */
	UFUNCTION(BlueprintCallable, Category = "Nexus|UI")
	void ToggleInventoryMenu();

	/** Opens LootMenuClass on the Narrative HUD. The loot source must already be set on the
	 * player's RunInventory (see UNexusAbilityUILibrary::OpenContainerLoot). The menu itself
	 * reads the run inventory + its LootSource and closes via CommonUI Back/OnEndLooting. */
	UFUNCTION(BlueprintCallable, Category = "Nexus|UI")
	void OpenLootMenu();

	/** Opens the looting menu with the STASH as the loot source: the front-end stash/loadout
	 * browser. Thin wrapper over UNexusAbilityUILibrary::OpenStashLoadout so W_MainMenu's Stash
	 * button has a UObject method to bind OnClicked to (see BindStashLoadoutButton). */
	UFUNCTION(BlueprintCallable, Category = "Nexus|UI")
	void OpenStashLoadoutMenu();

	UFUNCTION(BlueprintPure, Category = "Nexus|UI")
	UUserWidget* GetHUDWidget() const { return HUDWidget; }

protected:
	virtual void BeginPlay() override;

private:
	/** The WBP_NarrativeHUD instance inside the wrapper (or the wrapper itself). */
	UUserWidget* ResolveNarrativeHUD() const;

	/** Calls WBP_NarrativeHUD.OpenMenu(MenuClass) via reflection and returns the pushed
	 * activatable widget. Shared by the inventory toggle and the loot menu. */
	UCommonActivatableWidget* OpenMenuByClass(TSubclassOf<UUserWidget> MenuClass);

	/** Deactivates the menu we opened. Bound as a CommonUI action on the open menu so the
	 * toggle key closes it even though the active CommonUI menu captures game input. */
	void CloseInventoryMenu();

	UPROPERTY()
	TObjectPtr<UUserWidget> HUDWidget;

	TWeakObjectPtr<UCommonActivatableWidget> OpenMenuWidget;
};
