// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Templates/SubclassOf.h"
#include "GameplayAbilitySpecHandle.h"
#include "Nexus/GameplayAbilitySystem/Abilities/NexusGameplayAbility.h"
#include "NexusAbilityUILibrary.generated.h"

class UButton;
class UTextBlock;
class UGameplayAbility;
class UGameplayEffect;
class UNexusAbilityPreviewController;
class UNexusStatsPanel;
class UAbilitySystemComponent;
class UTexture2D;

/**
 * Which flask the player is drinking. The enumerator ORDER must stay aligned with the
 * Blueprint enum /Game/Potions/E_PotionType, because BP_NexusPlayer's "SelectedPotionType"
 * variable is of that Blueprint type and is read here by index (see ReadSelectedPotionType).
 * The two enums coexist on purpose: E_PotionType stays the player-facing selection type so
 * the working Q-swap and HUD colour switches keep compiling untouched, while this one is what
 * the C++ dispatch switches on. Adding a potion means adding it to BOTH, in the same slot.
 */
UENUM(BlueprintType)
enum class ENexusPotionType : uint8
{
	Health = 0,
	Mana   = 1
};

/** What a BP_LootPickup is. Replaces the old bIsGold bool, which could not express a third drop. */
UENUM(BlueprintType)
enum class ENexusLootType : uint8
{
	Gold         = 0,
	HealthPotion = 1,
	ManaPotion   = 2,
	/**
	 * Weapon drops: rare, enemy-only. Two curated items, each a full GLootFields row so style and
	 * grant flow through the existing table with no special-casing -- SpawnLootDrop just sub-rolls
	 * 50/50 between them. Index 3 repurposes the old reserved Weapon stub (never persisted on disk).
	 */
	WeaponRustedAxe       = 3,
	WeaponApprenticeStaff = 4
};

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
	 * Checks every W_AbilitySlot inside AbilitiesScreen and gives its lock one of three
	 * readings: hidden (bIsUnlocked = true) when the ability is in the save game's
	 * UnlockedAbilities list; gold when it is not owned but every stat requirement is
	 * already met, i.e. the player can afford it right now; dim grey otherwise.
	 * Requirements are evaluated against the live ASC (in-game) or the saved talent data
	 * (main menu). Also refills each tile's RequirementsRow with green/red stat chips.
	 *
	 * Note this only READS the save; unlocking is UnlockAbility's job.
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
	 * Item-2 HUD badge source: the input slot the ability was actually GRANTED on, read from
	 * the live FGameplayAbilitySpec (its runtime InputID) rather than the ability CDO's static
	 * AbilityInputID. The warrior/mage class abilities leave the CDO field at None and are bound
	 * to a keybind slot only at grant time (GrantAssignedAbilities sets spec InputID = Ability1-4),
	 * so W_Ability must key its keybind glyph off the spec, not the CDO. W_Ability already carries
	 * the AbilitySpecHandle it was spawned with; pass that plus the owning pawn's ASC. Returns
	 * None when the ASC/handle is invalid or the spec InputID falls outside EAbilityInputID.
	 */
	UFUNCTION(BlueprintPure, Category = "Nexus|UI")
	static EAbilityInputID GetGrantedAbilityInputID(UAbilitySystemComponent* ASC,
		FGameplayAbilitySpecHandle Handle);

	/**
	 * Item-2 icon fallback: the ability's own authored icon, read from S_AbilityData.AbilityIcon
	 * on the DA whose AbilityData.AbilityClass resolves to the given ability's class. Lets
	 * W_Ability show an icon for abilities that have no DT_AbilityMetaData row (the warrior/mage
	 * class abilities), so SetAbilityImage can fall back here on a row miss. The DA class paths
	 * under /Game/GameplayAbilitySystem/Abilities/DataAssets are scanned from the asset registry
	 * once and cached; the icon is read fresh from the matching DA CDO each call. Null when no
	 * DA maps to the ability's class.
	 */
	UFUNCTION(BlueprintPure, Category = "Nexus|UI")
	static UTexture2D* GetAbilityIconForAbility(UGameplayAbility* Ability);

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

	/**
	 * Adds Count to PlayerActor's HealthPotionCount / ManaPotionCount Blueprint variable
	 * (no cap exists). PotionType is last and defaults to Health so the existing
	 * BP_Item_HealthPotion OnUse call site keeps compiling without a re-wire.
	 */
	UFUNCTION(BlueprintCallable, Category = "Nexus|Loot", meta = (DefaultToSelf = "PlayerActor"))
	static bool GrantPotion(AActor* PlayerActor, int32 Count = 1,
		ENexusPotionType PotionType = ENexusPotionType::Health);

	// --- Potion drinking -------------------------------------------------------------
	//
	// The whole drink path lives here rather than in parallel Blueprint graphs: one place
	// decides which count, which charge pool, which sip amount and which GameplayEffect a
	// potion type maps to (see the FPotionFields table in the .cpp). BP_NexusPlayer keeps
	// the DATA -- the counts, the charges, the per-sip amounts and the effect classes are
	// all Blueprint variables read by reflection, which is also what keeps the GameplayEffect
	// assets real cook dependencies of BP_NexusPlayer. A LoadClass string path here would
	// cook to nothing.

	/** BP_NexusPlayer's "SelectedPotionType" (the Q-swap state), as the C++ enum. */
	UFUNCTION(BlueprintPure, Category = "Nexus|Potions", meta = (DefaultToSelf = "PlayerActor"))
	static ENexusPotionType GetSelectedPotionType(AActor* PlayerActor);

	/** Flasks held of the given type. */
	UFUNCTION(BlueprintPure, Category = "Nexus|Potions", meta = (DefaultToSelf = "PlayerActor"))
	static int32 GetPotionCount(AActor* PlayerActor, ENexusPotionType PotionType);

	/** Sets the flask count of the given type. Also how a PIE test hands the player mana potions. */
	UFUNCTION(BlueprintCallable, Category = "Nexus|Potions", meta = (DefaultToSelf = "PlayerActor"))
	static bool SetPotionCount(AActor* PlayerActor, ENexusPotionType PotionType, int32 NewCount);

	/** Charge left in the current flask of the given type (0..PotionMaxCharges). */
	UFUNCTION(BlueprintPure, Category = "Nexus|Potions", meta = (DefaultToSelf = "PlayerActor"))
	static float GetPotionCharges(AActor* PlayerActor, ENexusPotionType PotionType);

	UFUNCTION(BlueprintCallable, Category = "Nexus|Potions", meta = (DefaultToSelf = "PlayerActor"))
	static bool SetPotionCharges(AActor* PlayerActor, ENexusPotionType PotionType, float NewCharges);

	/** W_PotionSlot's count binding: the SELECTED type's flask count. */
	UFUNCTION(BlueprintPure, Category = "Nexus|Potions", meta = (DefaultToSelf = "PlayerActor"))
	static int32 GetSelectedPotionCount(AActor* PlayerActor);

	/** W_PotionSlot's bar binding: the SELECTED type's charge as a 0..1 fill. */
	UFUNCTION(BlueprintPure, Category = "Nexus|Potions", meta = (DefaultToSelf = "PlayerActor"))
	static float GetSelectedPotionChargePercent(AActor* PlayerActor);

	/**
	 * The restore effect for a potion type, read from BP_NexusPlayer's "HealthPotionEffect" /
	 * "ManaPotionEffect" variables. Both are infinite periodic GEs driven by the Data.Heal
	 * SetByCaller magnitude; they differ only in the attribute they modify.
	 */
	UFUNCTION(BlueprintPure, Category = "Nexus|Potions", meta = (DefaultToSelf = "PlayerActor"))
	static TSubclassOf<UGameplayEffect> GetPotionEffectClass(AActor* PlayerActor, ENexusPotionType PotionType);

	/** Restore per tick for a potion type ("HealAmountPerSip" / "ManaAmountPerSip"). */
	UFUNCTION(BlueprintPure, Category = "Nexus|Potions", meta = (DefaultToSelf = "PlayerActor"))
	static float GetPotionSipAmount(AActor* PlayerActor, ENexusPotionType PotionType);

	/**
	 * F held down: applies the type's restore GE and starts the 1s drain tick. Fails (and
	 * applies nothing) when the player holds no flask of that type. Drinking a second type
	 * while one is already going stops the first, so only one GE is ever active.
	 */
	UFUNCTION(BlueprintCallable, Category = "Nexus|Potions", meta = (DefaultToSelf = "PlayerActor"))
	static bool StartPotion(AActor* PlayerActor, ENexusPotionType PotionType);

	/** F released / drink finished: removes the restore GE and stops the drain tick. */
	UFUNCTION(BlueprintCallable, Category = "Nexus|Potions", meta = (DefaultToSelf = "PlayerActor"))
	static void StopPotion(AActor* PlayerActor);

	// --- Loot ------------------------------------------------------------------------

	/**
	 * Full pickup flow for BP_LootPickup's ActorBeginOverlap: ignores non-player actors,
	 * reads the pickup's LootType/GoldAmount/PickupSound variables, grants gold (saved
	 * immediately) or the matching potion item, plays the sound, and destroys the pickup.
	 */
	UFUNCTION(BlueprintCallable, Category = "Nexus|Loot", meta = (DefaultToSelf = "Pickup"))
	static void HandleLootPickup(AActor* Pickup, AActor* OtherActor);

	/**
	 * Enemy-death loot roll, in two stages: DropChance decides whether anything drops at all,
	 * then GoldShare of drops are gold and the rest are potions -- and a potion drop splits
	 * again, ManaPotionShare of the time, into mana rather than health. Deferred-spawns
	 * BP_LootPickup 50 units above DeadEnemy (LootType set before spawn finishes) and restyles
	 * it to match. Returns the spawned pickup or null when the first roll fails.
	 */
	UFUNCTION(BlueprintCallable, Category = "Nexus|Loot", meta = (DefaultToSelf = "DeadEnemy"))
	static AActor* SpawnLootDrop(AActor* DeadEnemy, float DropChance = 0.7f, float GoldShare = 0.6f);

	/**
	 * Boss death shower: a fixed, guaranteed celebration payout of one of *every* loot type -- gold,
	 * both potions, and both weapons -- scattered on a ring around the corpse. No rolling. Each pickup
	 * lands on its own spoke (even angles plus a small random ring rotation) and is dropped onto the
	 * floor by a per-point downward trace, so the five don't stack their trigger spheres or prompts.
	 * The gold pickup carries BossGold instead of the trash-mob default. Enemy-only, boss-only: this
	 * runs *alongside* the inherited SpawnLootDrop roll, which stays as a bonus sixth drop.
	 */
	UFUNCTION(BlueprintCallable, Category = "Nexus|Loot", meta = (DefaultToSelf = "DeadBoss"))
	static void SpawnBossLoot(AActor* DeadBoss, float RingRadius = 150.0f, int32 BossGold = 100);

	/**
	 * True only when Actor is an enemy whose Health has already hit zero. Anything else --
	 * null, the player, a level-placed trap -- reads false, so callers can gate on "my
	 * instigator is a corpse" without special-casing every other kind of instigator.
	 * Used by BP_AOE_Base to stop damage fields that outlive the enemy that cast them.
	 */
	UFUNCTION(BlueprintPure, Category = "Nexus|Combat")
	static bool IsActorDeadEnemy(AActor* Actor);

	/**
	 * Puts a dropped item on the ground in front of the player, for the inventory's drop actions.
	 *
	 * Spawns BP_DroppedItemPickup -- ours, not the plugin's BP_BasicItemPickup. Dropped items are
	 * E-interact pickups (same grammar as chests), never walk-over: the plugin's pickup grants
	 * itself on overlap from *Blueprint*, which C++ cannot suppress, so it needed a collision-off
	 * arm timer to stop it self-collecting on the frame it spawned. Ours simply has no overlap
	 * grant, so that whole hack is gone. Enemy loot (BP_LootPickup) stays walk-over.
	 *
	 * Sets the item class and quantity before FinishSpawningActor so BeginPlay sees them, then
	 * resolves the item's soft PickupMesh onto the mesh component and labels the prompt.
	 *
	 * It lands DropForwardOffset in front of the player, pulled back from any wall in between and
	 * dropped onto the floor below.
	 *
	 * Only spawns the world actor -- the caller still removes the item from the inventory.
	 * PlayerActor may be the pawn or its controller. Returns the pickup, or null on failure.
	 */
	UFUNCTION(BlueprintCallable, Category = "Nexus|Loot")
	static AActor* SpawnDroppedItem(AActor* PlayerActor, TSubclassOf<class UNarrativeItem> ItemClass,
		int32 Quantity = 1);

	/**
	 * Labels a dropped pickup's interact prompt from the item it holds: "[E] Mana Potion", or
	 * "[E] Mana Potion x3" when it holds a stack.
	 *
	 * The prompt widgets are otherwise per-verb with hardcoded text (W_InteractPrompt "[E] Extract",
	 * W_InteractPrompt_Open "[E] Open"), which is why the chest needed its own widget class. A
	 * dropped item's label is dynamic, so this reuses W_InteractPrompt's asset and overwrites the
	 * TextBlock in the widget component's own widget instance -- per-pickup, so it cannot leak
	 * into the chest's prompt. Called at spawn and again after a partial take.
	 */
	UFUNCTION(BlueprintCallable, Category = "Nexus|Loot", meta = (DefaultToSelf = "Pickup"))
	static bool RefreshDroppedItemPrompt(AActor* Pickup);

	/** BP_DroppedItemPickup's ShowInteractPrompt: shows/hides its prompt widget component. */
	UFUNCTION(BlueprintCallable, Category = "Nexus|Loot", meta = (DefaultToSelf = "Pickup"))
	static void SetDroppedItemPromptVisible(AActor* Pickup, bool bShow);

	/**
	 * BP_DroppedItemPickup's Interact: grants the pickup's item to the interacting player's
	 * RunInventory and destroys it.
	 *
	 * A full inventory is handled rather than swallowed: TryAddItemFromClass reports how much it
	 * actually took, so a partial add decrements the pickup's remaining quantity, relabels the
	 * prompt and leaves it standing. The item is never destroyed without being granted.
	 * Returns true when anything at all was granted.
	 */
	UFUNCTION(BlueprintCallable, Category = "Nexus|Loot", meta = (DefaultToSelf = "Pickup"))
	static bool TakeDroppedItem(AActor* Pickup, AActor* InteractingActor);

	/**
	 * An item's own class, typed. Blueprint cannot produce this: GetObjectClass returns a plain
	 * "Object Class Reference", which the schema refuses to narrow to a "Narrative Item Class
	 * Reference", and the only legal bridge (Cast To Class) is a node type our tooling cannot
	 * create. So the drop graphs get their ItemClass from here instead.
	 */
	UFUNCTION(BlueprintPure, Category = "Nexus|Loot")
	static TSubclassOf<class UNarrativeItem> GetNarrativeItemClass(class UNarrativeItem* Item);

	/**
	 * An item's use sound. Same reason as GetNarrativeItemClass: reading UNarrativeItem::UseSound
	 * needs a Get node bound to a property on another class, which our tooling cannot create (it
	 * only makes self-member getters). The use sound moved to the Use button, so the widgets need it.
	 */
	UFUNCTION(BlueprintPure, Category = "Nexus|Inventory")
	static class USoundBase* GetItemUseSound(class UNarrativeItem* Item);

	/**
	 * The named NarrativeInventoryComponent ("RunInventory" or "Stash") on the player state
	 * reached from PlayerActor. PlayerActor may be the pawn, its controller, or the player
	 * state itself. Logs the components it did find when the name does not resolve.
	 */
	static class UNarrativeInventoryComponent* FindPlayerInventory(AActor* PlayerActor, FName ComponentName);

	/**
	 * Chests/containers: fills Container's own NarrativeInventoryComponent with a random amount
	 * of gold (MinGold..MaxGold of BP_Item_Gold) and potions (MinPotions..MaxPotions). Each
	 * potion rolls its type independently, on the same ManaPotionShare split the enemy drops
	 * use, so a chest can hold a mix of both. Call once (e.g. chest BeginPlay). No-op without
	 * authority or an inventory component. Returns true when at least one item stack was added.
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
	 * "NexusStash" save slot. Deactivates every active item first (SetActive(false) -> the
	 * equipped weapon is lowered and its stats effect removed): extraction ends the loadout,
	 * and a bActive=true flag round-tripping through the stash save would re-equip the weapon
	 * straight out of the STASH on next session's Load(). Each item is added to the stash first
	 * and only consumed from
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
	 * Main-menu loadout: opens the Narrative looting menu with the STASH as the loot source. The
	 * two looting panes become the stash browser: "their" side shows the Stash component, "your"
	 * side shows RunInventory, which at the front end doubles as the loadout basket -- items
	 * clicked across move between the two components on the same RequestLootItem rails the chest
	 * flow uses. Front-end only by nature (it needs the player state components, which the menu
	 * game mode provides via PlayerStateClass = BP_NexusPlayerState), but nothing here persists:
	 * moves are component state only, and quitting the menu forgets them. SaveStashAndLoadout on
	 * the Play click is the one commit point.
	 *
	 * No DefaultToSelf: callers are widgets/components, whose self is not an Actor.
	 */
	UFUNCTION(BlueprintCallable, Category = "Nexus|Loadout")
	static bool OpenStashLoadout(AActor* PlayerActor);

	/**
	 * Main-menu loadout: the Play-click commit, spliced BEFORE Open Level. Writes the Stash
	 * component to "NexusStash" (debiting whatever was moved into the basket) and then the
	 * RunInventory basket to "NexusLoadout". Stash is written FIRST on purpose: a crash between
	 * the two writes loses the in-flight items instead of duplicating them (loadout-first would
	 * leave the items in BOTH saves). Returns false when either save failed; the caller may
	 * still travel -- a failed save means the run simply starts from the old slots.
	 */
	UFUNCTION(BlueprintCallable, Category = "Nexus|Loadout")
	static bool SaveStashAndLoadout(AActor* PlayerActor);

	/**
	 * Main-menu loadout: loads the "NexusLoadout" slot into RunInventory. Splice into the player
	 * state's BeginPlay between LoadStash and SeedStartingWeaponItems -- after LoadStash because
	 * both saves are written together, before the seeder so its dedupe guard sees brought
	 * weapons and vetoes the free copies. The plugin's Load() REPLACES the component's contents,
	 * which is correct here: RunInventory is empty at BeginPlay in both levels.
	 *
	 * The slot outlives the menu but not the run: at the front end (recognised by the game mode
	 * having no DefaultPawnClass) the slot is loaded into the basket and KEPT, so browsing the
	 * menu and quitting can never destroy a crash-recovered loadout -- Play rewrites the slot
	 * from the basket anyway. In a run level the slot is loaded and CONSUMED, so the items exist
	 * only in the run from that point on: dying loses them (ClearRunInventory), extracting banks
	 * them (ExtractRunInventoryToStash), and a stale slot can never re-grant them.
	 */
	UFUNCTION(BlueprintCallable, Category = "Nexus|Loadout", meta = (DefaultToSelf = "PlayerActor"))
	static bool LoadRunLoadout(AActor* PlayerActor);

	/**
	 * Main menu: binds the button named ButtonName inside MenuWidget to the owning controller's
	 * UNexusInventoryUIComponent::OpenStashLoadoutMenu. Splice into W_MainMenu's Construct chain
	 * (DefaultToSelf fills MenuWidget). Exists because our tooling cannot create component-bound
	 * OnClicked events in a widget graph; the button is resolved with GetWidgetFromName, so the
	 * new tree widget needs no "Is Variable" tick either. AddUniqueDynamic keeps a re-fired
	 * Construct from stacking bindings. Logs and no-ops when the button or component is missing.
	 */
	UFUNCTION(BlueprintCallable, Category = "Nexus|Loadout", meta = (DefaultToSelf = "MenuWidget"))
	static void BindStashLoadoutButton(UUserWidget* MenuWidget, FName ButtonName);

	/**
	 * Loadout: grants the two starting weapon items (rusted axe, apprentice's staff) to the run
	 * inventory. Replaces WeaponsManager's StartingWeapons pre-stow -- weapons now only ever
	 * arrive as ITEMS and reach the hand by equipping one. Call on BeginPlay AFTER LoadStash:
	 * the dedupe guard reads the stash, and a weapon item already held in the run inventory or
	 * the stash (equipped/hidden copies included) is not seeded again, so extracting every run
	 * does not pile up free axes. Returns the number of items granted (0-2).
	 */
	UFUNCTION(BlueprintCallable, Category = "Nexus|Equipment", meta = (DefaultToSelf = "PlayerActor"))
	static int32 SeedStartingWeaponItems(AActor* PlayerActor);

	/**
	 * Loadout: the hotkey path. Finds the first UNexusWeaponItem in the run inventory whose
	 * WeaponClass matches and toggles its active state; the item rails then do everything a bag
	 * "Equip" click does (equipment-slot swap, HandleEquip/HandleUnequip guards, weapon draw,
	 * bag-row hide/show). GA_EquipWeapon calls this INSTEAD of WeaponsManager.EquipWeapon so
	 * hotkeys and bag clicks share one path and item active state can never drift from the
	 * weapon in hand. Returns false (touching nothing) when no matching item is in the bag:
	 * the hotkey for an unowned weapon no-ops, by design.
	 *
	 * No DefaultToSelf: the caller is a GA graph, whose self is not an Actor -- wire
	 * GetAvatarActorFromActorInfo into PlayerActor (pawn/controller/player state all resolve).
	 */
	UFUNCTION(BlueprintCallable, Category = "Nexus|Equipment")
	static bool ToggleWeaponItemByClass(AActor* PlayerActor, TSubclassOf<AActor> WeaponClass);

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

private:
	/**
	 * The shared core behind SpawnDroppedItem and the weapon loot/boss drops: spawns a
	 * BP_DroppedItemPickup holding ItemClass x Quantity at an explicit transform, sizes its mesh by
	 * the item (weapons up-scaled from the loot-token sizes via DropScaleForItem), and labels the
	 * prompt hidden. The public SpawnDroppedItem adds the in-front-of-the-player placement; the loot
	 * paths pass a corpse or ring point. Only spawns the world actor -- the caller owns any inventory
	 * removal. Returns the pickup, or null on failure.
	 */
	static AActor* SpawnDroppedItemAt(class UWorld* World, TSubclassOf<class UNarrativeItem> ItemClass,
		int32 Quantity, const FTransform& SpawnTransform);
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
