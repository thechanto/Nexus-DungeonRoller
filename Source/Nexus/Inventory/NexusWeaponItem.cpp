// Fill out your copyright notice in the Description page of Project Settings.

#include "Nexus/Inventory/NexusWeaponItem.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameFramework/Pawn.h"
#include "GameplayEffect.h"
#include "InventoryComponent.h"
#include "UObject/UnrealType.h"

UNexusWeaponItem::UNexusWeaponItem()
{
	// Weapons always live in the ES_Weapon slot; the reparented item BPs inherit this default.
	EquippableSlot = EEquippableSlot::ES_Weapon;
}

void UNexusWeaponItem::HandleEquip_Implementation()
{
	Super::HandleEquip_Implementation();

	APawn* Pawn = GetOwningPawn();
	if (!Pawn || !WeaponClass)
	{
		return;
	}

	if (UActorComponent* Manager = FindWeaponsManager())
	{
		// GiveWeapon spawns a new actor unconditionally (AddUnique can't dedupe fresh instances),
		// so only grant when no instance of our class is stowed or in hand.
		if (!IsOurWeaponStowed(Manager) && !IsOurWeaponEquipped(Manager))
		{
			CallManagerFunction(Manager, TEXT("GiveWeapon"), *WeaponClass);
		}

		// GUARD 1: EquipWeapon toggles OFF when re-requested with the equipped class — if the
		// hotkey already put our weapon in hand, equipping the item must not disarm the player.
		if (!IsOurWeaponEquipped(Manager))
		{
			CallManagerFunction(Manager, TEXT("EquipWeapon"), *WeaponClass);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UNexusWeaponItem %s: owning pawn %s has no WeaponsManagerComponent."),
			*GetNameSafe(this), *GetNameSafe(Pawn));
	}

	// Phase 2: stats while equipped. Inert until EquipStatsEffect is set on the item.
	if (EquipStatsEffect)
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Pawn))
		{
			FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
			Context.AddSourceObject(this);

			const FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(EquipStatsEffect, 1.f, Context);
			if (Spec.IsValid())
			{
				StatsEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
			}
		}
	}

	RefreshBagList();
}

void UNexusWeaponItem::HandleUnequip_Implementation()
{
	Super::HandleUnequip_Implementation();

	// Always clear our stats effect, whatever the weapon actor is doing.
	if (StatsEffectHandle.IsValid())
	{
		if (UAbilitySystemComponent* ASC = StatsEffectHandle.GetOwningAbilitySystemComponent())
		{
			ASC->RemoveActiveGameplayEffect(StatsEffectHandle);
		}
		StatsEffectHandle = FActiveGameplayEffectHandle();
	}

	if (UActorComponent* Manager = FindWeaponsManager())
	{
		// GUARD 2: only lower the weapon if OUR class is actually in hand — if the player
		// hotkey-swapped to another weapon, deactivating this item must not disarm them.
		if (IsOurWeaponEquipped(Manager))
		{
			CallManagerFunction(Manager, TEXT("UnequipWeapon"), nullptr);
		}
	}

	RefreshBagList();
}

bool UNexusWeaponItem::ShouldShowInInventory_Implementation() const
{
	// Loadout flow: an equipped weapon's item lives in the equipment panel's ES_Weapon slot,
	// not the bag list. The plugin's default implementation hides active items for vendor
	// inventories only; the player gets the same rule here.
	return !bActive && Super::ShouldShowInInventory_Implementation();
}

void UNexusWeaponItem::RefreshBagList() const
{
	// SetActive flips bActive without broadcasting OnInventoryUpdated, so the bag grid would
	// keep showing (or missing) this item's row until some later rebuild. ClientRefreshInventory
	// broadcasts it explicitly — a Client-Reliable RPC, which in standalone runs immediately.
	if (OwningInventory)
	{
		OwningInventory->ClientRefreshInventory();
	}
}

UActorComponent* UNexusWeaponItem::FindWeaponsManager() const
{
	const APawn* Pawn = GetOwningPawn();
	if (!Pawn)
	{
		return nullptr;
	}

	// WeaponsManagerComponent is a Blueprint class (generated class "WeaponsManagerComponent_C"),
	// so we identify it by name instead of StaticClass. NOTE: do not use GetComponentByClass from
	// Blueprint here — this native iteration avoids that path entirely.
	for (UActorComponent* Component : Pawn->GetComponents())
	{
		if (Component && Component->GetClass()->GetName().StartsWith(TEXT("WeaponsManagerComponent")))
		{
			return Component;
		}
	}

	return nullptr;
}

bool UNexusWeaponItem::IsOurWeaponEquipped(UActorComponent* Manager) const
{
	const FObjectProperty* Prop = FindFProperty<FObjectProperty>(Manager->GetClass(), TEXT("EquippedWeapon"));
	if (!Prop)
	{
		return false;
	}

	const UObject* Equipped = Prop->GetObjectPropertyValue_InContainer(Manager);
	return Equipped && Equipped->GetClass()->IsChildOf(WeaponClass);
}

bool UNexusWeaponItem::IsOurWeaponStowed(UActorComponent* Manager) const
{
	const FArrayProperty* Prop = FindFProperty<FArrayProperty>(Manager->GetClass(), TEXT("StowedWeapons"));
	const FObjectPropertyBase* Inner = Prop ? CastField<FObjectPropertyBase>(Prop->Inner) : nullptr;
	if (!Inner)
	{
		// Fail safe: claim stowed so we never double-spawn via GiveWeapon.
		return true;
	}

	FScriptArrayHelper Helper(Prop, Prop->ContainerPtrToValuePtr<void>(Manager));
	for (int32 Idx = 0; Idx < Helper.Num(); ++Idx)
	{
		const UObject* Weapon = Inner->GetObjectPropertyValue(Helper.GetRawPtr(Idx));
		if (Weapon && Weapon->GetClass()->IsChildOf(WeaponClass))
		{
			return true;
		}
	}

	return false;
}

bool UNexusWeaponItem::CallManagerFunction(UActorComponent* Manager, FName FunctionName, UClass* ClassParam)
{
	UFunction* Function = Manager->FindFunction(FunctionName);
	if (!Function)
	{
		UE_LOG(LogTemp, Warning, TEXT("UNexusWeaponItem: WeaponsManagerComponent has no function '%s'."),
			*FunctionName.ToString());
		return false;
	}

	// Build the parameter block generically: init every parm, then drop ClassParam into the first
	// class-typed input so we never depend on the Blueprint's parameter name.
	TArray<uint8> Parms;
	Parms.AddZeroed(Function->ParmsSize);

	for (TFieldIterator<FProperty> It(Function); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
	{
		It->InitializeValue_InContainer(Parms.GetData());
	}

	if (ClassParam)
	{
		for (TFieldIterator<FProperty> It(Function); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
		{
			if (!It->HasAnyPropertyFlags(CPF_ReturnParm | CPF_OutParm))
			{
				if (FClassProperty* ClassProp = CastField<FClassProperty>(*It))
				{
					ClassProp->SetObjectPropertyValue_InContainer(Parms.GetData(), ClassParam);
					break;
				}
			}
		}
	}

	Manager->ProcessEvent(Function, Parms.GetData());

	for (TFieldIterator<FProperty> It(Function); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
	{
		It->DestroyValue_InContainer(Parms.GetData());
	}

	return true;
}
