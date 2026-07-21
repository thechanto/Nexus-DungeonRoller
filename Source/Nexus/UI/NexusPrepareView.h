// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Styling/SlateTypes.h"
#include "NexusPrepareView.generated.h"

class UButton;
class UImage;
class UScrollBox;
class UTextBlock;
class UVerticalBox;
class UNarrativeInventoryComponent;
class UNarrativeItem;
class UNexusPrepareView;

/**
 * One clickable item tile in the Prepare page. UButton::OnClicked carries no payload, so each
 * tile owns a relay that remembers which item it is and which side it lives on (stash vs basket),
 * exactly the pattern UNexusAbilitySlotRelay uses on the abilities screen. Kept alive by the
 * view's Tiles array; discarded and rebuilt whenever a move refreshes the panes.
 */
UCLASS()
class UNexusPrepareTile : public UObject
{
	GENERATED_BODY()

public:
	void Init(UNexusPrepareView* InView, UNarrativeItem* InItem, bool bInStashSide);

	UFUNCTION()
	void HandleClicked();

private:
	TWeakObjectPtr<UNexusPrepareView> View;
	TWeakObjectPtr<UNarrativeItem> Item;
	bool bStashSide = false;
};

/**
 * The "Prepare for Round" loadout page for the main menu, built in C++ at runtime (RebuildWidget)
 * for the same reason as UNexusStashView: a factory-created WidgetBlueprint has no scriptable root
 * widget, so W_PrepareView is an EMPTY blueprint whose CDO only carries fonts, styles and the
 * travel level name as native UPROPERTYs (recompile-proof).
 *
 * Two panes: LEFT = the Stash, RIGHT = the RunInventory basket (12/40, the same component the run
 * uses, so the basket can never exceed run capacity by construction). Each pane is a list of item
 * tiles read straight off the component -- NOT the plugin's WBP_Loot_* panes, whose tile clicks
 * resolve their inventories through the owning pawn (GetInventoryComponentFromTarget) and come back
 * null at the pawnless menu. Clicking a stash tile moves that whole stack into the basket; clicking
 * a basket tile moves it back. All moves go through UNexusAbilityUILibrary::MoveItemBetween, which
 * drives the RequestLootItem rails on two component pointers with no pawn involved.
 *
 * LAUNCH commits the basket (SaveStashAndLoadout: Stash -> "NexusStash", basket -> "NexusLoadout")
 * and opens the run level; the in-run LoadRunLoadout splice picks the basket up. BACK just closes,
 * discarding nothing on disk -- moves are component state only until Launch, so quitting the page
 * leaves the saved stash untouched.
 */
UCLASS()
class NEXUS_API UNexusPrepareView : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Cloned off the W_MainMenu title/label fonts by script; default engine font if unset. */
	UPROPERTY(EditDefaultsOnly, Category = "Prepare")
	FSlateFontInfo TitleFont;

	UPROPERTY(EditDefaultsOnly, Category = "Prepare")
	FSlateFontInfo LabelFont;

	/** Style for the Back / Launch buttons (cloned off a W_MainMenu button by script). */
	UPROPERTY(EditDefaultsOnly, Category = "Prepare")
	FButtonStyle ButtonStyle;

	/** Style for the per-item tile buttons; left at engine default when unset. */
	UPROPERTY(EditDefaultsOnly, Category = "Prepare")
	FButtonStyle TileButtonStyle;

	/** Level LAUNCH travels to after committing the loadout. Matches the menu Play button. */
	UPROPERTY(EditDefaultsOnly, Category = "Prepare")
	FName TravelLevelName = FName(TEXT("LV_Soul_Cave"));

	UFUNCTION(BlueprintCallable, Category = "Prepare")
	void CloseView();

	/** Re-reads both inventories and rebuilds the tile lists + gold readout. Headless-testable. */
	UFUNCTION(BlueprintCallable, Category = "Prepare")
	bool RefreshPanes();

	/** Commits the basket (SaveStashAndLoadout) and opens the run level. */
	UFUNCTION(BlueprintCallable, Category = "Prepare")
	void LaunchRun();

	/** Tile relay callback: moves the clicked stack across (stash<->basket) and refreshes. */
	void MoveTileItem(UNarrativeItem* Item, bool bStashSide);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;

	/** Runtime-built children (no BindWidget -- the BP tree is empty on purpose). */
	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> StashList;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> BasketList;

	UPROPERTY(Transient)
	TObjectPtr<UButton> Button_Back;

	UPROPERTY(Transient)
	TObjectPtr<UButton> Button_Launch;

	/** Keeps the per-tile relays alive; reset each RefreshPanes. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UNexusPrepareTile>> Tiles;

private:
	void BuildTree();
	void RebuildList(UVerticalBox* List, UNarrativeInventoryComponent* Source, bool bStashSide);

	UFUNCTION()
	void HandleBackClicked();

	UFUNCTION()
	void HandleLaunchClicked();
};
