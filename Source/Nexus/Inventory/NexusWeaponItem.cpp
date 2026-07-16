// Fill out your copyright notice in the Description page of Project Settings.

#include "Nexus/Inventory/NexusWeaponItem.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Pawn.h"
#include "GameplayEffect.h"
#include "InventoryComponent.h"
#include "UObject/UnrealType.h"

/** The melee trace ends this far past the visual mesh top (the original author's tuning intent). */
static constexpr float GTraceOvershootCm = 30.0f;

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

		// VARIANTS: re-skin the shared category actor from THIS item's PickupMesh + tuning before
		// EquipWeapon, so the draw montage reveals the right variant from frame one. Runs in the
		// GUARD-1 skip case too (same class hotkey-equipped, different variant item activated).
		ApplyVariantVisuals(Manager);

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

AActor* UNexusWeaponItem::FindOurWeaponActor(UActorComponent* Manager) const
{
	// Equipped first: with multiple variants of one category, the in-hand actor is the one to re-skin.
	if (const FObjectProperty* Prop = FindFProperty<FObjectProperty>(Manager->GetClass(), TEXT("EquippedWeapon")))
	{
		UObject* Equipped = Prop->GetObjectPropertyValue_InContainer(Manager);
		if (Equipped && Equipped->GetClass()->IsChildOf(WeaponClass))
		{
			return Cast<AActor>(Equipped);
		}
	}

	if (const FArrayProperty* Prop = FindFProperty<FArrayProperty>(Manager->GetClass(), TEXT("StowedWeapons")))
	{
		if (const FObjectPropertyBase* Inner = CastField<FObjectPropertyBase>(Prop->Inner))
		{
			FScriptArrayHelper Helper(Prop, Prop->ContainerPtrToValuePtr<void>(Manager));
			for (int32 Idx = 0; Idx < Helper.Num(); ++Idx)
			{
				UObject* Weapon = Inner->GetObjectPropertyValue(Helper.GetRawPtr(Idx));
				if (Weapon && Weapon->GetClass()->IsChildOf(WeaponClass))
				{
					return Cast<AActor>(Weapon);
				}
			}
		}
	}

	return nullptr;
}

void UNexusWeaponItem::ApplyVariantVisuals(UActorComponent* Manager) const
{
	AActor* Weapon = FindOurWeaponActor(Manager);
	if (!Weapon)
	{
		return;
	}

	// Locate the actor's components by their SCS variable names (trailing-space tolerant — this
	// project's Blueprints have shipped names with trailing spaces before).
	UStaticMeshComponent* MeshComponent = nullptr;
	USceneComponent* TraceEnd = nullptr;
	for (UActorComponent* Component : Weapon->GetComponents())
	{
		const FString Name = Component ? Component->GetName().TrimStartAndEnd() : FString();
		if (Name == TEXT("WeaponMesh"))
		{
			MeshComponent = Cast<UStaticMeshComponent>(Component);
		}
		else if (Name == TEXT("TraceEnd"))
		{
			TraceEnd = Cast<USceneComponent>(Component);
		}
	}

	// One field drives ground AND hand: PickupMesh is what the actor wields. If it is unset we
	// leave the actor's design-time look alone rather than half-apply the tuning.
	UStaticMesh* Mesh = PickupMesh.LoadSynchronous();
	if (Mesh && MeshComponent)
	{
		MeshComponent->SetStaticMesh(Mesh);
		MeshComponent->SetRelativeScale3D(FVector(InHandScale));
		MeshComponent->SetRelativeLocation(GripOffset);
		MeshComponent->SetRelativeRotation(GripRotation);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UNexusWeaponItem %s: variant visuals skipped (PickupMesh %s, WeaponMesh component %s)."),
			*GetNameSafe(this), Mesh ? TEXT("ok") : TEXT("unset"), MeshComponent ? TEXT("ok") : TEXT("missing"));
	}

	// Melee trace far point tracks the variant's visual length. Derived for every weapon; inert on
	// non-hitscan categories (the staff never runs the sphere trace).
	if (TraceEnd)
	{
		float EndZ = TraceLengthOverride;
		if (EndZ <= 0.0f && Mesh)
		{
			EndZ = Mesh->GetBoundingBox().Max.Z * InHandScale + GripOffset.Z + GTraceOvershootCm;
		}
		if (EndZ > 0.0f)
		{
			TraceEnd->SetRelativeLocation(FVector(0.0f, 0.0f, EndZ));
		}
	}
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
