// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Templates/SubclassOf.h"
#include "NexusAbilityUILibrary.generated.h"

class UButton;
class UTextBlock;
class UGameplayAbility;
class UNexusAbilityPreviewController;
class UNexusStatsPanel;

/**
 * UI helpers for the ability screen. Requirement data lives in Blueprint types
 * (DA_Ability_Base / S_AbilityData / S_AbilityRequirement), so it is read via reflection.
 *
 * Stat source: the player pawn's AbilitySystemComponent when one exists (in-game),
 * otherwise the persisted BP_SaveGame (main menu). Save games are resolved from
 * BP_MyGameInstance's SavedGameByPlayerID map first, then from the newest .sav on disk
 * (slots are named after the player controller's object name by the Blueprint save flow).
 */
UCLASS()
class NEXUS_API UNexusAbilityUILibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Checks every W_AbilitySlot inside AbilitiesScreen. A slot is unlocked
	 * (bIsUnlocked = true, LockOverlay hidden) when its ability is already in the
	 * save game's UnlockedAbilities list, or when all its stat requirements are met
	 * against the live ASC (in-game) or the saved talent data (main menu).
	 */
	UFUNCTION(BlueprintCallable, Category = "Nexus|UI", meta = (DefaultToSelf = "AbilitiesScreen"))
	static void CheckRequirements(UUserWidget* AbilitiesScreen);

	/**
	 * Permanently unlocks an ability: adds the ability data class's name to the save
	 * game's UnlockedAbilities array and writes the save to disk. Creates the save if
	 * none exists yet. Returns true if the save was written (or already contained it).
	 */
	UFUNCTION(BlueprintCallable, Category = "Nexus|UI", meta = (WorldContext = "WorldContextObject"))
	static bool UnlockAbility(const UObject* WorldContextObject, TSubclassOf<UObject> AbilityDataClass);

	/** True if the ability data class is in the save game's UnlockedAbilities array. */
	UFUNCTION(BlueprintCallable, Category = "Nexus|UI", meta = (WorldContext = "WorldContextObject"))
	static bool IsAbilityUnlocked(const UObject* WorldContextObject, TSubclassOf<UObject> AbilityDataClass);

	/**
	 * Writes the ability data class's path name into AssignedAbilities[SlotIndex] (keybind
	 * slots 0-3) and saves to disk. Fails when the ability is not in UnlockedAbilities
	 * or SlotIndex is out of range. An ability occupies at most one slot: any other
	 * slot already holding it is cleared. Pass a null class to just clear the slot.
	 * When a player pawn exists (in-game, authority), its granted keybind abilities
	 * are re-synced immediately via GrantAssignedAbilities.
	 */
	UFUNCTION(BlueprintCallable, Category = "Nexus|UI", meta = (WorldContext = "WorldContextObject"))
	static bool AssignAbilityToSlot(const UObject* WorldContextObject, TSubclassOf<UObject> AbilityDataClass, int32 SlotIndex);

	/** The ability data class path name in AssignedAbilities[SlotIndex], or empty when unassigned/no save. */
	UFUNCTION(BlueprintCallable, Category = "Nexus|UI", meta = (WorldContext = "WorldContextObject"))
	static FString GetAssignedAbility(const UObject* WorldContextObject, int32 SlotIndex);

	/** The keybind slot (0-3) holding the ability data class, or -1 when unassigned. */
	UFUNCTION(BlueprintCallable, Category = "Nexus|UI", meta = (WorldContext = "WorldContextObject"))
	static int32 GetAssignedSlotForAbility(const UObject* WorldContextObject, TSubclassOf<UObject> AbilityDataClass);

	/**
	 * Reads a stat from the save game's S_PlayerTalentData. Accepts stat names
	 * (Strength, Dexterity, Intelligence, Faith, Vitality, Endurance, Mana); Vitality
	 * and Endurance map to the struct's Health and Stamina fields. If no save exists
	 * yet, a fresh one is created with every stat at 10 and 50 stat points.
	 */
	UFUNCTION(BlueprintCallable, Category = "Nexus|UI", meta = (WorldContext = "WorldContextObject"))
	static float GetSavedStat(const UObject* WorldContextObject, FName StatName);

	/**
	 * Spends Amount stat points to raise StatName by Amount, then writes the save to
	 * disk. Returns false (and changes nothing) when the save has fewer points than
	 * Amount or the stat name is unknown.
	 */
	UFUNCTION(BlueprintCallable, Category = "Nexus|UI", meta = (WorldContext = "WorldContextObject"))
	static bool UpgradeSavedStat(const UObject* WorldContextObject, FName StatName, int32 Amount = 1);

	/** Unspent stat points in the save game (creates a fresh save like GetSavedStat). */
	UFUNCTION(BlueprintCallable, Category = "Nexus|UI", meta = (WorldContext = "WorldContextObject"))
	static int32 GetSavedStatPoints(const UObject* WorldContextObject);

	/**
	 * Fills the VerticalBox named "StatsContainer" inside AbilitiesScreen with a
	 * UNexusStatsPanel: a stat points readout, one row per RPG stat with -/+ buttons
	 * that stage pending changes, and shared SAVE/CANCEL buttons. Nothing touches the
	 * save file until SAVE is clicked, which also re-runs CheckRequirements.
	 */
	UFUNCTION(BlueprintCallable, Category = "Nexus|UI", meta = (DefaultToSelf = "AbilitiesScreen"))
	static void SetupStatsUI(UUserWidget* AbilitiesScreen);

	/**
	 * Binds every W_AbilitySlot's OnAbilitySlotClicked dispatcher inside AbilitiesScreen
	 * to a preview controller that fills the Designer-placed preview widgets (PreviewIcon,
	 * PreviewName, PreviewDescription, PreviewRequirementsBox, UnlockButton +
	 * UnlockButtonText) when a slot is clicked. The Unlock button calls UnlockAbility for
	 * the selected ability and re-runs CheckRequirements. Buttons Keybind1-4 assign the
	 * selected (unlocked) ability to keybind slots 1-4 via AssignAbilityToKeybindSlot.
	 * Safe to call repeatedly: a second call for the same screen only refreshes the
	 * existing panel.
	 */
	UFUNCTION(BlueprintCallable, Category = "Nexus|UI", meta = (DefaultToSelf = "AbilitiesScreen"))
	static void SetupAbilityPreview(UUserWidget* AbilitiesScreen);

	/** The ability data class currently shown in the preview panel (null before any click). */
	UFUNCTION(BlueprintPure, Category = "Nexus|UI", meta = (DefaultToSelf = "AbilitiesScreen"))
	static TSubclassOf<UObject> GetSelectedAbility(UUserWidget* AbilitiesScreen);

	/**
	 * Assigns an unlocked ability to keybind slot 1-4 (keys One-Four, GAS input IDs
	 * Ability1-4) and writes the save. Clicking the slot the ability already occupies
	 * clears it; assigning also removes the ability from any other slot it held, so an
	 * ability never occupies two slots. Returns false (and saves nothing) when the
	 * ability is not in the save's UnlockedAbilities or the slot is out of range.
	 */
	UFUNCTION(BlueprintCallable, Category = "Nexus|UI", meta = (WorldContext = "WorldContextObject"))
	static bool AssignAbilityToKeybindSlot(const UObject* WorldContextObject,
		TSubclassOf<UObject> AbilityDataClass, int32 KeybindSlot);

	/**
	 * The save's AssignedAbilities as exactly 4 entries (keybind slots 1-4): each is an
	 * ability data class path name, or empty when the slot is free.
	 */
	UFUNCTION(BlueprintCallable, Category = "Nexus|UI", meta = (WorldContext = "WorldContextObject"))
	static TArray<FString> GetAssignedAbilities(const UObject* WorldContextObject);

	/**
	 * Grants every keybind ability in the save to Character's ASC: loads each assigned
	 * slot's ability data class, reads the GameplayAbility from AbilityClass inside its
	 * S_AbilityData, and grants it with GAS input ID Ability1-4 (5-8) so keys One-Four
	 * activate it. Abilities previously granted on those input IDs are cleared first,
	 * so calling again re-syncs with the save. Runs automatically for player characters
	 * from ANexusCharacterBase::PossessedBy. Returns the number of abilities granted.
	 */
	UFUNCTION(BlueprintCallable, Category = "Nexus|UI")
	static int32 GrantAssignedAbilities(class ACharacter* Character);

	/**
	 * Tooling: exports TargetClass's CDO property to its text form (FProperty::ExportText).
	 * Empty string when the class/property is missing.
	 */
	UFUNCTION(BlueprintCallable, Category = "Nexus|Tooling")
	static FString GetClassDefaultPropertyText(UClass* TargetClass, FName PropertyName);

	/**
	 * Tooling counterpart: imports ValueText into the CDO property (FProperty::ImportText,
	 * which ignores per-field editability restrictions) and marks the package dirty so the
	 * asset can be saved. Returns false when the property is missing or the text fails to
	 * parse. Intended for editor scripting against data-only Blueprint CDOs.
	 */
	UFUNCTION(BlueprintCallable, Category = "Nexus|Tooling")
	static bool SetClassDefaultPropertyText(UClass* TargetClass, FName PropertyName, const FString& ValueText);

	/** Gold from the save game's Gold variable (creates a fresh save like GetSavedStat). */
	UFUNCTION(BlueprintPure, Category = "Nexus|Loot", meta = (WorldContext = "WorldContextObject"))
	static int32 GetGold(const UObject* WorldContextObject);

	/**
	 * Total quantity of BP_Item_Gold currently held in the player's RunInventory (player 0).
	 * This is the in-dungeon carried gold the HUD shows; it is lost on death and banked on
	 * extraction like any other run item. Returns 0 when the inventory or gold class is missing.
	 */
	UFUNCTION(BlueprintPure, Category = "Nexus|Loot", meta = (WorldContext = "WorldContextObject"))
	static int32 GetRunInventoryGold(const UObject* WorldContextObject);

	/**
	 * Adds Amount to the save game's Gold variable, writes the save to disk immediately
	 * (loot should survive a crash), and refreshes any on-screen GoldText widget.
	 * Returns the new total, or -1 when no save/Gold variable exists.
	 */
	UFUNCTION(BlueprintCallable, Category = "Nexus|Loot", meta = (WorldContext = "WorldContextObject"))
	static int32 AddGold(const UObject* WorldContextObject, int32 Amount);

	/**
	 * Sets the text of the TextBlock named "GoldText" in any top-level widget to the
	 * saved gold value. Called by AddGold and shortly after possession; false when no
	 * GoldText widget is on screen yet.
	 */
	UFUNCTION(BlueprintCallable, Category = "Nexus|Loot", meta = (WorldContext = "WorldContextObject"))
	static bool UpdateGoldDisplay(const UObject* WorldContextObject);

	/** Adds Count to PlayerActor's HealthPotionCount Blueprint variable (no cap exists). */
	UFUNCTION(BlueprintCallable, Category = "Nexus|Loot")
	static bool GrantPotion(AActor* PlayerActor, int32 Count = 1);

	/**
	 * Full pickup flow for BP_LootPickup's ActorBeginOverlap: ignores non-player actors,
	 * reads the pickup's bIsGold/GoldAmount/PickupSound variables, grants gold (saved
	 * immediately) or a potion, plays the sound, and destroys the pickup.
	 */
	UFUNCTION(BlueprintCallable, Category = "Nexus|Loot", meta = (DefaultToSelf = "Pickup"))
	static void HandleLootPickup(AActor* Pickup, AActor* OtherActor);

	/**
	 * Enemy-death loot roll: DropChance to drop anything, then GoldShare of drops are
	 * gold, the rest potions. Deferred-spawns BP_LootPickup 50 units above DeadEnemy
	 * (bIsGold set before spawn finishes) and restyles the mesh for potion drops.
	 * Returns the spawned pickup or null when the roll fails.
	 */
	UFUNCTION(BlueprintCallable, Category = "Nexus|Loot", meta = (DefaultToSelf = "DeadEnemy"))
	static AActor* SpawnLootDrop(AActor* DeadEnemy, float DropChance = 0.7f, float GoldShare = 0.6f);

	/**
	 * The named NarrativeInventoryComponent ("RunInventory" or "Stash") on the player state
	 * reached from PlayerActor. PlayerActor may be the pawn, its controller, or the player
	 * state itself. Logs the components it did find when the name does not resolve.
	 */
	static class UNarrativeInventoryComponent* FindPlayerInventory(AActor* PlayerActor, FName ComponentName);

	/**
	 * Chests/containers: fills Container's own NarrativeInventoryComponent with a random amount
	 * of gold (MinGold..MaxGold of BP_Item_Gold) and potions (MinPotions..MaxPotions of
	 * BP_Item_HealthPotion). Call once (e.g. chest BeginPlay). No-op without authority or an
	 * inventory component. Returns true when at least one item stack was added.
	 */
	UFUNCTION(BlueprintCallable, Category = "Nexus|Loot", meta = (DefaultToSelf = "Container"))
	static bool PopulateContainerLoot(AActor* Container, int32 MinGold = 30, int32 MaxGold = 80,
		int32 MinPotions = 1, int32 MaxPotions = 2);

	/**
	 * Chests/containers: opens the Narrative looting UI for Looter against Container's inventory.
	 * Sets Container's inventory as the LootSource on the looter's RunInventory (authority only),
	 * then opens LootMenuClass via the looter's UNexusInventoryUIComponent. Looted items flow into
	 * RunInventory. Returns false when the container/run inventory/UI component can't be resolved.
	 */
	UFUNCTION(BlueprintCallable, Category = "Nexus|Loot", meta = (DefaultToSelf = "Container"))
	static bool OpenContainerLoot(AActor* Looter, AActor* Container);

	/**
	 * Chests/containers: OpenContainerLoot after Delay seconds, so the lid starts swinging before
	 * the looting UI covers it. A Delay of 0 opens immediately. The container is held weakly: a
	 * chest destroyed inside the delay window simply never opens a menu.
	 */
	UFUNCTION(BlueprintCallable, Category = "Nexus|Loot", meta = (DefaultToSelf = "Container"))
	static void OpenContainerLootDelayed(AActor* Looter, AActor* Container, float Delay = 0.2f);

	/**
	 * Chest lid: eases the scene component named PivotName on Chest towards TargetRoll and returns
	 * the roll it reached, which the caller stores back into its CurrentRoll variable (call from
	 * Event Tick). Speed is an FInterpTo speed; Speed <= 0 snaps to TargetRoll in one call, which
	 * is how BeginPlay poses the lid closed without a visible swing.
	 *
	 * The pivot is found BY NAME, never by child index: the editor-only BillboardComponent sits at
	 * child index 0 in PIE and is stripped from a packaged build, so any hardcoded index would
	 * silently rotate the wrong component in the shipped game.
	 */
	UFUNCTION(BlueprintCallable, Category = "Nexus|Chest", meta = (DefaultToSelf = "Chest"))
	static float TickChestLid(AActor* Chest, FName PivotName, float CurrentRoll, float TargetRoll,
		float Speed, float DeltaTime);

	/**
	 * Chest lid: the one-shot open beat -- Sound at the chest (SoundPitch pitches the reused wood
	 * footstep down into a lid thunk) and DustFX at the lid's hinge. Either may be null; a chest
	 * with both null opens silently, which is a supported way to ship.
	 */
	UFUNCTION(BlueprintCallable, Category = "Nexus|Chest", meta = (DefaultToSelf = "Chest"))
	static void PlayChestOpenFX(AActor* Chest, class USoundBase* Sound, class UParticleSystem* DustFX,
		FName PivotName, float SoundPitch = 0.6f);

	/**
	 * Chests/containers: "Take All" for the looting UI. Moves every stack in Source into Looter's
	 * RunInventory, clamping each request to the space actually available for that item class --
	 * Narrative's RequestLootItem is all-or-nothing per call (it refuses outright when the asked
	 * quantity does not wholly fit), so an unclamped full-stack request would take *nothing* from
	 * a stack that only partly fits. Whatever does not fit is left in the container.
	 *
	 * Iterates a snapshot: GetItems() returns the array by value, so consuming items mid-loop
	 * cannot invalidate it.
	 *
	 * @param Looter              The looting player -- pawn, controller or player state all resolve.
	 *                            No DefaultToSelf: this is called from the looting widget, whose
	 *                            self is a UUserWidget and would not satisfy an AActor pin.
	 * @param Source              The container's inventory (the looting widget's LootSource).
	 * @param OutError            Why the last stack could not be taken, for the UI notification.
	 *                            Empty when everything transferred.
	 * @param bOutContainerEmpty  True when Source holds nothing afterwards -- the caller closes
	 *                            the looting menu on this.
	 * @return Number of individual items (not stacks) moved into RunInventory.
	 */
	UFUNCTION(BlueprintCallable, Category = "Nexus|Loot")
	static int32 TakeAllLoot(AActor* Looter, class UNarrativeInventoryComponent* Source,
		FText& OutError, bool& bOutContainerEmpty);

	/**
	 * Extraction game: empties the run inventory (items + currency) so dying loses the run's
	 * loot. Safe to call when the player state or component is missing. Items whose
	 * CanBeRemoved() is false (quest items) survive by the plugin's own rule.
	 */
	UFUNCTION(BlueprintCallable, Category = "Nexus|Extraction", meta = (DefaultToSelf = "PlayerActor"))
	static void ClearRunInventory(AActor* PlayerActor);

	/**
	 * Extraction game: moves every RunInventory item into Stash, then writes Stash to the
	 * "NexusStash" save slot. Each item is added to the stash first and only consumed from
	 * the run inventory for the amount the stash actually accepted (AmountGiven), so a full
	 * stash leaves loot in the run inventory rather than voiding it. Run currency is folded
	 * into the stash. Returns false when any item was partially/fully rejected or the save
	 * failed; extraction is never blocked, the caller may proceed regardless.
	 */
	UFUNCTION(BlueprintCallable, Category = "Nexus|Extraction", meta = (DefaultToSelf = "PlayerActor"))
	static bool ExtractRunInventoryToStash(AActor* PlayerActor);

	/**
	 * Extraction game: loads the "NexusStash" slot into the Stash component only, never into
	 * RunInventory. A missing slot is not an error (first run): the plugin's Load() returns
	 * false without touching the inventory. Returns true when a save was loaded.
	 */
	UFUNCTION(BlueprintCallable, Category = "Nexus|Extraction", meta = (DefaultToSelf = "PlayerActor"))
	static bool LoadStash(AActor* PlayerActor);

	/**
	 * The shared finisher gate, used by every cinematic finisher (Sky Crusher, Burden). Passes only
	 * when the avatar's lock-on component holds a target that is not BP_Enemy_Boss (or a child) and
	 * is already below half health; the cached lock-on target is reused rather than re-traced.
	 * OutTarget is the validated victim, which the cinematic frames its camera on. On a false return
	 * OutTarget is null and the caller must EndAbility before the montage plays — the gate has to run
	 * before Parent: ActivateAbility, which is what starts the montage.
	 *
	 * Named for Sky Crusher because that is what it shipped with; the logic is not specific to it.
	 */
	UFUNCTION(BlueprintCallable, Category = "Nexus|Abilities", meta = (DefaultToSelf = "Ability"))
	static bool CanSkyCrusherExecute(UGameplayAbility* Ability, AActor*& OutTarget);

	/**
	 * Arms Sky Crusher's cinematic, gap-close and finisher for one activation. Target is the
	 * validated victim from CanSkyCrusherExecute. Must be called after the gate passes and BEFORE
	 * Parent: ActivateAbility, which is what starts the montage the whole sequence is timed against.
	 *
	 * Off the montage clock: play rate 1.4, the camera cut at launch, slow motion at the apex, a
	 * teleport onto the target as the descent starts, and a guaranteed-lethal finisher on the slam's
	 * Event.HitScan.Start. See FNexusFinisherProfile::SkyCrusher.
	 */
	UFUNCTION(BlueprintCallable, Category = "Nexus|Abilities", meta = (DefaultToSelf = "Ability"))
	static void ArmSkyCrusher(UGameplayAbility* Ability, AActor* Target);

	/**
	 * Arms Burden's cinematic force-push finisher for one activation. Same contract as ArmSkyCrusher:
	 * after the gate passes, before Parent: ActivateAbility.
	 *
	 * The mage never moves. Off the montage clock: the camera cut at 0.30, then at the cast beat (0.60)
	 * a guaranteed-lethal blow, the corpse thrown backwards off the caster once the death pipeline
	 * ragdolls it, and a coned shove on every other living non-boss enemy in range.
	 * See FNexusFinisherProfile::Burden.
	 */
	UFUNCTION(BlueprintCallable, Category = "Nexus|Abilities", meta = (DefaultToSelf = "Ability"))
	static void ArmBurden(UGameplayAbility* Ability, AActor* Target);

	/**
	 * Unwinds whatever ArmSkyCrusher or ArmBurden set up: restores global time dilation and the
	 * avatar's CustomTimeDilation unconditionally, restores the view target ONLY if the camera beat
	 * actually took it, and retires the spawned camera. Call from OnEndAbility so it runs on every exit
	 * path (end/cancel/death/interrupt), including an EndAbility that fires mid-flight before any beat
	 * has run. A no-op when nothing was armed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Nexus|Abilities", meta = (DefaultToSelf = "Ability"))
	static void RestoreFromCinematic(UGameplayAbility* Ability);
};

/**
 * Forwards one W_AbilitySlot's OnAbilitySlotClicked dispatcher to the preview controller.
 * One relay exists per slot because the dispatcher's ClickedAbilityData payload is always
 * null (the Blueprint's pin cannot be wired: the slot stores a class reference but the
 * dispatcher param is an object reference), so the slot must be identified by binding.
 */
UCLASS()
class NEXUS_API UNexusAbilitySlotRelay : public UObject
{
	GENERATED_BODY()

public:
	void InitRelay(UUserWidget* InSlotWidget, UNexusAbilityPreviewController* InController);

	/** Signature-compatible with OnAbilitySlotClicked; the payload is ignored (see above). */
	UFUNCTION()
	void HandleSlotClicked(UObject* ClickedAbilityData);

private:
	TWeakObjectPtr<UUserWidget> SlotWidget;
	TWeakObjectPtr<UNexusAbilityPreviewController> Controller;
};

/**
 * Drives the ability preview panel on W_AbilitiesScreen: shows the clicked slot's
 * name/description/icon, one requirement chip per stat (green when met, red when not),
 * and the Unlock button (Unlocked = grey disabled, Unlock = gold enabled, Locked = grey
 * disabled). Kept alive by a static GC registry because dispatcher bindings hold only
 * weak references.
 *
 * Keybind1-4 buttons assign the selected ability to keybind slots 1-4 (toggle: clicking
 * its current slot unassigns). They are enabled only while the selection is unlocked;
 * the slot the selection occupies is tinted gold, the rest default white.
 */
UCLASS()
class NEXUS_API UNexusAbilityPreviewController : public UObject
{
	GENERATED_BODY()

public:
	void InitController(UUserWidget* InScreen);

	/** Selects the slot's AbilityData class and refreshes the preview panel. */
	void ShowPreviewForSlot(UUserWidget* InSlotWidget);

	/** Re-fills the preview widgets from the current selection and stat source. */
	void RefreshPreview();

	TSubclassOf<UObject> GetSelectedAbility() const { return SelectedAbility; }
	UUserWidget* GetScreen() const { return Screen.Get(); }

protected:
	UFUNCTION()
	void HandleUnlockClicked();

	// One UFUNCTION per Keybind button because OnClicked carries no payload.
	UFUNCTION()
	void HandleKeybind1Clicked();

	UFUNCTION()
	void HandleKeybind2Clicked();

	UFUNCTION()
	void HandleKeybind3Clicked();

	UFUNCTION()
	void HandleKeybind4Clicked();

private:
	/** Assigns/toggles the selected ability into keybind slot 1-4, then re-tints. */
	void HandleKeybindClicked(int32 KeybindSlot);

	/** Keybind1-4 enabled only when the selection is unlocked; assigned slot gold. */
	void RefreshKeybindButtons();

	TWeakObjectPtr<UUserWidget> Screen;
	TWeakObjectPtr<UUserWidget> SelectedSlot;

	UPROPERTY()
	TSubclassOf<UObject> SelectedAbility;

	UPROPERTY()
	TArray<TObjectPtr<UNexusAbilitySlotRelay>> Relays;
};

/**
 * One stat row inside UNexusStatsPanel: name, value (with "saved -> pending" preview
 * while a change is staged), and -/+ buttons that only adjust the pending delta.
 * Minus never goes below the saved value; plus is limited by the panel's remaining
 * points. Built entirely in C++ because the abilities screen lives in the main menu
 * where W_StatRow's ASC-driven upgrade flow cannot run.
 */
UCLASS()
class NEXUS_API UNexusStatRow : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitRow(FName InStatName, UNexusStatsPanel* InPanel);

	/** Sets the persisted value and discards any pending delta. */
	void SetSavedValue(int32 InSavedValue);

	FName GetStatName() const { return StatName; }
	int32 GetPendingDelta() const { return PendingDelta; }
	void ClearPending() { PendingDelta = 0; }

	void RefreshTexts();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

	UFUNCTION()
	void HandlePlusClicked();

	UFUNCTION()
	void HandleMinusClicked();

private:
	FName StatName;
	int32 SavedValue = 0;
	int32 PendingDelta = 0;

	UPROPERTY()
	TWeakObjectPtr<UNexusStatsPanel> Panel;

	UPROPERTY()
	TObjectPtr<UTextBlock> NameText;

	UPROPERTY()
	TObjectPtr<UTextBlock> ValueText;

	UPROPERTY()
	TObjectPtr<UButton> MinusButton;

	UPROPERTY()
	TObjectPtr<UButton> PlusButton;
};

/**
 * The whole save-game-backed stats panel created by SetupStatsUI: points readout,
 * one UNexusStatRow per stat, and shared SAVE/CANCEL buttons. Pending deltas live on
 * the rows; SAVE writes them all to the save file in one batch (single SaveGameToSlot)
 * and re-runs CheckRequirements on the abilities screen, CANCEL discards them.
 */
UCLASS()
class NEXUS_API UNexusStatsPanel : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitPanel(UUserWidget* InAbilitiesScreen);

	/** Saved points minus every row's pending delta. */
	int32 GetRemainingPoints() const;

	/** Rows call this after any pending change; refreshes all rows, points, buttons. */
	void NotifyPendingChanged();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

	UFUNCTION()
	void HandleSaveClicked();

	UFUNCTION()
	void HandleCancelClicked();

private:
	/** Re-reads saved stat values and points into the rows, discarding pending deltas. */
	void RefreshFromSave();

	int32 GetTotalPendingDelta() const;

	UPROPERTY()
	TWeakObjectPtr<UUserWidget> AbilitiesScreen;

	UPROPERTY()
	TArray<TObjectPtr<UNexusStatRow>> Rows;

	UPROPERTY()
	TObjectPtr<UTextBlock> PointsText;

	UPROPERTY()
	TObjectPtr<UButton> SaveButton;

	UPROPERTY()
	TObjectPtr<UButton> CancelButton;

	int32 SavedPoints = 0;
};
