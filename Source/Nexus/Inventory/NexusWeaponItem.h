// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EquippableItem.h"
#include "ActiveGameplayEffectHandle.h"
#include "Templates/SubclassOf.h"
#include "NexusWeaponItem.generated.h"

class UGameplayEffect;

/**
 * An inventory weapon item (bag icon) that, when equipped through the Narrative equipment rails
 * ("Equip" use action -> EquipmentComponent ES_Weapon slot), puts the matching weapon ACTOR in the
 * player's hand by driving the existing Blueprint WeaponsManagerComponent. The item is the trigger
 * and persistence layer; WeaponsManagerComponent stays the sole execution layer (sockets, montages,
 * anim class, OnWeaponChanged -> ability bar rebuild).
 *
 * BP COUPLING CHOICE: WeaponsManagerComponent is a Blueprint class, so C++ cannot link against it.
 * We call it via reflection (FindFunction + ProcessEvent) rather than a BlueprintImplementableEvent
 * hop, because the BIE route would require every item Blueprint to re-implement the same graph
 * (component lookup, guards, calls) and keep it in sync — the reflection helper keeps the logic in
 * one audited place and the item BPs stay pure data. Function/property names were verified against
 * the live class (EquipWeapon / UnequipWeapon / GiveWeapon, EquippedWeapon / StowedWeapons — no
 * trailing spaces). Parameters are set by iterating the UFunction's parm properties, so we do not
 * depend on Blueprint parameter names.
 *
 * TOGGLE GUARDS: UNarrativeItem's Use() toggles bActive, and WeaponsManagerComponent::EquipWeapon
 * toggles OFF when re-requested with the already-equipped class. Stacked naively these double-toggle.
 * Two guards keep every state combination convergent:
 *   - HandleEquip skips EquipWeapon when the equipped weapon is already our WeaponClass
 *     (e.g. hotkey 1/2 equipped it first; equipping the item must not disarm the player).
 *   - HandleUnequip skips UnequipWeapon unless the equipped weapon IS our WeaponClass
 *     (e.g. the player hotkey-swapped weapons after equipping this item; deactivating this item
 *     must not disarm the weapon it never equipped).
 *
 * LOADOUT FLOW: an equipped weapon's item lives in the equipment panel's ES_Weapon slot ONLY --
 * ShouldShowInInventory hides active items from the bag list (the plugin already does exactly
 * this for vendors; we extend the rule to the player). SetActive broadcasts nothing, so
 * HandleEquip/HandleUnequip follow up with ClientRefreshInventory to rebuild the bag grid the
 * moment the row should appear/disappear.
 *
 * VARIANTS (mesh-from-item): the mesh you PICK UP is the mesh you WIELD. The weapon actor class
 * is a CATEGORY (anim set, sockets, montages, ability gate); the item is the VARIANT. On equip,
 * ApplyVariantVisuals re-skins the shared category actor from this item's PickupMesh and the
 * tuning fields below, so a new axe-swinging weapon is a pure-data item Blueprint -- no new
 * BP_Weapon_* class. TraceEnd (the melee sphere-trace's far point) auto-derives from the scaled
 * mesh bounds top + GripOffset.Z + a 30cm overshoot -- the convention the original author tuned
 * (old blade top ~123cm vs trace end ~154) -- with TraceLengthOverride as the escape hatch.
 * The re-skin happens BEFORE EquipWeapon so the draw montage reveals the right variant from
 * frame one (the montage-notify attach is actor-root -> socket, orthogonal to the mesh).
 */
UCLASS()
class NEXUS_API UNexusWeaponItem : public UEquippableItem
{
	GENERATED_BODY()

public:

	UNexusWeaponItem();

	/** The weapon actor class (e.g. BP_Weapon_Axe) handed to WeaponsManagerComponent on equip. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<AActor> WeaponClass;

	/** Optional stats effect applied to the owning pawn while equipped (Phase 2 — leave unset for Phase 1). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<UGameplayEffect> EquipStatsEffect;

	/** In-hand scale applied to the weapon actor's WeaponMesh (independent of GroundScale). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Variant")
	float InHandScale = 1.0f;

	/** Grip correction: WeaponMesh relative location. Actor origin stays the grip point. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Variant")
	FVector GripOffset = FVector::ZeroVector;

	/** Grip correction: WeaponMesh relative rotation. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Variant")
	FRotator GripRotation = FRotator::ZeroRotator;

	/**
	 * Melee trace far point (TraceEnd relative Z). 0 = auto-derive from the scaled mesh bounds
	 * top + GripOffset.Z + the 30cm overshoot convention; set explicitly to override.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Variant")
	float TraceLengthOverride = 0.0f;

	/** Ground scale for this item's dropped pickup mesh (was a hardcoded per-class table in C++). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Variant")
	float GroundScale = 1.0f;

protected:

	virtual void HandleEquip_Implementation() override;
	virtual void HandleUnequip_Implementation() override;

	/** Loadout flow: equipped (active) weapons leave the bag list; they live in the equipment slot. */
	virtual bool ShouldShowInInventory_Implementation() const override;

private:

	/** Find the Blueprint WeaponsManagerComponent on the owning pawn (matched by generated-class name). */
	UActorComponent* FindWeaponsManager() const;

	/** True if the manager's EquippedWeapon actor is (a child of) our WeaponClass. */
	bool IsOurWeaponEquipped(UActorComponent* Manager) const;

	/** True if any actor in the manager's StowedWeapons array is (a child of) our WeaponClass. */
	bool IsOurWeaponStowed(UActorComponent* Manager) const;

	/** The live weapon actor of our WeaponClass — equipped first, else stowed. Null if neither. */
	AActor* FindOurWeaponActor(UActorComponent* Manager) const;

	/** Re-skin the (shared, per-category) weapon actor from this item's PickupMesh + tuning fields. */
	void ApplyVariantVisuals(UActorComponent* Manager) const;

	/** Reflection call into the Blueprint manager. Sets the first class-typed parameter to ClassParam if given. */
	static bool CallManagerFunction(UActorComponent* Manager, FName FunctionName, UClass* ClassParam);

	/** Rebuilds the bag grid after bActive changes (SetActive itself broadcasts nothing). */
	void RefreshBagList() const;

	/** Active handle for EquipStatsEffect so unequip removes exactly what equip applied. */
	FActiveGameplayEffectHandle StatsEffectHandle;
};
