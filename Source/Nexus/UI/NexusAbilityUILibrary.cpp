// Fill out your copyright notice in the Description page of Project Settings.

#include "Nexus/UI/NexusAbilityUILibrary.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/Widget.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Engine/GameInstance.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "Particles/ParticleSystem.h"
#include "TimerManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/SaveGame.h"
#include "InventoryComponent.h"
#include "NarrativeItem.h"
#include "Nexus/UI/NexusInventoryUIComponent.h"
#include "HAL/FileManager.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"
#include "Sound/SoundBase.h"
#include "UObject/GCObject.h"
#include "UObject/ObjectKey.h"
#include "UObject/Package.h"
#include "Abilities/GameplayAbility.h"
#include "Nexus/GameplayAbilitySystem/Abilities/NexusAbility_UpgradeStat.h"
#include "Nexus/GameplayAbilitySystem/Abilities/NexusGameplayAbility.h"
#include "Nexus/GameplayAbilitySystem/AttributeSets/BasicAttributeSet.h"
#include "Nexus/GameplayAbilitySystem/Abilities/NexusFinisherDirector.h"
#include "Nexus/GameplayAbilitySystem/Characters/NexusCharacterBase.h"
#include "Nexus/GameplayAbilitySystem/Characters/NexusLockOnComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogNexusAbilityUI, Log, All);

namespace
{
	// Blueprint user-defined struct fields are stored with mangled names
	// ("StatType_2_1234ABCD..."), so match on the authored prefix. Blueprint class
	// variables can also carry accidental trailing whitespace in their authored
	// names (e.g. BP_NexusPlayer's "HealthPotionCount "), so match trimmed too.
	FProperty* FindPropByDisplayName(const UStruct* Owner, const TCHAR* DisplayName)
	{
		if (!Owner)
		{
			return nullptr;
		}
		const FString Prefix = FString(DisplayName) + TEXT("_");
		for (TFieldIterator<FProperty> It(Owner); It; ++It)
		{
			const FString Name = It->GetName();
			if (Name == DisplayName || Name.TrimStartAndEnd() == DisplayName || Name.StartsWith(Prefix))
			{
				return *It;
			}
		}
		return nullptr;
	}

	UClass* LoadSaveGameClass()
	{
		return LoadClass<USaveGame>(nullptr, TEXT("/Game/SavedGameData/BP_SaveGame.BP_SaveGame_C"));
	}

	// Where stat values come from: the live ASC in-game, saved talent data in the menu.
	struct FNexusStatSource
	{
		UAbilitySystemComponent* ASC = nullptr;
		const UStruct* SavedStatsStruct = nullptr;
		const void* SavedStatsPtr = nullptr;

		bool IsValid() const { return ASC != nullptr || SavedStatsPtr != nullptr; }
	};

	/**
	 * Finds the player's BP_SaveGame. Checks BP_MyGameInstance's SavedGameByPlayerID map
	 * first (the live in-game object), then falls back to loading the newest matching
	 * .sav from disk. OutSlotName receives the slot the save belongs to.
	 */
	USaveGame* ResolveSaveGame(const UObject* WorldContext, FString& OutSlotName)
	{
		if (const UGameInstance* GI = UGameplayStatics::GetGameInstance(WorldContext))
		{
			const FMapProperty* MapProp = CastField<FMapProperty>(
				FindPropByDisplayName(GI->GetClass(), TEXT("SavedGameByPlayerID")));
			const FStrProperty* KeyProp = MapProp ? CastField<FStrProperty>(MapProp->KeyProp) : nullptr;
			const FObjectPropertyBase* ValueProp = MapProp ? CastField<FObjectPropertyBase>(MapProp->ValueProp) : nullptr;
			if (KeyProp && ValueProp)
			{
				FScriptMapHelper MapHelper(MapProp, MapProp->ContainerPtrToValuePtr<void>(GI));
				for (int32 Index = 0; Index < MapHelper.GetMaxIndex(); ++Index)
				{
					if (!MapHelper.IsValidIndex(Index))
					{
						continue;
					}
					if (USaveGame* Save = Cast<USaveGame>(ValueProp->GetObjectPropertyValue(MapHelper.GetValuePtr(Index))))
					{
						OutSlotName = KeyProp->GetPropertyValue(MapHelper.GetKeyPtr(Index));
						UE_LOG(LogNexusAbilityUI, Log,
							TEXT("ResolveSaveGame: using GameInstance map entry, slot '%s'"), *OutSlotName);
						return Save;
					}
				}
			}
		}

		// No in-memory save (e.g. main menu on a fresh boot) -> newest save file on disk.
		const FString SaveDir = FPaths::ProjectSavedDir() / TEXT("SaveGames");
		TArray<FString> FileNames;
		IFileManager::Get().FindFiles(FileNames, *(SaveDir / TEXT("*.sav")), /*Files*/ true, /*Directories*/ false);
		FileNames.Sort([&SaveDir](const FString& A, const FString& B)
		{
			return IFileManager::Get().GetTimeStamp(*(SaveDir / A)) > IFileManager::Get().GetTimeStamp(*(SaveDir / B));
		});

		UClass* SaveClass = LoadSaveGameClass();
		for (const FString& FileName : FileNames)
		{
			const FString SlotName = FPaths::GetBaseFilename(FileName);
			USaveGame* Save = UGameplayStatics::LoadGameFromSlot(SlotName, 0);
			if (Save && (!SaveClass || Save->GetClass()->IsChildOf(SaveClass)))
			{
				OutSlotName = SlotName;
				UE_LOG(LogNexusAbilityUI, Log, TEXT("ResolveSaveGame: loaded slot '%s' from disk"), *SlotName);
				return Save;
			}
		}

		UE_LOG(LogNexusAbilityUI, Log, TEXT("ResolveSaveGame: no save game found (map empty, %d file(s) on disk)"),
			FileNames.Num());
		return nullptr;
	}

	// Locates the S_PlayerTalentData struct inside a BP_SaveGame instance.
	void GetSavedTalentDataMutable(USaveGame* Save, const UStruct*& OutStruct, void*& OutPtr)
	{
		OutStruct = nullptr;
		OutPtr = nullptr;
		if (!Save)
		{
			return;
		}
		if (const FStructProperty* TalentProp = CastField<FStructProperty>(
			FindPropByDisplayName(Save->GetClass(), TEXT("PlayerTalentData"))))
		{
			OutStruct = TalentProp->Struct;
			OutPtr = TalentProp->ContainerPtrToValuePtr<void>(Save);
		}
	}

	void GetSavedTalentData(const USaveGame* Save, const UStruct*& OutStruct, const void*& OutPtr)
	{
		void* MutablePtr = nullptr;
		GetSavedTalentDataMutable(const_cast<USaveGame*>(Save), OutStruct, MutablePtr);
		OutPtr = MutablePtr;
	}

	FNumericProperty* FindNumericField(const UStruct* Owner, const TCHAR* Name)
	{
		return CastField<FNumericProperty>(FindPropByDisplayName(Owner, Name));
	}

	bool ReadNumericField(const UStruct* Owner, const void* Ptr, const TCHAR* Name, double& OutValue)
	{
		if (const FNumericProperty* Prop = FindNumericField(Owner, Name))
		{
			const void* ValuePtr = Prop->ContainerPtrToValuePtr<void>(Ptr);
			OutValue = Prop->IsFloatingPoint()
				? Prop->GetFloatingPointPropertyValue(ValuePtr)
				: static_cast<double>(Prop->GetSignedIntPropertyValue(ValuePtr));
			return true;
		}
		return false;
	}

	bool WriteNumericField(const UStruct* Owner, void* Ptr, const TCHAR* Name, double Value)
	{
		if (FNumericProperty* Prop = FindNumericField(Owner, Name))
		{
			void* ValuePtr = Prop->ContainerPtrToValuePtr<void>(Ptr);
			if (Prop->IsFloatingPoint())
			{
				Prop->SetFloatingPointPropertyValue(ValuePtr, Value);
			}
			else
			{
				Prop->SetIntPropertyValue(ValuePtr, static_cast<int64>(Value));
			}
			return true;
		}
		return false;
	}

	// S_PlayerTalentData stores Vitality/Endurance under the pool names, matching
	// W_StatRow's Vitality->MaxHealth / Endurance->MaxStamina attribute mapping.
	FString StatFieldName(FName StatName)
	{
		const FString Name = StatName.ToString();
		if (Name == TEXT("Vitality"))
		{
			return TEXT("Health");
		}
		if (Name == TEXT("Endurance"))
		{
			return TEXT("Stamina");
		}
		return Name;
	}

	const TCHAR* ResolvePointsFieldName(const UStruct* TalentStruct)
	{
		if (FindNumericField(TalentStruct, TEXT("StatPoints")))
		{
			return TEXT("StatPoints");
		}
		if (FindNumericField(TalentStruct, TEXT("TalentPoints")))
		{
			return TEXT("TalentPoints");
		}
		return nullptr;
	}

	// BP_MyGameInstance names slots after GetObjectName(PlayerController); the first
	// player in a session always resolves to this, so a save created from the main
	// menu (where that controller does not exist) is still found by the in-game flow.
	const TCHAR* const GDefaultSaveSlotName = TEXT("BP_ThirdPersonPlayerController_C_0");

	const TCHAR* const GFreshSaveStatFields[] =
	{
		TEXT("Strength"), TEXT("Dexterity"), TEXT("Intelligence"), TEXT("Faith"),
		TEXT("Health"), TEXT("Stamina"), TEXT("Mana")
	};

	// TESTING VALUE: a fresh save is minted with enough points to buy anything on the
	// Skills screen without grinding. Revisit before any real balance pass.
	constexpr double GFreshSaveStatPoints = 100.0;
	constexpr double GFreshSaveTalentPoints = 100.0;

	// Ultimates a fresh save starts with: unlocked and already sitting on their keybind.
	// The unlock key is the data class name, which is derived from the path below, so the
	// path is the single source of truth for both arrays.
	struct FFreshSaveAbilitySeed
	{
		const TCHAR* DataClassPath;
		int32 SlotIndex;
	};

	const FFreshSaveAbilitySeed GFreshSaveAbilities[] =
	{
		{ TEXT("/Game/GameplayAbilitySystem/Abilities/DataAssets/Mage/DA_Ability_Mage_Burden.DA_Ability_Mage_Burden_C"), 2 },
		{ TEXT("/Game/GameplayAbilitySystem/Abilities/DataAssets/Warrior/DA_Ability_Warrior_SkyCrusher.DA_Ability_Warrior_SkyCrusher_C"), 3 }
	};

	// Defined below, next to the UnlockedAbilities/AssignedAbilities helpers it needs.
	void SeedFreshSaveAbilities(USaveGame* Save);

	// The first-run state: all stats 10, GFreshSaveStatPoints points, the
	// GFreshSaveAbilities ultimates unlocked and sitting on their keybinds.
	void InitializeFreshSave(USaveGame* Save)
	{
		const UStruct* TalentStruct = nullptr;
		void* TalentPtr = nullptr;
		GetSavedTalentDataMutable(Save, TalentStruct, TalentPtr);
		if (TalentStruct && TalentPtr)
		{
			for (const TCHAR* Field : GFreshSaveStatFields)
			{
				if (!WriteNumericField(TalentStruct, TalentPtr, Field, 10.0))
				{
					UE_LOG(LogNexusAbilityUI, Warning,
						TEXT("InitializeFreshSave: fresh save has no '%s' field to initialize"), Field);
				}
			}
			WriteNumericField(TalentStruct, TalentPtr, TEXT("StatPoints"), GFreshSaveStatPoints);
			WriteNumericField(TalentStruct, TalentPtr, TEXT("TalentPoints"), GFreshSaveTalentPoints);
		}
		else
		{
			UE_LOG(LogNexusAbilityUI, Warning,
				TEXT("InitializeFreshSave: fresh save has no PlayerTalentData struct to initialize"));
		}

		SeedFreshSaveAbilities(Save);
	}

	/**
	 * ResolveSaveGame, but never returns null while the BP_SaveGame class loads:
	 * when no save exists a fresh one is created (see InitializeFreshSave) and written
	 * to the slot the in-game Blueprint flow will look for.
	 */
	USaveGame* ResolveOrCreateSaveGame(const UObject* WorldContext, FString& OutSlotName)
	{
		if (USaveGame* Existing = ResolveSaveGame(WorldContext, OutSlotName))
		{
			// The BP controller flow mints a blank BP_SaveGame into the GameInstance map
			// before the pawn is possessed, so booting straight into a level on a wiped
			// SaveGames folder resolves an in-memory save with nothing on disk behind it.
			// That object is a first-run save; seed it like one we minted ourselves. The
			// disk check is the guard: a slot the player has ever saved is never touched,
			// so this cannot stomp a deliberately-cleared keybind or spent points.
			if (!UGameplayStatics::DoesSaveGameExist(OutSlotName, 0))
			{
				InitializeFreshSave(Existing);
				const bool bSaved = UGameplayStatics::SaveGameToSlot(Existing, OutSlotName, 0);
				UE_LOG(LogNexusAbilityUI, Log,
					TEXT("ResolveOrCreateSaveGame: seeded unpersisted save (stats 10, points %.0f) in slot '%s' -> %s"),
					GFreshSaveStatPoints, *OutSlotName, bSaved ? TEXT("saved") : TEXT("SAVE FAILED"));
			}
			return Existing;
		}

		UClass* SaveClass = LoadSaveGameClass();
		if (!SaveClass)
		{
			UE_LOG(LogNexusAbilityUI, Warning,
				TEXT("ResolveOrCreateSaveGame: BP_SaveGame class failed to load, cannot create a save"));
			return nullptr;
		}

		USaveGame* Save = UGameplayStatics::CreateSaveGameObject(SaveClass);
		InitializeFreshSave(Save);

		OutSlotName = GDefaultSaveSlotName;
		const bool bSaved = UGameplayStatics::SaveGameToSlot(Save, OutSlotName, 0);
		UE_LOG(LogNexusAbilityUI, Log,
			TEXT("ResolveOrCreateSaveGame: created fresh save (stats 10, points %.0f) in slot '%s' -> %s"),
			GFreshSaveStatPoints, *OutSlotName, bSaved ? TEXT("saved") : TEXT("SAVE FAILED"));
		return Save;
	}

	// The UnlockedAbilities TArray<FString> variable added to BP_SaveGame.
	const FArrayProperty* FindUnlockedAbilitiesProp(const USaveGame* Save)
	{
		if (!Save)
		{
			return nullptr;
		}
		const FArrayProperty* ArrayProp = CastField<FArrayProperty>(
			FindPropByDisplayName(Save->GetClass(), TEXT("UnlockedAbilities")));
		return (ArrayProp && CastField<FStrProperty>(ArrayProp->Inner)) ? ArrayProp : nullptr;
	}

	void ReadUnlockedAbilities(const USaveGame* Save, TSet<FString>& OutUnlocked)
	{
		const FArrayProperty* ArrayProp = FindUnlockedAbilitiesProp(Save);
		if (!ArrayProp)
		{
			return;
		}
		const FStrProperty* Inner = CastField<FStrProperty>(ArrayProp->Inner);
		FScriptArrayHelper ArrayHelper(ArrayProp,
			ArrayProp->ContainerPtrToValuePtr<void>(const_cast<USaveGame*>(Save)));
		for (int32 Index = 0; Index < ArrayHelper.Num(); ++Index)
		{
			OutUnlocked.Add(Inner->GetPropertyValue(ArrayHelper.GetRawPtr(Index)));
		}
	}

	// Same stat -> attribute mapping W_StatRow uses for display; SavedField is the
	// matching S_PlayerTalentData member for the no-ASC (main menu) fallback; Abbrev
	// is the label used by the preview panel's requirement chips.
	struct FNexusStatBinding
	{
		FGameplayAttribute Attribute;
		const TCHAR* SavedField = nullptr;
		const TCHAR* Abbrev = nullptr;
	};

	bool ResolveStatBinding(int64 StatTypeRaw, FNexusStatBinding& Out)
	{
		switch (static_cast<ENexusStatType>(StatTypeRaw))
		{
		case ENexusStatType::Strength:     Out = { UBasicAttributeSet::GetStrengthAttribute(),     TEXT("Strength"),     TEXT("STR") }; return true;
		case ENexusStatType::Dexterity:    Out = { UBasicAttributeSet::GetDexterityAttribute(),    TEXT("Dexterity"),    TEXT("DEX") }; return true;
		case ENexusStatType::Intelligence: Out = { UBasicAttributeSet::GetIntelligenceAttribute(), TEXT("Intelligence"), TEXT("INT") }; return true;
		case ENexusStatType::Faith:        Out = { UBasicAttributeSet::GetFaithAttribute(),        TEXT("Faith"),        TEXT("FAI") }; return true;
		// S_PlayerTalentData stores these under the attribute-style names, not the stat names
		case ENexusStatType::Vitality:     Out = { UBasicAttributeSet::GetMaxHealthAttribute(),    TEXT("Health"),       TEXT("VIT") }; return true;
		case ENexusStatType::Endurance:    Out = { UBasicAttributeSet::GetMaxStaminaAttribute(),   TEXT("Stamina"),      TEXT("END") }; return true;
		case ENexusStatType::Mana:         Out = { UBasicAttributeSet::GetMaxManaAttribute(),      TEXT("Mana"),         TEXT("MANA") }; return true;
		default: return false;
		}
	}

	enum class ENexusReqEval : uint8 { Met, NotMet, Skipped };

	// One evaluated requirement: what it asks for, what the player has, and the verdict.
	// Skipped means the data was unreadable/unmapped and never gates an unlock.
	struct FNexusRequirementView
	{
		FString TypeName;
		const TCHAR* Abbrev = nullptr;
		int64 Raw = 0;
		double Required = 0.0;
		double Actual = 0.0;
		ENexusReqEval Result = ENexusReqEval::Skipped;
		FString Note;

		bool CountsAsMet() const { return Result != ENexusReqEval::NotMet; }
	};

	FNexusRequirementView EvaluateRequirement(const FNexusStatSource& Source,
		const UStruct* RequirementStruct, const void* RequirementPtr)
	{
		FNexusRequirementView View;

		const FProperty* StatTypeProp = FindPropByDisplayName(RequirementStruct, TEXT("StatType"));
		const FProperty* StatValueProp = FindPropByDisplayName(RequirementStruct, TEXT("StatValue"));
		if (!StatTypeProp || !StatValueProp)
		{
			View.Note = FString::Printf(TEXT("requirement struct %s is missing %s"),
				*RequirementStruct->GetName(), !StatTypeProp ? TEXT("StatType") : TEXT("StatValue"));
			return View;
		}

		const UEnum* StatTypeEnum = nullptr;
		if (const FEnumProperty* EnumProp = CastField<FEnumProperty>(StatTypeProp))
		{
			View.Raw = EnumProp->GetUnderlyingProperty()->GetSignedIntPropertyValue(
				EnumProp->ContainerPtrToValuePtr<void>(RequirementPtr));
			StatTypeEnum = EnumProp->GetEnum();
		}
		else if (const FByteProperty* ByteProp = CastField<FByteProperty>(StatTypeProp))
		{
			View.Raw = ByteProp->GetPropertyValue_InContainer(RequirementPtr);
			StatTypeEnum = ByteProp->Enum;
		}
		else
		{
			View.Note = FString::Printf(TEXT("StatType property is %s, not an enum/byte"),
				*StatTypeProp->GetClass()->GetName());
			return View;
		}

		if (const FNumericProperty* NumericProp = CastField<FNumericProperty>(StatValueProp))
		{
			const void* ValuePtr = NumericProp->ContainerPtrToValuePtr<void>(RequirementPtr);
			View.Required = NumericProp->IsFloatingPoint()
				? NumericProp->GetFloatingPointPropertyValue(ValuePtr)
				: static_cast<double>(NumericProp->GetSignedIntPropertyValue(ValuePtr));
		}

		View.TypeName = StatTypeEnum
			? StatTypeEnum->GetNameStringByValue(View.Raw)
			: FString::Printf(TEXT("<no enum, raw %lld>"), View.Raw);

		FNexusStatBinding Binding;
		if (!ResolveStatBinding(View.Raw, Binding))
		{
			View.Note = FString::Printf(TEXT("StatType=%s (raw %lld) has no attribute mapping"),
				*View.TypeName, View.Raw);
			return View;
		}
		View.Abbrev = Binding.Abbrev;

		if (Source.ASC)
		{
			if (!Source.ASC->HasAttributeSetForAttribute(Binding.Attribute))
			{
				View.Result = ENexusReqEval::NotMet;
				View.Note = FString::Printf(TEXT("ASC has no attribute set for %s"),
					*Binding.Attribute.GetName());
				return View;
			}
			View.Actual = Source.ASC->GetNumericAttribute(Binding.Attribute);
		}
		else
		{
			const FNumericProperty* StatProp = CastField<FNumericProperty>(
				FindPropByDisplayName(Source.SavedStatsStruct, Binding.SavedField));
			if (!StatProp)
			{
				View.Result = ENexusReqEval::NotMet;
				View.Note = FString::Printf(TEXT("saved talent data (%s) has no '%s' field"),
					Source.SavedStatsStruct ? *Source.SavedStatsStruct->GetName() : TEXT("null"),
					Binding.SavedField);
				return View;
			}
			const void* ValuePtr = StatProp->ContainerPtrToValuePtr<void>(Source.SavedStatsPtr);
			View.Actual = StatProp->IsFloatingPoint()
				? StatProp->GetFloatingPointPropertyValue(ValuePtr)
				: static_cast<double>(StatProp->GetSignedIntPropertyValue(ValuePtr));
		}

		View.Result = View.Actual >= View.Required ? ENexusReqEval::Met : ENexusReqEval::NotMet;
		return View;
	}

	bool IsSingleRequirementMet(const FNexusStatSource& Source, const UStruct* RequirementStruct,
		const void* RequirementPtr, const FString& SlotName)
	{
		const FNexusRequirementView View = EvaluateRequirement(Source, RequirementStruct, RequirementPtr);
		if (View.Result == ENexusReqEval::Skipped)
		{
			UE_LOG(LogNexusAbilityUI, Warning, TEXT("Slot %s: %s -> requirement skipped"),
				*SlotName, *View.Note);
			return true;
		}
		if (!View.Note.IsEmpty())
		{
			UE_LOG(LogNexusAbilityUI, Warning, TEXT("Slot %s: requirement %s >= %.0f -> %s -> NOT met"),
				*SlotName, *View.TypeName, View.Required, *View.Note);
			return false;
		}

		UE_LOG(LogNexusAbilityUI, Log,
			TEXT("Slot %s: requirement StatType=%s (raw %lld), required %.0f, %s value %.2f -> %s"),
			*SlotName, *View.TypeName, View.Raw, View.Required,
			Source.ASC ? TEXT("ASC") : TEXT("saved"), View.Actual,
			View.Result == ENexusReqEval::Met ? TEXT("MET") : TEXT("NOT met"));
		return View.Result == ENexusReqEval::Met;
	}

	bool AreRequirementsMet(const FNexusStatSource& Source, UClass* AbilityDataClass, const FString& SlotName)
	{
		if (!Source.IsValid() || !AbilityDataClass)
		{
			UE_LOG(LogNexusAbilityUI, Warning, TEXT("Slot %s: %s -> requirements NOT met"),
				*SlotName, !Source.IsValid() ? TEXT("no stat source (no ASC, no save)") : TEXT("AbilityData class is null"));
			return false;
		}

		const UObject* DataCDO = AbilityDataClass->GetDefaultObject();
		if (!DataCDO)
		{
			UE_LOG(LogNexusAbilityUI, Warning, TEXT("Slot %s: %s has no CDO"),
				*SlotName, *AbilityDataClass->GetName());
			return false;
		}

		const FStructProperty* AbilityDataProp = CastField<FStructProperty>(
			FindPropByDisplayName(AbilityDataClass, TEXT("AbilityData")));
		if (!AbilityDataProp)
		{
			UE_LOG(LogNexusAbilityUI, Warning,
				TEXT("Slot %s: no AbilityData struct property found on %s"),
				*SlotName, *AbilityDataClass->GetName());
			return false;
		}
		const void* AbilityDataPtr = AbilityDataProp->ContainerPtrToValuePtr<void>(DataCDO);

		const FProperty* RequirementsProp = FindPropByDisplayName(AbilityDataProp->Struct, TEXT("Requirements"));
		if (!RequirementsProp)
		{
			// No requirements authored -> nothing blocks the unlock.
			UE_LOG(LogNexusAbilityUI, Log,
				TEXT("Slot %s: no Requirements property in %s -> treated as unlocked"),
				*SlotName, *AbilityDataProp->Struct->GetName());
			return true;
		}

		if (const FArrayProperty* RequirementsArray = CastField<FArrayProperty>(RequirementsProp))
		{
			const FStructProperty* ElementProp = CastField<FStructProperty>(RequirementsArray->Inner);
			if (!ElementProp)
			{
				UE_LOG(LogNexusAbilityUI, Warning,
					TEXT("Slot %s: Requirements array element is %s, not a struct -> treated as unlocked"),
					*SlotName, *RequirementsArray->Inner->GetClass()->GetName());
				return true;
			}

			FScriptArrayHelper ArrayHelper(RequirementsArray, RequirementsArray->ContainerPtrToValuePtr<void>(AbilityDataPtr));
			UE_LOG(LogNexusAbilityUI, Log, TEXT("Slot %s: %d requirement(s) in array"),
				*SlotName, ArrayHelper.Num());
			for (int32 Index = 0; Index < ArrayHelper.Num(); ++Index)
			{
				if (!IsSingleRequirementMet(Source, ElementProp->Struct, ArrayHelper.GetRawPtr(Index), SlotName))
				{
					return false;
				}
			}
			return true;
		}

		if (const FStructProperty* SingleRequirement = CastField<FStructProperty>(RequirementsProp))
		{
			return IsSingleRequirementMet(Source, SingleRequirement->Struct,
				SingleRequirement->ContainerPtrToValuePtr<void>(AbilityDataPtr), SlotName);
		}

		UE_LOG(LogNexusAbilityUI, Warning,
			TEXT("Slot %s: Requirements property is %s, neither array nor struct -> treated as unlocked"),
			*SlotName, *RequirementsProp->GetClass()->GetName());
		return true;
	}

	// Requirement views for the preview panel; same traversal AreRequirementsMet uses.
	void CollectRequirementViews(const FNexusStatSource& Source, UClass* AbilityDataClass,
		TArray<FNexusRequirementView>& OutViews)
	{
		OutViews.Reset();
		if (!AbilityDataClass)
		{
			return;
		}
		const UObject* DataCDO = AbilityDataClass->GetDefaultObject();
		const FStructProperty* AbilityDataProp = CastField<FStructProperty>(
			FindPropByDisplayName(AbilityDataClass, TEXT("AbilityData")));
		if (!DataCDO || !AbilityDataProp)
		{
			return;
		}
		const void* AbilityDataPtr = AbilityDataProp->ContainerPtrToValuePtr<void>(DataCDO);

		const FProperty* RequirementsProp = FindPropByDisplayName(AbilityDataProp->Struct, TEXT("Requirements"));
		if (const FArrayProperty* RequirementsArray = CastField<FArrayProperty>(RequirementsProp))
		{
			if (const FStructProperty* ElementProp = CastField<FStructProperty>(RequirementsArray->Inner))
			{
				FScriptArrayHelper ArrayHelper(RequirementsArray,
					RequirementsArray->ContainerPtrToValuePtr<void>(AbilityDataPtr));
				for (int32 Index = 0; Index < ArrayHelper.Num(); ++Index)
				{
					OutViews.Add(EvaluateRequirement(Source, ElementProp->Struct, ArrayHelper.GetRawPtr(Index)));
				}
			}
		}
		else if (const FStructProperty* SingleRequirement = CastField<FStructProperty>(RequirementsProp))
		{
			OutViews.Add(EvaluateRequirement(Source, SingleRequirement->Struct,
				SingleRequirement->ContainerPtrToValuePtr<void>(AbilityDataPtr)));
		}
	}

	// The stat source CheckRequirements uses: live ASC in-game, saved talent data in
	// the menu. Returns the resolved save so callers can also read UnlockedAbilities.
	USaveGame* BuildStatSource(const UObject* WorldContext, FNexusStatSource& OutSource)
	{
		const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(WorldContext, 0);
		const IAbilitySystemInterface* AscInterface = Cast<IAbilitySystemInterface>(PlayerPawn);
		OutSource.ASC = AscInterface ? AscInterface->GetAbilitySystemComponent() : nullptr;

		FString SlotNameUnused;
		USaveGame* Save = ResolveSaveGame(WorldContext, SlotNameUnused);
		if (!OutSource.ASC)
		{
			GetSavedTalentData(Save, OutSource.SavedStatsStruct, OutSource.SavedStatsPtr);
		}
		return Save;
	}

	void UnlockSlot(UUserWidget* SlotWidget)
	{
		// The Blueprint variable was authored as "bIsUnlocked " (trailing space),
		// so compare trimmed names instead of using FindFProperty.
		for (TFieldIterator<FBoolProperty> It(SlotWidget->GetClass()); It; ++It)
		{
			if (It->GetName().TrimStartAndEnd() == TEXT("bIsUnlocked"))
			{
				It->SetPropertyValue_InContainer(SlotWidget, true);
				break;
			}
		}

		if (UWidget* LockOverlay = SlotWidget->GetWidgetFromName(TEXT("LockOverlay")))
		{
			LockOverlay->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void UNexusAbilityUILibrary::CheckRequirements(UUserWidget* AbilitiesScreen)
{
	if (!AbilitiesScreen || !AbilitiesScreen->WidgetTree)
	{
		UE_LOG(LogNexusAbilityUI, Warning, TEXT("CheckRequirements: AbilitiesScreen is %s"),
			AbilitiesScreen ? TEXT("valid but has no WidgetTree") : TEXT("null"));
		return;
	}

	FNexusStatSource StatSource;
	const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(AbilitiesScreen, 0);
	const IAbilitySystemInterface* AscInterface = Cast<IAbilitySystemInterface>(PlayerPawn);
	StatSource.ASC = AscInterface ? AscInterface->GetAbilitySystemComponent() : nullptr;

	FString SlotNameUnused;
	const USaveGame* SaveGame = ResolveSaveGame(AbilitiesScreen, SlotNameUnused);
	TSet<FString> UnlockedAbilities;
	ReadUnlockedAbilities(SaveGame, UnlockedAbilities);

	if (StatSource.ASC)
	{
		UE_LOG(LogNexusAbilityUI, Log, TEXT("CheckRequirements: PlayerPawn=%s, ASC=%s (owner %s)"),
			*PlayerPawn->GetName(), *StatSource.ASC->GetName(),
			StatSource.ASC->GetOwner() ? *StatSource.ASC->GetOwner()->GetName() : TEXT("null"));
	}
	else
	{
		// Main menu path: no pawn/ASC exists, fall back to the saved talent data.
		GetSavedTalentData(SaveGame, StatSource.SavedStatsStruct, StatSource.SavedStatsPtr);
		UE_LOG(LogNexusAbilityUI, Log,
			TEXT("CheckRequirements: no ASC (PlayerPawn=%s), save game %s, saved talent data %s"),
			PlayerPawn ? *PlayerPawn->GetName() : TEXT("null"),
			SaveGame ? TEXT("found") : TEXT("NOT found"),
			StatSource.SavedStatsPtr ? TEXT("found") : TEXT("NOT found"));
	}
	UE_LOG(LogNexusAbilityUI, Log, TEXT("CheckRequirements: %d ability(ies) in saved UnlockedAbilities"),
		UnlockedAbilities.Num());

	UClass* SlotClass = LoadClass<UUserWidget>(nullptr, TEXT("/Game/Widgets/W_AbilitySlot.W_AbilitySlot_C"));
	if (!SlotClass)
	{
		UE_LOG(LogNexusAbilityUI, Warning,
			TEXT("CheckRequirements: failed to load /Game/Widgets/W_AbilitySlot.W_AbilitySlot_C"));
		return;
	}

	int32 WidgetsVisited = 0;
	int32 SlotsFound = 0;
	AbilitiesScreen->WidgetTree->ForEachWidget([&](UWidget* Widget)
	{
		++WidgetsVisited;
		UUserWidget* SlotWidget = Cast<UUserWidget>(Widget);
		if (!SlotWidget || !SlotWidget->GetClass()->IsChildOf(SlotClass))
		{
			return;
		}
		++SlotsFound;

		const FProperty* RawProp = FindPropByDisplayName(SlotWidget->GetClass(), TEXT("AbilityData"));
		const FClassProperty* AbilityDataProp = CastField<FClassProperty>(RawProp);
		UClass* AbilityDataClass = AbilityDataProp
			? Cast<UClass>(AbilityDataProp->GetObjectPropertyValue_InContainer(SlotWidget))
			: nullptr;

		if (!RawProp)
		{
			UE_LOG(LogNexusAbilityUI, Warning, TEXT("Slot %s: no AbilityData property found on %s"),
				*SlotWidget->GetName(), *SlotWidget->GetClass()->GetName());
		}
		else if (!AbilityDataProp)
		{
			UE_LOG(LogNexusAbilityUI, Warning,
				TEXT("Slot %s: AbilityData property is %s (%s), not a class reference (FClassProperty)"),
				*SlotWidget->GetName(), *RawProp->GetClass()->GetName(), *RawProp->GetCPPType());
		}
		else
		{
			UE_LOG(LogNexusAbilityUI, Log, TEXT("Slot %s: AbilityData class = %s"),
				*SlotWidget->GetName(),
				AbilityDataClass ? *AbilityDataClass->GetName() : TEXT("null (not assigned)"));
		}

		// Already-unlocked abilities stay unlocked regardless of current stats.
		if (AbilityDataClass && UnlockedAbilities.Contains(AbilityDataClass->GetName()))
		{
			UE_LOG(LogNexusAbilityUI, Log, TEXT("Slot %s: '%s' is in saved UnlockedAbilities -> unlocking"),
				*SlotWidget->GetName(), *AbilityDataClass->GetName());
			UnlockSlot(SlotWidget);
		}
		else if (AreRequirementsMet(StatSource, AbilityDataClass, SlotWidget->GetName()))
		{
			UE_LOG(LogNexusAbilityUI, Log, TEXT("Slot %s: all requirements met -> unlocking"),
				*SlotWidget->GetName());
			UnlockSlot(SlotWidget);
		}
		else
		{
			UE_LOG(LogNexusAbilityUI, Log, TEXT("Slot %s: stays locked"), *SlotWidget->GetName());
		}
	});

	UE_LOG(LogNexusAbilityUI, Log,
		TEXT("CheckRequirements: visited %d widgets in tree, found %d W_AbilitySlot widgets"),
		WidgetsVisited, SlotsFound);
	if (SlotsFound == 0)
	{
		UE_LOG(LogNexusAbilityUI, Warning,
			TEXT("CheckRequirements: no W_AbilitySlot widgets found in %s's widget tree. ")
			TEXT("Screen class=%s, root=%s, expected slot class=%s"),
			*AbilitiesScreen->GetName(), *AbilitiesScreen->GetClass()->GetName(),
			AbilitiesScreen->WidgetTree->RootWidget ? *AbilitiesScreen->WidgetTree->RootWidget->GetName() : TEXT("null"),
			*SlotClass->GetName());
	}
}

bool UNexusAbilityUILibrary::UnlockAbility(const UObject* WorldContextObject, TSubclassOf<UObject> AbilityDataClass)
{
	if (!AbilityDataClass)
	{
		UE_LOG(LogNexusAbilityUI, Warning, TEXT("UnlockAbility: AbilityDataClass is null"));
		return false;
	}

	FString SlotName;
	USaveGame* SaveGame = ResolveOrCreateSaveGame(WorldContextObject, SlotName);
	if (!SaveGame)
	{
		return false;
	}

	const FArrayProperty* ArrayProp = FindUnlockedAbilitiesProp(SaveGame);
	if (!ArrayProp)
	{
		UE_LOG(LogNexusAbilityUI, Warning,
			TEXT("UnlockAbility: %s has no UnlockedAbilities TArray<FString> variable"),
			*SaveGame->GetClass()->GetName());
		return false;
	}

	const FString AbilityKey = AbilityDataClass->GetName();
	const FStrProperty* Inner = CastField<FStrProperty>(ArrayProp->Inner);
	FScriptArrayHelper ArrayHelper(ArrayProp, ArrayProp->ContainerPtrToValuePtr<void>(SaveGame));
	for (int32 Index = 0; Index < ArrayHelper.Num(); ++Index)
	{
		if (Inner->GetPropertyValue(ArrayHelper.GetRawPtr(Index)) == AbilityKey)
		{
			UE_LOG(LogNexusAbilityUI, Log, TEXT("UnlockAbility: '%s' already unlocked in slot '%s'"),
				*AbilityKey, *SlotName);
			return true;
		}
	}
	const int32 NewIndex = ArrayHelper.AddValue();
	Inner->SetPropertyValue(ArrayHelper.GetRawPtr(NewIndex), AbilityKey);

	const bool bSaved = UGameplayStatics::SaveGameToSlot(SaveGame, SlotName, 0);
	UE_LOG(LogNexusAbilityUI, Log, TEXT("UnlockAbility: '%s' added to slot '%s', SaveGameToSlot -> %s"),
		*AbilityKey, *SlotName, bSaved ? TEXT("success") : TEXT("FAILED"));
	return bSaved;
}

bool UNexusAbilityUILibrary::IsAbilityUnlocked(const UObject* WorldContextObject, TSubclassOf<UObject> AbilityDataClass)
{
	if (!AbilityDataClass)
	{
		return false;
	}
	FString SlotNameUnused;
	const USaveGame* SaveGame = ResolveSaveGame(WorldContextObject, SlotNameUnused);
	TSet<FString> UnlockedAbilities;
	ReadUnlockedAbilities(SaveGame, UnlockedAbilities);
	return UnlockedAbilities.Contains(AbilityDataClass->GetName());
}

float UNexusAbilityUILibrary::GetSavedStat(const UObject* WorldContextObject, FName StatName)
{
	FString SlotNameUnused;
	const USaveGame* SaveGame = ResolveOrCreateSaveGame(WorldContextObject, SlotNameUnused);
	const UStruct* TalentStruct = nullptr;
	const void* TalentPtr = nullptr;
	GetSavedTalentData(SaveGame, TalentStruct, TalentPtr);
	if (!TalentStruct || !TalentPtr)
	{
		UE_LOG(LogNexusAbilityUI, Warning, TEXT("GetSavedStat(%s): no save/talent data available"),
			*StatName.ToString());
		return 0.0f;
	}

	double Value = 0.0;
	if (!ReadNumericField(TalentStruct, TalentPtr, *StatFieldName(StatName), Value))
	{
		UE_LOG(LogNexusAbilityUI, Warning, TEXT("GetSavedStat(%s): %s has no '%s' field"),
			*StatName.ToString(), *TalentStruct->GetName(), *StatFieldName(StatName));
	}
	return static_cast<float>(Value);
}

bool UNexusAbilityUILibrary::UpgradeSavedStat(const UObject* WorldContextObject, FName StatName, int32 Amount)
{
	if (Amount <= 0)
	{
		UE_LOG(LogNexusAbilityUI, Warning, TEXT("UpgradeSavedStat(%s): Amount %d must be positive"),
			*StatName.ToString(), Amount);
		return false;
	}

	FString SlotName;
	USaveGame* SaveGame = ResolveOrCreateSaveGame(WorldContextObject, SlotName);
	const UStruct* TalentStruct = nullptr;
	void* TalentPtr = nullptr;
	GetSavedTalentDataMutable(SaveGame, TalentStruct, TalentPtr);
	if (!TalentStruct || !TalentPtr)
	{
		UE_LOG(LogNexusAbilityUI, Warning, TEXT("UpgradeSavedStat(%s): no save/talent data available"),
			*StatName.ToString());
		return false;
	}

	const TCHAR* PointsField = ResolvePointsFieldName(TalentStruct);
	if (!PointsField)
	{
		UE_LOG(LogNexusAbilityUI, Warning,
			TEXT("UpgradeSavedStat(%s): %s has neither a StatPoints nor a TalentPoints field"),
			*StatName.ToString(), *TalentStruct->GetName());
		return false;
	}

	double Points = 0.0;
	ReadNumericField(TalentStruct, TalentPtr, PointsField, Points);
	if (Points < Amount)
	{
		UE_LOG(LogNexusAbilityUI, Log, TEXT("UpgradeSavedStat(%s): not enough points (%s=%.0f, need %d)"),
			*StatName.ToString(), PointsField, Points, Amount);
		return false;
	}

	const FString Field = StatFieldName(StatName);
	double Current = 0.0;
	if (!ReadNumericField(TalentStruct, TalentPtr, *Field, Current))
	{
		UE_LOG(LogNexusAbilityUI, Warning, TEXT("UpgradeSavedStat(%s): %s has no '%s' field"),
			*StatName.ToString(), *TalentStruct->GetName(), *Field);
		return false;
	}

	WriteNumericField(TalentStruct, TalentPtr, *Field, Current + Amount);
	WriteNumericField(TalentStruct, TalentPtr, PointsField, Points - Amount);

	const bool bSaved = UGameplayStatics::SaveGameToSlot(SaveGame, SlotName, 0);
	UE_LOG(LogNexusAbilityUI, Log,
		TEXT("UpgradeSavedStat: %s %.0f -> %.0f, %s %.0f -> %.0f, slot '%s' -> %s"),
		*Field, Current, Current + Amount, PointsField, Points, Points - Amount,
		*SlotName, bSaved ? TEXT("saved") : TEXT("SAVE FAILED"));
	return bSaved;
}

int32 UNexusAbilityUILibrary::GetSavedStatPoints(const UObject* WorldContextObject)
{
	FString SlotNameUnused;
	const USaveGame* SaveGame = ResolveOrCreateSaveGame(WorldContextObject, SlotNameUnused);
	const UStruct* TalentStruct = nullptr;
	const void* TalentPtr = nullptr;
	GetSavedTalentData(SaveGame, TalentStruct, TalentPtr);
	if (!TalentStruct || !TalentPtr)
	{
		return 0;
	}

	const TCHAR* PointsField = ResolvePointsFieldName(TalentStruct);
	double Points = 0.0;
	if (PointsField)
	{
		ReadNumericField(TalentStruct, TalentPtr, PointsField, Points);
	}
	return static_cast<int32>(Points);
}

void UNexusAbilityUILibrary::SetupStatsUI(UUserWidget* AbilitiesScreen)
{
	if (!AbilitiesScreen || !AbilitiesScreen->WidgetTree)
	{
		UE_LOG(LogNexusAbilityUI, Warning, TEXT("SetupStatsUI: AbilitiesScreen is %s"),
			AbilitiesScreen ? TEXT("valid but has no WidgetTree") : TEXT("null"));
		return;
	}

	UVerticalBox* Container = Cast<UVerticalBox>(AbilitiesScreen->GetWidgetFromName(TEXT("StatsContainer")));
	if (!Container)
	{
		FString BoxNames;
		AbilitiesScreen->WidgetTree->ForEachWidget([&BoxNames](UWidget* Widget)
		{
			if (Widget->IsA<UVerticalBox>())
			{
				BoxNames += Widget->GetName() + TEXT(" ");
			}
		});
		UE_LOG(LogNexusAbilityUI, Warning,
			TEXT("SetupStatsUI: %s has no VerticalBox named 'StatsContainer'. ")
			TEXT("Rename the target VerticalBox in the Designer. VerticalBoxes in tree: %s"),
			*AbilitiesScreen->GetName(), BoxNames.IsEmpty() ? TEXT("<none>") : *BoxNames);
		return;
	}

	Container->ClearChildren();

	UNexusStatsPanel* Panel = CreateWidget<UNexusStatsPanel>(AbilitiesScreen, UNexusStatsPanel::StaticClass());
	UVerticalBoxSlot* PanelSlot = Container->AddChildToVerticalBox(Panel);
	PanelSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	Panel->InitPanel(AbilitiesScreen);

	UE_LOG(LogNexusAbilityUI, Log, TEXT("SetupStatsUI: built stats panel in %s's StatsContainer"),
		*AbilitiesScreen->GetName());
}

namespace
{
	// Dark fantasy palette: near-black warm browns, parchment text, tarnished gold accents.
	const FLinearColor GRowBackground(0.030f, 0.024f, 0.018f, 0.85f);
	const FLinearColor GParchmentText(0.83f, 0.78f, 0.66f, 1.0f);
	const FLinearColor GAccentGold(0.85f, 0.66f, 0.28f, 1.0f);
	const FLinearColor GButtonTint(0.10f, 0.08f, 0.06f, 1.0f);
	const FLinearColor GSaveButtonTint(0.30f, 0.22f, 0.08f, 1.0f);

	UButton* MakeStyledButton(UWidgetTree* Tree, const TCHAR* Label, const FLinearColor& Tint,
		UTextBlock** OutLabel = nullptr)
	{
		UButton* Button = Tree->ConstructWidget<UButton>();
		Button->SetBackgroundColor(Tint);

		UTextBlock* Text = Tree->ConstructWidget<UTextBlock>();
		Text->SetText(FText::FromString(Label));
		Text->SetColorAndOpacity(FSlateColor(GParchmentText));
		FSlateFontInfo Font = Text->GetFont();
		Font.Size = 12;
		Text->SetFont(Font);
		Button->AddChild(Text);

		if (OutLabel)
		{
			*OutLabel = Text;
		}
		return Button;
	}
}

void UNexusStatRow::InitRow(FName InStatName, UNexusStatsPanel* InPanel)
{
	StatName = InStatName;
	Panel = InPanel;
	RefreshTexts();
}

void UNexusStatRow::SetSavedValue(int32 InSavedValue)
{
	SavedValue = InSavedValue;
	PendingDelta = 0;
}

TSharedRef<SWidget> UNexusStatRow::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		UBorder* Background = WidgetTree->ConstructWidget<UBorder>();
		Background->SetBrushColor(GRowBackground);
		Background->SetPadding(FMargin(10.0f, 5.0f));
		WidgetTree->RootWidget = Background;

		UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
		Background->SetContent(Row);

		NameText = WidgetTree->ConstructWidget<UTextBlock>();
		NameText->SetColorAndOpacity(FSlateColor(GParchmentText));
		UHorizontalBoxSlot* NameSlot = Row->AddChildToHorizontalBox(NameText);
		NameSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		NameSlot->SetVerticalAlignment(VAlign_Center);

		ValueText = WidgetTree->ConstructWidget<UTextBlock>();
		UHorizontalBoxSlot* ValueSlot = Row->AddChildToHorizontalBox(ValueText);
		ValueSlot->SetPadding(FMargin(8.0f, 0.0f));
		ValueSlot->SetVerticalAlignment(VAlign_Center);

		MinusButton = MakeStyledButton(WidgetTree, TEXT("-"), GButtonTint);
		MinusButton->OnClicked.AddDynamic(this, &UNexusStatRow::HandleMinusClicked);
		UHorizontalBoxSlot* MinusSlot = Row->AddChildToHorizontalBox(MinusButton);
		MinusSlot->SetPadding(FMargin(2.0f, 0.0f));
		MinusSlot->SetVerticalAlignment(VAlign_Center);

		PlusButton = MakeStyledButton(WidgetTree, TEXT("+"), GButtonTint);
		PlusButton->OnClicked.AddDynamic(this, &UNexusStatRow::HandlePlusClicked);
		UHorizontalBoxSlot* PlusSlot = Row->AddChildToHorizontalBox(PlusButton);
		PlusSlot->SetPadding(FMargin(2.0f, 0.0f));
		PlusSlot->SetVerticalAlignment(VAlign_Center);

		RefreshTexts();
	}
	return Super::RebuildWidget();
}

void UNexusStatRow::HandlePlusClicked()
{
	UNexusStatsPanel* OwningPanel = Panel.Get();
	if (OwningPanel && OwningPanel->GetRemainingPoints() > 0)
	{
		++PendingDelta;
		OwningPanel->NotifyPendingChanged();
	}
}

void UNexusStatRow::HandleMinusClicked()
{
	// Pending only: never drops below the value already persisted in the save.
	if (PendingDelta > 0)
	{
		--PendingDelta;
		if (UNexusStatsPanel* OwningPanel = Panel.Get())
		{
			OwningPanel->NotifyPendingChanged();
		}
	}
}

void UNexusStatRow::RefreshTexts()
{
	if (NameText)
	{
		NameText->SetText(FText::FromName(StatName));
	}
	if (ValueText)
	{
		if (PendingDelta > 0)
		{
			// \u2192 is a right arrow; escaped so the file stays ASCII-safe for MSVC.
			ValueText->SetText(FText::FromString(FString::Printf(
				TEXT("%d \u2192 %d"), SavedValue, SavedValue + PendingDelta)));
			ValueText->SetColorAndOpacity(FSlateColor(GAccentGold));
		}
		else
		{
			ValueText->SetText(FText::AsNumber(SavedValue));
			ValueText->SetColorAndOpacity(FSlateColor(GParchmentText));
		}
	}

	const UNexusStatsPanel* OwningPanel = Panel.Get();
	if (PlusButton)
	{
		PlusButton->SetIsEnabled(OwningPanel && OwningPanel->GetRemainingPoints() > 0);
	}
	if (MinusButton)
	{
		MinusButton->SetIsEnabled(PendingDelta > 0);
	}
}

void UNexusStatsPanel::InitPanel(UUserWidget* InAbilitiesScreen)
{
	AbilitiesScreen = InAbilitiesScreen;
	RefreshFromSave();
}

int32 UNexusStatsPanel::GetRemainingPoints() const
{
	return SavedPoints - GetTotalPendingDelta();
}

int32 UNexusStatsPanel::GetTotalPendingDelta() const
{
	int32 Total = 0;
	for (const UNexusStatRow* Row : Rows)
	{
		if (Row)
		{
			Total += Row->GetPendingDelta();
		}
	}
	return Total;
}

void UNexusStatsPanel::NotifyPendingChanged()
{
	if (PointsText)
	{
		PointsText->SetText(FText::Format(
			NSLOCTEXT("Nexus", "StatPointsReadout", "Stat Points: {0}"), GetRemainingPoints()));
	}
	for (UNexusStatRow* Row : Rows)
	{
		if (Row)
		{
			Row->RefreshTexts();
		}
	}

	const bool bHasPending = GetTotalPendingDelta() > 0;
	if (SaveButton)
	{
		SaveButton->SetIsEnabled(bHasPending);
	}
	if (CancelButton)
	{
		CancelButton->SetIsEnabled(bHasPending);
	}
}

TSharedRef<SWidget> UNexusStatsPanel::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		UVerticalBox* Root = WidgetTree->ConstructWidget<UVerticalBox>();
		WidgetTree->RootWidget = Root;

		PointsText = WidgetTree->ConstructWidget<UTextBlock>();
		PointsText->SetColorAndOpacity(FSlateColor(GAccentGold));
		FSlateFontInfo PointsFont = PointsText->GetFont();
		PointsFont.Size = 16;
		PointsText->SetFont(PointsFont);
		UVerticalBoxSlot* PointsSlot = Root->AddChildToVerticalBox(PointsText);
		PointsSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 10.0f));

		static const FName StatNames[] =
		{
			TEXT("Strength"), TEXT("Dexterity"), TEXT("Intelligence"), TEXT("Faith"),
			TEXT("Vitality"), TEXT("Endurance"), TEXT("Mana")
		};
		Rows.Reset();
		for (const FName& RowStatName : StatNames)
		{
			UNexusStatRow* Row = CreateWidget<UNexusStatRow>(this, UNexusStatRow::StaticClass());
			Row->InitRow(RowStatName, this);
			UVerticalBoxSlot* RowSlot = Root->AddChildToVerticalBox(Row);
			RowSlot->SetPadding(FMargin(0.0f, 2.0f));
			Rows.Add(Row);
		}

		UHorizontalBox* ButtonRow = WidgetTree->ConstructWidget<UHorizontalBox>();
		UVerticalBoxSlot* ButtonRowSlot = Root->AddChildToVerticalBox(ButtonRow);
		ButtonRowSlot->SetPadding(FMargin(0.0f, 12.0f, 0.0f, 0.0f));

		SaveButton = MakeStyledButton(WidgetTree, TEXT("SAVE"), GSaveButtonTint);
		SaveButton->OnClicked.AddDynamic(this, &UNexusStatsPanel::HandleSaveClicked);
		UHorizontalBoxSlot* SaveSlot = ButtonRow->AddChildToHorizontalBox(SaveButton);
		SaveSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		SaveSlot->SetPadding(FMargin(0.0f, 0.0f, 4.0f, 0.0f));
		SaveSlot->SetHorizontalAlignment(HAlign_Fill);

		CancelButton = MakeStyledButton(WidgetTree, TEXT("CANCEL"), GButtonTint);
		CancelButton->OnClicked.AddDynamic(this, &UNexusStatsPanel::HandleCancelClicked);
		UHorizontalBoxSlot* CancelSlot = ButtonRow->AddChildToHorizontalBox(CancelButton);
		CancelSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		CancelSlot->SetPadding(FMargin(4.0f, 0.0f, 0.0f, 0.0f));
		CancelSlot->SetHorizontalAlignment(HAlign_Fill);

		NotifyPendingChanged();
	}
	return Super::RebuildWidget();
}

void UNexusStatsPanel::HandleSaveClicked()
{
	const int32 TotalDelta = GetTotalPendingDelta();
	if (TotalDelta <= 0)
	{
		return;
	}

	FString SlotName;
	USaveGame* SaveGame = ResolveOrCreateSaveGame(this, SlotName);
	const UStruct* TalentStruct = nullptr;
	void* TalentPtr = nullptr;
	GetSavedTalentDataMutable(SaveGame, TalentStruct, TalentPtr);
	if (!TalentStruct || !TalentPtr)
	{
		UE_LOG(LogNexusAbilityUI, Warning, TEXT("StatsPanel Save: no save/talent data available"));
		return;
	}

	const TCHAR* PointsField = ResolvePointsFieldName(TalentStruct);
	double Points = 0.0;
	if (!PointsField || !ReadNumericField(TalentStruct, TalentPtr, PointsField, Points))
	{
		UE_LOG(LogNexusAbilityUI, Warning,
			TEXT("StatsPanel Save: %s has neither a StatPoints nor a TalentPoints field"),
			*TalentStruct->GetName());
		return;
	}
	if (Points < TotalDelta)
	{
		// Save changed underneath the pending edits (e.g. spent in-game); resync instead.
		UE_LOG(LogNexusAbilityUI, Warning,
			TEXT("StatsPanel Save: %d pending point(s) but only %.0f in save -> discarding pending"),
			TotalDelta, Points);
		RefreshFromSave();
		return;
	}

	for (UNexusStatRow* Row : Rows)
	{
		if (!Row || Row->GetPendingDelta() <= 0)
		{
			continue;
		}
		const FString Field = StatFieldName(Row->GetStatName());
		double Current = 0.0;
		if (!ReadNumericField(TalentStruct, TalentPtr, *Field, Current))
		{
			UE_LOG(LogNexusAbilityUI, Warning, TEXT("StatsPanel Save: %s has no '%s' field, skipped"),
				*TalentStruct->GetName(), *Field);
			continue;
		}
		WriteNumericField(TalentStruct, TalentPtr, *Field, Current + Row->GetPendingDelta());
		UE_LOG(LogNexusAbilityUI, Log, TEXT("StatsPanel Save: %s %.0f -> %.0f"),
			*Field, Current, Current + Row->GetPendingDelta());
	}
	WriteNumericField(TalentStruct, TalentPtr, PointsField, Points - TotalDelta);

	const bool bSaved = UGameplayStatics::SaveGameToSlot(SaveGame, SlotName, 0);
	UE_LOG(LogNexusAbilityUI, Log, TEXT("StatsPanel Save: %s %.0f -> %.0f, slot '%s' -> %s"),
		PointsField, Points, Points - TotalDelta, *SlotName,
		bSaved ? TEXT("saved") : TEXT("SAVE FAILED"));

	RefreshFromSave();

	// Refresh ability locks so newly met requirements unlock slots live, and the
	// preview panel so its requirement colors and Unlock button follow the new stats.
	if (UUserWidget* Screen = AbilitiesScreen.Get())
	{
		UNexusAbilityUILibrary::CheckRequirements(Screen);
		UNexusAbilityUILibrary::SetupAbilityPreview(Screen);
	}
}

void UNexusStatsPanel::HandleCancelClicked()
{
	for (UNexusStatRow* Row : Rows)
	{
		if (Row)
		{
			Row->ClearPending();
		}
	}
	NotifyPendingChanged();
}

void UNexusStatsPanel::RefreshFromSave()
{
	SavedPoints = UNexusAbilityUILibrary::GetSavedStatPoints(this);
	for (UNexusStatRow* Row : Rows)
	{
		if (Row)
		{
			Row->SetSavedValue(FMath::RoundToInt32(
				UNexusAbilityUILibrary::GetSavedStat(this, Row->GetStatName())));
		}
	}
	NotifyPendingChanged();
}

namespace
{
	const FLinearColor GReqMetGreen(0.25f, 0.72f, 0.30f, 1.0f);
	const FLinearColor GReqUnmetRed(0.80f, 0.22f, 0.16f, 1.0f);
	const FLinearColor GDisabledGrey(0.22f, 0.22f, 0.22f, 1.0f);

	// The requested C9A55CFF, converted at call time (FromSRGBColor's lookup table is
	// not safe to use during static initialization).
	FLinearColor UnlockGold()
	{
		return FLinearColor::FromSRGBColor(FColor(0xC9, 0xA5, 0x5C, 0xFF));
	}

	/**
	 * Keeps preview controllers alive and maps each abilities screen to its controller.
	 * Needed because dispatcher/button bindings hold only weak references to the bound
	 * object; stale entries (dead screens) are pruned on every SetupAbilityPreview call.
	 */
	class FNexusPreviewControllerRegistry final : public FGCObject
	{
	public:
		static FNexusPreviewControllerRegistry& Get()
		{
			static FNexusPreviewControllerRegistry Instance;
			return Instance;
		}

		UNexusAbilityPreviewController* Find(const UUserWidget* InScreen) const
		{
			const TObjectPtr<UNexusAbilityPreviewController>* Found = Controllers.Find(FObjectKey(InScreen));
			return Found ? Found->Get() : nullptr;
		}

		void Add(const UUserWidget* InScreen, UNexusAbilityPreviewController* Controller)
		{
			Controllers.Add(FObjectKey(InScreen), Controller);
		}

		void Prune()
		{
			for (auto It = Controllers.CreateIterator(); It; ++It)
			{
				if (!It->Value || !It->Value->GetScreen())
				{
					It.RemoveCurrent();
				}
			}
		}

		virtual void AddReferencedObjects(FReferenceCollector& Collector) override
		{
			for (auto& Pair : Controllers)
			{
				Collector.AddReferencedObject(Pair.Value);
			}
		}

		virtual FString GetReferencerName() const override
		{
			return TEXT("FNexusPreviewControllerRegistry");
		}

	private:
		TMap<FObjectKey, TObjectPtr<UNexusAbilityPreviewController>> Controllers;
	};
}

void UNexusAbilitySlotRelay::InitRelay(UUserWidget* InSlotWidget, UNexusAbilityPreviewController* InController)
{
	SlotWidget = InSlotWidget;
	Controller = InController;
}

void UNexusAbilitySlotRelay::HandleSlotClicked(UObject* /*ClickedAbilityData*/)
{
	// Payload is always null (see header); the slot's AbilityData variable is authoritative.
	if (UNexusAbilityPreviewController* Owner = Controller.Get())
	{
		Owner->ShowPreviewForSlot(SlotWidget.Get());
	}
}

void UNexusAbilityPreviewController::InitController(UUserWidget* InScreen)
{
	Screen = InScreen;

	UClass* SlotClass = LoadClass<UUserWidget>(nullptr, TEXT("/Game/Widgets/W_AbilitySlot.W_AbilitySlot_C"));
	if (!SlotClass)
	{
		UE_LOG(LogNexusAbilityUI, Warning,
			TEXT("AbilityPreview: failed to load W_AbilitySlot_C, no slots bound"));
		return;
	}

	int32 BoundCount = 0;
	InScreen->WidgetTree->ForEachWidget([&](UWidget* Widget)
	{
		UUserWidget* SlotW = Cast<UUserWidget>(Widget);
		if (!SlotW || !SlotW->GetClass()->IsChildOf(SlotClass))
		{
			return;
		}

		const FMulticastDelegateProperty* DispatcherProp = CastField<FMulticastDelegateProperty>(
			FindPropByDisplayName(SlotW->GetClass(), TEXT("OnAbilitySlotClicked")));
		if (!DispatcherProp)
		{
			UE_LOG(LogNexusAbilityUI, Warning,
				TEXT("AbilityPreview: slot %s has no OnAbilitySlotClicked dispatcher"), *SlotW->GetName());
			return;
		}

		UNexusAbilitySlotRelay* Relay = NewObject<UNexusAbilitySlotRelay>(this);
		Relay->InitRelay(SlotW, this);
		Relays.Add(Relay);

		FScriptDelegate Delegate;
		Delegate.BindUFunction(Relay, GET_FUNCTION_NAME_CHECKED(UNexusAbilitySlotRelay, HandleSlotClicked));
		DispatcherProp->AddDelegate(MoveTemp(Delegate), SlotW);
		++BoundCount;
	});

	if (UButton* UnlockButton = Cast<UButton>(InScreen->GetWidgetFromName(TEXT("UnlockButton"))))
	{
		UnlockButton->OnClicked.AddUniqueDynamic(this, &UNexusAbilityPreviewController::HandleUnlockClicked);
		// Nothing selected yet: keep the button inert until a slot is clicked.
		UnlockButton->SetIsEnabled(false);
		UnlockButton->SetBackgroundColor(GDisabledGrey);
	}
	else
	{
		UE_LOG(LogNexusAbilityUI, Warning,
			TEXT("AbilityPreview: %s has no Button named 'UnlockButton'"), *InScreen->GetName());
	}

	const auto BindKeybindButton = [this, InScreen](const TCHAR* WidgetName, FName HandlerName)
	{
		UButton* Button = Cast<UButton>(InScreen->GetWidgetFromName(WidgetName));
		if (!Button)
		{
			UE_LOG(LogNexusAbilityUI, Warning,
				TEXT("AbilityPreview: %s has no Button named '%s'"), *InScreen->GetName(), WidgetName);
			return;
		}
		FScriptDelegate Delegate;
		Delegate.BindUFunction(this, HandlerName);
		Button->OnClicked.AddUnique(Delegate);
		// Nothing selected yet: inert like the Unlock button until a slot is clicked.
		Button->SetIsEnabled(false);
		Button->SetBackgroundColor(GDisabledGrey);
	};
	BindKeybindButton(TEXT("Keybind1"),
		GET_FUNCTION_NAME_CHECKED(UNexusAbilityPreviewController, HandleKeybind1Clicked));
	BindKeybindButton(TEXT("Keybind2"),
		GET_FUNCTION_NAME_CHECKED(UNexusAbilityPreviewController, HandleKeybind2Clicked));
	BindKeybindButton(TEXT("Keybind3"),
		GET_FUNCTION_NAME_CHECKED(UNexusAbilityPreviewController, HandleKeybind3Clicked));
	BindKeybindButton(TEXT("Keybind4"),
		GET_FUNCTION_NAME_CHECKED(UNexusAbilityPreviewController, HandleKeybind4Clicked));

	UE_LOG(LogNexusAbilityUI, Log, TEXT("AbilityPreview: bound %d ability slot dispatcher(s) on %s"),
		BoundCount, *InScreen->GetName());
}

void UNexusAbilityPreviewController::ShowPreviewForSlot(UUserWidget* InSlotWidget)
{
	if (!InSlotWidget)
	{
		return;
	}

	const FClassProperty* AbilityDataProp = CastField<FClassProperty>(
		FindPropByDisplayName(InSlotWidget->GetClass(), TEXT("AbilityData")));
	UClass* AbilityClass = AbilityDataProp
		? Cast<UClass>(AbilityDataProp->GetObjectPropertyValue_InContainer(InSlotWidget))
		: nullptr;

	SelectedSlot = InSlotWidget;
	SelectedAbility = AbilityClass;
	UE_LOG(LogNexusAbilityUI, Log, TEXT("AbilityPreview: slot %s clicked, ability %s"),
		*InSlotWidget->GetName(), AbilityClass ? *AbilityClass->GetName() : TEXT("null (not assigned)"));
	RefreshPreview();
}

void UNexusAbilityPreviewController::RefreshPreview()
{
	UUserWidget* ScreenW = Screen.Get();
	if (!ScreenW || !ScreenW->WidgetTree)
	{
		return;
	}

	UButton* UnlockButton = Cast<UButton>(ScreenW->GetWidgetFromName(TEXT("UnlockButton")));
	UTextBlock* UnlockText = Cast<UTextBlock>(ScreenW->GetWidgetFromName(TEXT("UnlockButtonText")));

	UClass* AbilityClass = SelectedAbility.Get();
	if (!AbilityClass)
	{
		if (UnlockButton)
		{
			UnlockButton->SetIsEnabled(false);
			UnlockButton->SetBackgroundColor(GDisabledGrey);
		}
		RefreshKeybindButtons();
		return;
	}

	// Name / description / icon from the ability data CDO's S_AbilityData struct
	// (AbilityName and Description are FStrings there, AbilityIcon a Texture2D).
	const UObject* DataCDO = AbilityClass->GetDefaultObject();
	const FStructProperty* DataProp = CastField<FStructProperty>(
		FindPropByDisplayName(AbilityClass, TEXT("AbilityData")));
	if (DataCDO && DataProp)
	{
		const void* DataPtr = DataProp->ContainerPtrToValuePtr<void>(DataCDO);
		const UStruct* DataStruct = DataProp->Struct;

		if (UTextBlock* NameText = Cast<UTextBlock>(ScreenW->GetWidgetFromName(TEXT("PreviewName"))))
		{
			if (const FStrProperty* NameProp = CastField<FStrProperty>(
				FindPropByDisplayName(DataStruct, TEXT("AbilityName"))))
			{
				NameText->SetText(FText::FromString(NameProp->GetPropertyValue_InContainer(DataPtr)));
			}
		}
		if (UTextBlock* DescText = Cast<UTextBlock>(ScreenW->GetWidgetFromName(TEXT("PreviewDescription"))))
		{
			if (const FStrProperty* DescProp = CastField<FStrProperty>(
				FindPropByDisplayName(DataStruct, TEXT("Description"))))
			{
				DescText->SetText(FText::FromString(DescProp->GetPropertyValue_InContainer(DataPtr)));
			}
		}
		if (UImage* IconImage = Cast<UImage>(ScreenW->GetWidgetFromName(TEXT("PreviewIcon"))))
		{
			const FObjectPropertyBase* IconProp = CastField<FObjectPropertyBase>(
				FindPropByDisplayName(DataStruct, TEXT("AbilityIcon")));
			if (UTexture2D* Icon = IconProp
				? Cast<UTexture2D>(IconProp->GetObjectPropertyValue_InContainer(DataPtr))
				: nullptr)
			{
				IconImage->SetBrushFromTexture(Icon);
			}
		}
	}
	else
	{
		UE_LOG(LogNexusAbilityUI, Warning, TEXT("AbilityPreview: %s has no AbilityData struct to preview"),
			*AbilityClass->GetName());
	}

	FNexusStatSource Source;
	const USaveGame* Save = BuildStatSource(ScreenW, Source);
	TSet<FString> Unlocked;
	ReadUnlockedAbilities(Save, Unlocked);

	if (UHorizontalBox* ReqBox = Cast<UHorizontalBox>(ScreenW->GetWidgetFromName(TEXT("PreviewRequirementsBox"))))
	{
		TArray<FNexusRequirementView> Views;
		CollectRequirementViews(Source, AbilityClass, Views);

		ReqBox->ClearChildren();
		for (const FNexusRequirementView& View : Views)
		{
			UTextBlock* ReqText = ScreenW->WidgetTree->ConstructWidget<UTextBlock>();
			ReqText->SetText(FText::FromString(FString::Printf(TEXT("%s %d"),
				View.Abbrev ? View.Abbrev : *View.TypeName.ToUpper(),
				FMath::RoundToInt32(View.Required))));
			ReqText->SetColorAndOpacity(FSlateColor(View.CountsAsMet() ? GReqMetGreen : GReqUnmetRed));
			FSlateFontInfo Font = ReqText->GetFont();
			Font.Size = 12;
			ReqText->SetFont(Font);
			if (UHorizontalBoxSlot* ReqSlot = ReqBox->AddChildToHorizontalBox(ReqText))
			{
				ReqSlot->SetPadding(FMargin(0.0f, 0.0f, 10.0f, 0.0f));
				ReqSlot->SetVerticalAlignment(VAlign_Center);
			}
		}
	}
	else
	{
		UE_LOG(LogNexusAbilityUI, Warning,
			TEXT("AbilityPreview: %s has no HorizontalBox named 'PreviewRequirementsBox'"),
			*ScreenW->GetName());
	}

	const bool bUnlocked = Unlocked.Contains(AbilityClass->GetName());
	// Same verdict CheckRequirements reaches, so the button always matches the overlays.
	const bool bAllMet = AreRequirementsMet(Source, AbilityClass, TEXT("Preview"));
	const bool bCanUnlock = bAllMet && !bUnlocked;

	if (UnlockText)
	{
		UnlockText->SetText(bUnlocked
			? NSLOCTEXT("Nexus", "AbilityUnlocked", "Unlocked")
			: bCanUnlock
				? NSLOCTEXT("Nexus", "AbilityUnlock", "Unlock")
				: NSLOCTEXT("Nexus", "AbilityLocked", "Locked"));
	}
	if (UnlockButton)
	{
		UnlockButton->SetIsEnabled(bCanUnlock);
		UnlockButton->SetBackgroundColor(bCanUnlock ? UnlockGold() : GDisabledGrey);
	}

	RefreshKeybindButtons();

	UE_LOG(LogNexusAbilityUI, Log, TEXT("AbilityPreview: showing %s -> %s"),
		*AbilityClass->GetName(),
		bUnlocked ? TEXT("Unlocked") : bCanUnlock ? TEXT("Unlock (gold)") : TEXT("Locked"));
}

void UNexusAbilityPreviewController::HandleUnlockClicked()
{
	UUserWidget* ScreenW = Screen.Get();
	UClass* AbilityClass = SelectedAbility.Get();
	if (!ScreenW || !AbilityClass)
	{
		return;
	}

	if (UNexusAbilityUILibrary::UnlockAbility(ScreenW, AbilityClass))
	{
		// Refreshes every slot's lock overlay (including the selected one), then the panel.
		UNexusAbilityUILibrary::CheckRequirements(ScreenW);
		RefreshPreview();
	}
}

void UNexusAbilityPreviewController::HandleKeybind1Clicked() { HandleKeybindClicked(1); }
void UNexusAbilityPreviewController::HandleKeybind2Clicked() { HandleKeybindClicked(2); }
void UNexusAbilityPreviewController::HandleKeybind3Clicked() { HandleKeybindClicked(3); }
void UNexusAbilityPreviewController::HandleKeybind4Clicked() { HandleKeybindClicked(4); }

void UNexusAbilityPreviewController::HandleKeybindClicked(int32 KeybindSlot)
{
	UUserWidget* ScreenW = Screen.Get();
	UClass* AbilityClass = SelectedAbility.Get();
	if (!ScreenW || !AbilityClass)
	{
		return;
	}
	// The library rejects locked abilities too; this only spares the warning log when
	// a click sneaks through on a button that should already be disabled.
	if (!UNexusAbilityUILibrary::IsAbilityUnlocked(ScreenW, AbilityClass))
	{
		return;
	}

	// Toggle semantics live in the library: clicking the occupied slot clears it.
	if (UNexusAbilityUILibrary::AssignAbilityToKeybindSlot(ScreenW, AbilityClass, KeybindSlot))
	{
		RefreshKeybindButtons();
	}
}

void UNexusAbilityPreviewController::RefreshKeybindButtons()
{
	UUserWidget* ScreenW = Screen.Get();
	if (!ScreenW)
	{
		return;
	}

	UClass* AbilityClass = SelectedAbility.Get();
	const bool bUnlocked = AbilityClass && UNexusAbilityUILibrary::IsAbilityUnlocked(ScreenW, AbilityClass);
	const int32 AssignedSlot = bUnlocked
		? UNexusAbilityUILibrary::GetAssignedSlotForAbility(ScreenW, AbilityClass)
		: INDEX_NONE;

	for (int32 KeybindSlot = 1; KeybindSlot <= 4; ++KeybindSlot)
	{
		UButton* Button = Cast<UButton>(ScreenW->GetWidgetFromName(
			FName(*FString::Printf(TEXT("Keybind%d"), KeybindSlot))));
		if (!Button)
		{
			continue;
		}
		Button->SetIsEnabled(bUnlocked);
		Button->SetBackgroundColor(!bUnlocked
			? GDisabledGrey
			: KeybindSlot - 1 == AssignedSlot ? UnlockGold() : FLinearColor::White);
	}
}

void UNexusAbilityUILibrary::SetupAbilityPreview(UUserWidget* AbilitiesScreen)
{
	if (!AbilitiesScreen || !AbilitiesScreen->WidgetTree)
	{
		UE_LOG(LogNexusAbilityUI, Warning, TEXT("SetupAbilityPreview: AbilitiesScreen is %s"),
			AbilitiesScreen ? TEXT("valid but has no WidgetTree") : TEXT("null"));
		return;
	}

	FNexusPreviewControllerRegistry& Registry = FNexusPreviewControllerRegistry::Get();
	Registry.Prune();
	if (UNexusAbilityPreviewController* Existing = Registry.Find(AbilitiesScreen))
	{
		// Construct fired again for the same screen: bindings are still in place.
		Existing->RefreshPreview();
		return;
	}

	UNexusAbilityPreviewController* Controller = NewObject<UNexusAbilityPreviewController>(GetTransientPackage());
	Controller->InitController(AbilitiesScreen);
	Registry.Add(AbilitiesScreen, Controller);
}

TSubclassOf<UObject> UNexusAbilityUILibrary::GetSelectedAbility(UUserWidget* AbilitiesScreen)
{
	const UNexusAbilityPreviewController* Controller = AbilitiesScreen
		? FNexusPreviewControllerRegistry::Get().Find(AbilitiesScreen)
		: nullptr;
	return Controller ? Controller->GetSelectedAbility() : nullptr;
}

namespace
{
	// Keybind slots One-Four; index 0-3 into AssignedAbilities, input IDs Ability1-4.
	constexpr int32 GNumAbilityKeybindSlots = 4;

	// The AssignedAbilities TArray<FString> variable added to BP_SaveGame.
	const FArrayProperty* FindAssignedAbilitiesProp(const USaveGame* Save)
	{
		if (!Save)
		{
			return nullptr;
		}
		const FArrayProperty* ArrayProp = CastField<FArrayProperty>(
			FindPropByDisplayName(Save->GetClass(), TEXT("AssignedAbilities")));
		return (ArrayProp && CastField<FStrProperty>(ArrayProp->Inner)) ? ArrayProp : nullptr;
	}

	// Always yields exactly GNumAbilityKeybindSlots entries; missing/short data reads as empty.
	void ReadAssignedAbilities(const USaveGame* Save, TArray<FString>& OutAssigned)
	{
		OutAssigned.Init(FString(), GNumAbilityKeybindSlots);
		const FArrayProperty* ArrayProp = FindAssignedAbilitiesProp(Save);
		if (!ArrayProp)
		{
			return;
		}
		const FStrProperty* Inner = CastField<FStrProperty>(ArrayProp->Inner);
		FScriptArrayHelper ArrayHelper(ArrayProp,
			ArrayProp->ContainerPtrToValuePtr<void>(const_cast<USaveGame*>(Save)));
		const int32 Count = FMath::Min(ArrayHelper.Num(), GNumAbilityKeybindSlots);
		for (int32 Index = 0; Index < Count; ++Index)
		{
			OutAssigned[Index] = Inner->GetPropertyValue(ArrayHelper.GetRawPtr(Index));
		}
	}

	bool WriteAssignedAbilities(USaveGame* Save, const TArray<FString>& Assigned)
	{
		const FArrayProperty* ArrayProp = FindAssignedAbilitiesProp(Save);
		if (!ArrayProp)
		{
			UE_LOG(LogNexusAbilityUI, Warning,
				TEXT("WriteAssignedAbilities: %s has no AssignedAbilities TArray<FString> variable"),
				Save ? *Save->GetClass()->GetName() : TEXT("null save"));
			return false;
		}
		const FStrProperty* Inner = CastField<FStrProperty>(ArrayProp->Inner);
		FScriptArrayHelper ArrayHelper(ArrayProp, ArrayProp->ContainerPtrToValuePtr<void>(Save));
		ArrayHelper.Resize(GNumAbilityKeybindSlots);
		for (int32 Index = 0; Index < GNumAbilityKeybindSlots; ++Index)
		{
			Inner->SetPropertyValue(ArrayHelper.GetRawPtr(Index), Assigned[Index]);
		}
		return true;
	}

	// Declared up by ResolveOrCreateSaveGame, which only ever runs this on a save that has
	// never been written to disk, so it can never overwrite a slot the player has
	// deliberately cleared. It appends to UnlockedAbilities rather than assuming it is empty.
	void SeedFreshSaveAbilities(USaveGame* Save)
	{
		const FArrayProperty* UnlockedProp = FindUnlockedAbilitiesProp(Save);
		if (!UnlockedProp)
		{
			UE_LOG(LogNexusAbilityUI, Warning,
				TEXT("SeedFreshSaveAbilities: %s has no UnlockedAbilities TArray<FString> variable"),
				Save ? *Save->GetClass()->GetName() : TEXT("null save"));
			return;
		}

		const FStrProperty* UnlockedInner = CastField<FStrProperty>(UnlockedProp->Inner);
		FScriptArrayHelper UnlockedHelper(UnlockedProp, UnlockedProp->ContainerPtrToValuePtr<void>(Save));

		TArray<FString> Assigned;
		ReadAssignedAbilities(Save, Assigned);

		for (const FFreshSaveAbilitySeed& Seed : GFreshSaveAbilities)
		{
			const FString Path(Seed.DataClassPath);

			// UnlockedAbilities stores the data class name; AssignedAbilities stores its
			// full path. Derive the name from the path so the two can never disagree.
			FString ClassName;
			if (!Path.Split(TEXT("."), nullptr, &ClassName, ESearchCase::CaseSensitive, ESearchDir::FromEnd))
			{
				UE_LOG(LogNexusAbilityUI, Warning,
					TEXT("SeedFreshSaveAbilities: '%s' is not an object path, skipped"), *Path);
				continue;
			}

			const int32 NewIndex = UnlockedHelper.AddValue();
			UnlockedInner->SetPropertyValue(UnlockedHelper.GetRawPtr(NewIndex), ClassName);

			if (Assigned.IsValidIndex(Seed.SlotIndex))
			{
				Assigned[Seed.SlotIndex] = Path;
			}

			UE_LOG(LogNexusAbilityUI, Log,
				TEXT("SeedFreshSaveAbilities: unlocked '%s', keybind slot %d"),
				*ClassName, Seed.SlotIndex + 1);
		}

		WriteAssignedAbilities(Save, Assigned);
	}

	// The GameplayAbility class stored in AbilityClass inside the data class's S_AbilityData.
	UClass* ResolveGrantedAbilityClass(UClass* AbilityDataClass)
	{
		const UObject* DataCDO = AbilityDataClass ? AbilityDataClass->GetDefaultObject() : nullptr;
		const FStructProperty* DataProp = AbilityDataClass
			? CastField<FStructProperty>(FindPropByDisplayName(AbilityDataClass, TEXT("AbilityData")))
			: nullptr;
		if (!DataCDO || !DataProp)
		{
			return nullptr;
		}
		const void* DataPtr = DataProp->ContainerPtrToValuePtr<void>(DataCDO);

		const FProperty* AbilityClassProp = FindPropByDisplayName(DataProp->Struct, TEXT("AbilityClass"));
		UClass* GrantClass = nullptr;
		if (const FClassProperty* HardProp = CastField<FClassProperty>(AbilityClassProp))
		{
			GrantClass = Cast<UClass>(HardProp->GetObjectPropertyValue_InContainer(DataPtr));
		}
		else if (const FSoftClassProperty* SoftProp = CastField<FSoftClassProperty>(AbilityClassProp))
		{
			GrantClass = Cast<UClass>(SoftProp->GetPropertyValue_InContainer(DataPtr).LoadSynchronous());
		}
		else if (const FObjectProperty* ObjProp = CastField<FObjectProperty>(AbilityClassProp))
		{
			// S_AbilityData declares AbilityClass as a GameplayAbility *object* reference,
			// so the data assets hold an ability CDO; grant that instance's class.
			if (const UObject* Obj = ObjProp->GetObjectPropertyValue_InContainer(DataPtr))
			{
				GrantClass = Obj->GetClass();
			}
		}
		return (GrantClass && GrantClass->IsChildOf(UGameplayAbility::StaticClass())) ? GrantClass : nullptr;
	}
}

bool UNexusAbilityUILibrary::AssignAbilityToSlot(const UObject* WorldContextObject,
	TSubclassOf<UObject> AbilityDataClass, int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= GNumAbilityKeybindSlots)
	{
		UE_LOG(LogNexusAbilityUI, Warning, TEXT("AssignAbilityToSlot: SlotIndex %d out of range 0-%d"),
			SlotIndex, GNumAbilityKeybindSlots - 1);
		return false;
	}

	FString SlotName;
	USaveGame* SaveGame = ResolveOrCreateSaveGame(WorldContextObject, SlotName);
	if (!SaveGame)
	{
		return false;
	}

	if (AbilityDataClass)
	{
		TSet<FString> Unlocked;
		ReadUnlockedAbilities(SaveGame, Unlocked);
		if (!Unlocked.Contains(AbilityDataClass->GetName()))
		{
			UE_LOG(LogNexusAbilityUI, Warning,
				TEXT("AssignAbilityToSlot: '%s' is not in UnlockedAbilities, not assigning"),
				*AbilityDataClass->GetName());
			return false;
		}
	}

	TArray<FString> Assigned;
	ReadAssignedAbilities(SaveGame, Assigned);

	// An ability occupies at most one slot: clear it from wherever else it sits.
	const FString AbilityPath = AbilityDataClass ? AbilityDataClass->GetPathName() : FString();
	if (!AbilityPath.IsEmpty())
	{
		for (int32 Index = 0; Index < Assigned.Num(); ++Index)
		{
			if (Index != SlotIndex && Assigned[Index] == AbilityPath)
			{
				Assigned[Index].Reset();
			}
		}
	}
	Assigned[SlotIndex] = AbilityPath;

	if (!WriteAssignedAbilities(SaveGame, Assigned))
	{
		return false;
	}

	const bool bSaved = UGameplayStatics::SaveGameToSlot(SaveGame, SlotName, 0);
	UE_LOG(LogNexusAbilityUI, Log,
		TEXT("AssignAbilityToSlot: slot %d = '%s' in save slot '%s' -> %s"),
		SlotIndex, AbilityPath.IsEmpty() ? TEXT("<cleared>") : *AbilityPath, *SlotName,
		bSaved ? TEXT("saved") : TEXT("SAVE FAILED"));

	// In-game reassignment: possession-time granting only covers the initial spawn,
	// so re-sync the live pawn's keybind abilities (clear stale + regrant) right away.
	// In the main menu there is no pawn and this is a no-op.
	if (bSaved)
	{
		if (ACharacter* PlayerCharacter = Cast<ACharacter>(UGameplayStatics::GetPlayerPawn(WorldContextObject, 0)))
		{
			if (PlayerCharacter->HasAuthority())
			{
				GrantAssignedAbilities(PlayerCharacter);
			}
		}
	}
	return bSaved;
}

FString UNexusAbilityUILibrary::GetAssignedAbility(const UObject* WorldContextObject, int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= GNumAbilityKeybindSlots)
	{
		return FString();
	}
	FString SlotNameUnused;
	const USaveGame* SaveGame = ResolveSaveGame(WorldContextObject, SlotNameUnused);
	TArray<FString> Assigned;
	ReadAssignedAbilities(SaveGame, Assigned);
	return Assigned[SlotIndex];
}

int32 UNexusAbilityUILibrary::GetAssignedSlotForAbility(const UObject* WorldContextObject,
	TSubclassOf<UObject> AbilityDataClass)
{
	if (!AbilityDataClass)
	{
		return INDEX_NONE;
	}
	FString SlotNameUnused;
	const USaveGame* SaveGame = ResolveSaveGame(WorldContextObject, SlotNameUnused);
	TArray<FString> Assigned;
	ReadAssignedAbilities(SaveGame, Assigned);
	return Assigned.IndexOfByKey(AbilityDataClass->GetPathName());
}

bool UNexusAbilityUILibrary::AssignAbilityToKeybindSlot(const UObject* WorldContextObject,
	TSubclassOf<UObject> AbilityDataClass, int32 KeybindSlot)
{
	if (!AbilityDataClass)
	{
		UE_LOG(LogNexusAbilityUI, Warning, TEXT("AssignAbilityToKeybindSlot: AbilityDataClass is null"));
		return false;
	}
	if (KeybindSlot < 1 || KeybindSlot > GNumAbilityKeybindSlots)
	{
		UE_LOG(LogNexusAbilityUI, Warning,
			TEXT("AssignAbilityToKeybindSlot: KeybindSlot %d out of range 1-%d"),
			KeybindSlot, GNumAbilityKeybindSlots);
		return false;
	}

	// Clicking the slot the ability already occupies clears it (toggle).
	const int32 SlotIndex = KeybindSlot - 1;
	const bool bAlreadyInSlot =
		GetAssignedSlotForAbility(WorldContextObject, AbilityDataClass) == SlotIndex;
	return AssignAbilityToSlot(WorldContextObject, bAlreadyInSlot ? nullptr : AbilityDataClass, SlotIndex);
}

TArray<FString> UNexusAbilityUILibrary::GetAssignedAbilities(const UObject* WorldContextObject)
{
	FString SlotNameUnused;
	const USaveGame* SaveGame = ResolveSaveGame(WorldContextObject, SlotNameUnused);
	TArray<FString> Assigned;
	ReadAssignedAbilities(SaveGame, Assigned);
	return Assigned;
}

int32 UNexusAbilityUILibrary::GrantAssignedAbilities(ACharacter* Character)
{
	if (!Character || !Character->HasAuthority())
	{
		UE_LOG(LogNexusAbilityUI, Warning, TEXT("GrantAssignedAbilities: %s"),
			!Character ? TEXT("Character is null") : TEXT("no authority (grants are server-side)"));
		return 0;
	}
	const IAbilitySystemInterface* AscInterface = Cast<IAbilitySystemInterface>(Character);
	UAbilitySystemComponent* ASC = AscInterface ? AscInterface->GetAbilitySystemComponent() : nullptr;
	if (!ASC)
	{
		UE_LOG(LogNexusAbilityUI, Warning, TEXT("GrantAssignedAbilities: %s has no AbilitySystemComponent"),
			*Character->GetName());
		return 0;
	}

	// Clear whatever a previous sync granted on the keybind input IDs so this call
	// always mirrors the save exactly.
	const int32 FirstInputID = static_cast<int32>(EAbilityInputID::Ability1);
	TArray<FGameplayAbilitySpecHandle> StaleHandles;
	for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		if (Spec.InputID >= FirstInputID && Spec.InputID < FirstInputID + GNumAbilityKeybindSlots)
		{
			StaleHandles.Add(Spec.Handle);
		}
	}
	for (const FGameplayAbilitySpecHandle& Handle : StaleHandles)
	{
		ASC->ClearAbility(Handle);
	}

	// Booting straight into a level on a wiped save would otherwise read an empty slot
	// array and grant nothing, because the fresh save is only minted by the menu flow.
	// Resolve-or-create here so a first-run pawn still spawns with the seeded ultimates.
	FString SlotNameUnused;
	USaveGame* SaveGame = ResolveOrCreateSaveGame(Character, SlotNameUnused);
	TArray<FString> Assigned;
	ReadAssignedAbilities(SaveGame, Assigned);

	int32 GrantedCount = 0;
	for (int32 SlotIndex = 0; SlotIndex < Assigned.Num(); ++SlotIndex)
	{
		if (Assigned[SlotIndex].IsEmpty())
		{
			continue;
		}

		UClass* AbilityDataClass = LoadClass<UObject>(nullptr, *Assigned[SlotIndex]);
		if (!AbilityDataClass)
		{
			UE_LOG(LogNexusAbilityUI, Warning,
				TEXT("GrantAssignedAbilities: slot %d data class '%s' failed to load"),
				SlotIndex, *Assigned[SlotIndex]);
			continue;
		}

		UClass* GrantClass = ResolveGrantedAbilityClass(AbilityDataClass);
		if (!GrantClass)
		{
			UE_LOG(LogNexusAbilityUI, Warning,
				TEXT("GrantAssignedAbilities: %s has no GameplayAbility in its AbilityData's AbilityClass"),
				*AbilityDataClass->GetName());
			continue;
		}

		const int32 InputID = FirstInputID + SlotIndex;
		bool bGranted;
		if (ANexusCharacterBase* NexusCharacter = Cast<ANexusCharacterBase>(Character))
		{
			bGranted = NexusCharacter->GrantAbilityWithInputID(GrantClass, InputID).IsValid();
		}
		else
		{
			bGranted = ASC->GiveAbility(FGameplayAbilitySpec(GrantClass, 1, InputID, Character)).IsValid();
		}
		if (bGranted)
		{
			++GrantedCount;
			UE_LOG(LogNexusAbilityUI, Log,
				TEXT("GrantAssignedAbilities: slot %d -> %s (InputID %d)"),
				SlotIndex, *GrantClass->GetName(), InputID);
		}
	}

	UE_LOG(LogNexusAbilityUI, Log, TEXT("GrantAssignedAbilities: granted %d ability(ies) to %s, cleared %d stale"),
		GrantedCount, *Character->GetName(), StaleHandles.Num());
	return GrantedCount;
}


FString UNexusAbilityUILibrary::GetClassDefaultPropertyText(UClass* TargetClass, FName PropertyName)
{
	if (!TargetClass)
	{
		return FString();
	}
	FProperty* Property = TargetClass->FindPropertyByName(PropertyName);
	UObject* DefaultObject = TargetClass->GetDefaultObject();
	if (!Property || !DefaultObject)
	{
		return FString();
	}
	FString ValueText;
	Property->ExportText_InContainer(0, ValueText, DefaultObject, nullptr, DefaultObject, PPF_None);
	return ValueText;
}

bool UNexusAbilityUILibrary::SetClassDefaultPropertyText(UClass* TargetClass, FName PropertyName, const FString& ValueText)
{
	if (!TargetClass)
	{
		return false;
	}
	FProperty* Property = TargetClass->FindPropertyByName(PropertyName);
	UObject* DefaultObject = TargetClass->GetDefaultObject();
	if (!Property || !DefaultObject)
	{
		return false;
	}
	DefaultObject->Modify();
	const TCHAR* Result = Property->ImportText_InContainer(*ValueText, DefaultObject, DefaultObject, PPF_None);
	if (!Result)
	{
		UE_LOG(LogNexusAbilityUI, Warning, TEXT("SetClassDefaultPropertyText: ImportText failed for %s.%s"),
			*TargetClass->GetName(), *PropertyName.ToString());
		return false;
	}
	DefaultObject->MarkPackageDirty();
	return true;
}

int32 UNexusAbilityUILibrary::GetGold(const UObject* WorldContextObject)
{
	FString SlotNameUnused;
	const USaveGame* SaveGame = ResolveOrCreateSaveGame(WorldContextObject, SlotNameUnused);
	if (!SaveGame)
	{
		return 0;
	}
	double Gold = 0.0;
	if (!ReadNumericField(SaveGame->GetClass(), SaveGame, TEXT("Gold"), Gold))
	{
		UE_LOG(LogNexusAbilityUI, Warning, TEXT("GetGold: %s has no Gold variable"),
			*SaveGame->GetClass()->GetName());
	}
	return static_cast<int32>(Gold);
}

int32 UNexusAbilityUILibrary::AddGold(const UObject* WorldContextObject, int32 Amount)
{
	FString SlotName;
	USaveGame* SaveGame = ResolveOrCreateSaveGame(WorldContextObject, SlotName);
	if (!SaveGame)
	{
		return -1;
	}

	double Gold = 0.0;
	if (!ReadNumericField(SaveGame->GetClass(), SaveGame, TEXT("Gold"), Gold))
	{
		UE_LOG(LogNexusAbilityUI, Warning, TEXT("AddGold: %s has no Gold variable"),
			*SaveGame->GetClass()->GetName());
		return -1;
	}
	const int32 NewTotal = static_cast<int32>(Gold) + Amount;
	WriteNumericField(SaveGame->GetClass(), SaveGame, TEXT("Gold"), static_cast<double>(NewTotal));

	const bool bSaved = UGameplayStatics::SaveGameToSlot(SaveGame, SlotName, 0);
	UE_LOG(LogNexusAbilityUI, Log, TEXT("AddGold: %+d -> %d in slot '%s', SaveGameToSlot -> %s"),
		Amount, NewTotal, *SlotName, bSaved ? TEXT("success") : TEXT("FAILED"));

	UpdateGoldDisplay(WorldContextObject);
	return NewTotal;
}

int32 UNexusAbilityUILibrary::GetRunInventoryGold(const UObject* WorldContextObject)
{
	APawn* Pawn = UGameplayStatics::GetPlayerPawn(const_cast<UObject*>(WorldContextObject), 0);
	UNarrativeInventoryComponent* RunInventory = FindPlayerInventory(Pawn, FName(TEXT("RunInventory")));
	if (!RunInventory)
	{
		return 0;
	}

	UClass* GoldClass = LoadClass<UNarrativeItem>(nullptr, TEXT("/Game/Inventory/Items/BP_Item_Gold.BP_Item_Gold_C"));
	if (!GoldClass)
	{
		return 0;
	}

	int32 Total = 0;
	for (UNarrativeItem* Item : RunInventory->FindItemsOfClass(GoldClass))
	{
		if (IsValid(Item))
		{
			Total += Item->GetQuantity();
		}
	}
	return Total;
}

bool UNexusAbilityUILibrary::UpdateGoldDisplay(const UObject* WorldContextObject)
{
	const int32 GoldTotal = GetRunInventoryGold(WorldContextObject);

	TArray<UUserWidget*> TopLevelWidgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(const_cast<UObject*>(WorldContextObject),
		TopLevelWidgets, UUserWidget::StaticClass(), /*TopLevelOnly*/ true);

	bool bFound = false;
	for (UUserWidget* Widget : TopLevelWidgets)
	{
		if (UTextBlock* GoldText = Widget ? Cast<UTextBlock>(Widget->GetWidgetFromName(TEXT("GoldText"))) : nullptr)
		{
			GoldText->SetText(FText::FromString(FString::FromInt(GoldTotal)));
			bFound = true;
		}
	}
	return bFound;
}

bool UNexusAbilityUILibrary::GrantPotion(AActor* PlayerActor, int32 Count)
{
	if (!PlayerActor)
	{
		return false;
	}
	double PotionCount = 0.0;
	if (!ReadNumericField(PlayerActor->GetClass(), PlayerActor, TEXT("HealthPotionCount"), PotionCount))
	{
		UE_LOG(LogNexusAbilityUI, Warning, TEXT("GrantPotion: %s has no HealthPotionCount variable"),
			*PlayerActor->GetClass()->GetName());
		return false;
	}
	WriteNumericField(PlayerActor->GetClass(), PlayerActor, TEXT("HealthPotionCount"), PotionCount + Count);
	UE_LOG(LogNexusAbilityUI, Log, TEXT("GrantPotion: %s now has %d potion(s)"),
		*PlayerActor->GetName(), static_cast<int32>(PotionCount) + Count);
	return true;
}

void UNexusAbilityUILibrary::HandleLootPickup(AActor* Pickup, AActor* OtherActor)
{
	if (!Pickup || Pickup->IsActorBeingDestroyed() || !Pickup->HasAuthority())
	{
		return;
	}
	const ANexusCharacterBase* Player = Cast<ANexusCharacterBase>(OtherActor);
	if (!Player || !Player->IsPlayerControlled())
	{
		return;
	}

	bool bIsGold = true;
	if (const FBoolProperty* GoldProp = CastField<FBoolProperty>(
		FindPropByDisplayName(Pickup->GetClass(), TEXT("bIsGold"))))
	{
		bIsGold = GoldProp->GetPropertyValue(GoldProp->ContainerPtrToValuePtr<void>(Pickup));
	}

	// Pickups now flow into the run inventory as Narrative items instead of the save-int gold
	// / flask-count. Run loot is lost on death and banked only on extraction (see
	// ExtractRunInventoryToStash). Drinking a potion from the inventory still feeds the flask
	// via BP_Item_HealthPotion's OnUse, so the F-key flow is unaffected.
	UNarrativeInventoryComponent* RunInventory = FindPlayerInventory(OtherActor, FName(TEXT("RunInventory")));
	if (RunInventory)
	{
		if (bIsGold)
		{
			double GoldAmount = 10.0;
			ReadNumericField(Pickup->GetClass(), Pickup, TEXT("GoldAmount"), GoldAmount);
			if (UClass* GoldClass = LoadClass<UNarrativeItem>(nullptr,
				TEXT("/Game/Inventory/Items/BP_Item_Gold.BP_Item_Gold_C")))
			{
				RunInventory->TryAddItemFromClass(GoldClass, static_cast<int32>(GoldAmount), /*bCheckAutoUse*/ false);
			}
			UpdateGoldDisplay(OtherActor);
		}
		else if (UClass* PotionClass = LoadClass<UNarrativeItem>(nullptr,
			TEXT("/Game/Inventory/Items/BP_Item_HealthPotion.BP_Item_HealthPotion_C")))
		{
			RunInventory->TryAddItemFromClass(PotionClass, 1, /*bCheckAutoUse*/ false);
		}
	}
	else
	{
		UE_LOG(LogNexusAbilityUI, Warning, TEXT("HandleLootPickup: no RunInventory on %s, loot dropped"),
			*GetNameSafe(OtherActor));
	}

	if (const FObjectPropertyBase* SoundProp = CastField<FObjectPropertyBase>(
		FindPropByDisplayName(Pickup->GetClass(), TEXT("PickupSound"))))
	{
		if (USoundBase* Sound = Cast<USoundBase>(
			SoundProp->GetObjectPropertyValue(SoundProp->ContainerPtrToValuePtr<void>(Pickup))))
		{
			UGameplayStatics::PlaySoundAtLocation(Pickup, Sound, Pickup->GetActorLocation());
		}
	}

	Pickup->Destroy();
}

AActor* UNexusAbilityUILibrary::SpawnLootDrop(AActor* DeadEnemy, float DropChance, float GoldShare)
{
	if (!DeadEnemy || !DeadEnemy->HasAuthority() || FMath::FRand() >= DropChance)
	{
		return nullptr;
	}

	UClass* PickupClass = LoadClass<AActor>(nullptr, TEXT("/Game/Loot/BP_LootPickup.BP_LootPickup_C"));
	UWorld* World = DeadEnemy->GetWorld();
	if (!PickupClass || !World)
	{
		UE_LOG(LogNexusAbilityUI, Warning, TEXT("SpawnLootDrop: BP_LootPickup class failed to load"));
		return nullptr;
	}

	const bool bIsGold = FMath::FRand() < GoldShare;
	const FTransform SpawnTransform(DeadEnemy->GetActorLocation() + FVector(0.0f, 0.0f, 50.0f));
	AActor* Pickup = World->SpawnActorDeferred<AActor>(PickupClass, SpawnTransform, nullptr, nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!Pickup)
	{
		return nullptr;
	}

	if (FBoolProperty* GoldProp = CastField<FBoolProperty>(
		FindPropByDisplayName(PickupClass, TEXT("bIsGold"))))
	{
		GoldProp->SetPropertyValue(GoldProp->ContainerPtrToValuePtr<void>(Pickup), bIsGold);
	}

	UGameplayStatics::FinishSpawningActor(Pickup, SpawnTransform);

	// The Blueprint's defaults are the gold-coin look; potions get a glowing orb instead.
	// Restyle must run after FinishSpawningActor: the Blueprint's SCS components do not
	// exist before it. The pickup may already be consumed (spawned on top of the player).
	if (!bIsGold && IsValid(Pickup))
	{
		if (UStaticMeshComponent* MeshComp = Pickup->FindComponentByClass<UStaticMeshComponent>())
		{
			if (UStaticMesh* OrbMesh = LoadObject<UStaticMesh>(nullptr,
				TEXT("/Game/ParagonGreystone/FX/Meshes/Shapes/SM_Sphere_Scale_Unit_100.SM_Sphere_Scale_Unit_100")))
			{
				MeshComp->SetStaticMesh(OrbMesh);
			}
			if (UMaterialInterface* OrbMaterial = LoadObject<UMaterialInterface>(nullptr,
				TEXT("/Game/StarterContent/Materials/M_Tech_Hex_Tile_Pulse_custom.M_Tech_Hex_Tile_Pulse_custom")))
			{
				MeshComp->SetMaterial(0, OrbMaterial);
			}
			MeshComp->SetRelativeScale3D(FVector(0.35f));
		}
	}
	UE_LOG(LogNexusAbilityUI, Log, TEXT("SpawnLootDrop: dropped %s from %s"),
		bIsGold ? TEXT("gold") : TEXT("potion"), *DeadEnemy->GetName());
	return Pickup;
}

namespace
{
	const FName RunInventoryName(TEXT("RunInventory"));
	const FName StashName(TEXT("Stash"));
	const TCHAR* StashSaveSlot = TEXT("NexusStash");

	/** Pawn / controller / player state -> player state. */
	APlayerState* ResolvePlayerState(AActor* PlayerActor)
	{
		if (APlayerState* AsState = Cast<APlayerState>(PlayerActor))
		{
			return AsState;
		}
		if (const APawn* AsPawn = Cast<APawn>(PlayerActor))
		{
			return AsPawn->GetPlayerState();
		}
		if (const AController* AsController = Cast<AController>(PlayerActor))
		{
			return AsController->PlayerState;
		}
		return nullptr;
	}
}

UNarrativeInventoryComponent* UNexusAbilityUILibrary::FindPlayerInventory(AActor* PlayerActor, FName ComponentName)
{
	APlayerState* PlayerState = ResolvePlayerState(PlayerActor);
	if (!PlayerState)
	{
		UE_LOG(LogNexusAbilityUI, Warning, TEXT("FindPlayerInventory: no PlayerState from %s"),
			PlayerActor ? *PlayerActor->GetName() : TEXT("null"));
		return nullptr;
	}

	TArray<UNarrativeInventoryComponent*> Inventories;
	PlayerState->GetComponents<UNarrativeInventoryComponent>(Inventories);

	for (UNarrativeInventoryComponent* Inventory : Inventories)
	{
		if (Inventory && Inventory->GetFName() == ComponentName)
		{
			return Inventory;
		}
	}

	FString Found;
	for (const UNarrativeInventoryComponent* Inventory : Inventories)
	{
		Found += (Found.IsEmpty() ? TEXT("") : TEXT(", "));
		Found += Inventory ? Inventory->GetName() : TEXT("null");
	}
	UE_LOG(LogNexusAbilityUI, Warning, TEXT("FindPlayerInventory: no '%s' on %s (found: %s)"),
		*ComponentName.ToString(), *PlayerState->GetName(), Found.IsEmpty() ? TEXT("none") : *Found);
	return nullptr;
}

bool UNexusAbilityUILibrary::PopulateContainerLoot(AActor* Container, int32 MinGold, int32 MaxGold,
	int32 MinPotions, int32 MaxPotions)
{
	if (!Container || !Container->HasAuthority())
	{
		return false;
	}
	UNarrativeInventoryComponent* ChestInventory = Container->FindComponentByClass<UNarrativeInventoryComponent>();
	if (!ChestInventory)
	{
		UE_LOG(LogNexusAbilityUI, Warning, TEXT("PopulateContainerLoot: %s has no inventory component"),
			*GetNameSafe(Container));
		return false;
	}

	bool bAddedAny = false;

	const int32 GoldAmount = FMath::RandRange(FMath::Max(0, MinGold), FMath::Max(MinGold, MaxGold));
	if (GoldAmount > 0)
	{
		if (UClass* GoldClass = LoadClass<UNarrativeItem>(nullptr,
			TEXT("/Game/Inventory/Items/BP_Item_Gold.BP_Item_Gold_C")))
		{
			ChestInventory->TryAddItemFromClass(GoldClass, GoldAmount, /*bCheckAutoUse*/ false);
			bAddedAny = true;
		}
	}

	const int32 PotionAmount = FMath::RandRange(FMath::Max(0, MinPotions), FMath::Max(MinPotions, MaxPotions));
	if (PotionAmount > 0)
	{
		if (UClass* PotionClass = LoadClass<UNarrativeItem>(nullptr,
			TEXT("/Game/Inventory/Items/BP_Item_HealthPotion.BP_Item_HealthPotion_C")))
		{
			ChestInventory->TryAddItemFromClass(PotionClass, PotionAmount, /*bCheckAutoUse*/ false);
			bAddedAny = true;
		}
	}

	UE_LOG(LogNexusAbilityUI, Log, TEXT("PopulateContainerLoot: %s given %d gold, %d potion(s)"),
		*GetNameSafe(Container), GoldAmount, PotionAmount);
	return bAddedAny;
}

bool UNexusAbilityUILibrary::OpenContainerLoot(AActor* Looter, AActor* Container)
{
	if (!Looter || !Container)
	{
		return false;
	}

	UNarrativeInventoryComponent* ChestInventory = Container->FindComponentByClass<UNarrativeInventoryComponent>();
	if (!ChestInventory)
	{
		UE_LOG(LogNexusAbilityUI, Warning, TEXT("OpenContainerLoot: %s has no inventory component"),
			*GetNameSafe(Container));
		return false;
	}

	// The looting menu queries GetInventoryComponentFromTarget(GetOwningPlayer) == RunInventory,
	// so the loot source must be set on that same component.
	UNarrativeInventoryComponent* RunInventory = FindPlayerInventory(Looter, RunInventoryName);
	if (!RunInventory)
	{
		return false;
	}
	RunInventory->SetLootSource(ChestInventory);

	// Resolve the local player controller from the looter (pawn) to reach its UI component.
	APlayerController* PC = nullptr;
	if (const APawn* Pawn = Cast<APawn>(Looter))
	{
		PC = Cast<APlayerController>(Pawn->GetController());
	}
	else if (APlayerController* AsPC = Cast<APlayerController>(Looter))
	{
		PC = AsPC;
	}
	if (!PC)
	{
		PC = UGameplayStatics::GetPlayerController(Looter, 0);
	}

	UNexusInventoryUIComponent* UI = PC ? PC->FindComponentByClass<UNexusInventoryUIComponent>() : nullptr;
	if (!UI)
	{
		UE_LOG(LogNexusAbilityUI, Warning, TEXT("OpenContainerLoot: no UNexusInventoryUIComponent on %s"),
			*GetNameSafe(PC));
		return false;
	}

	UI->OpenLootMenu();
	UE_LOG(LogNexusAbilityUI, Log, TEXT("OpenContainerLoot: %s looting %s"),
		*GetNameSafe(Looter), *GetNameSafe(Container));
	return true;
}

void UNexusAbilityUILibrary::OpenContainerLootDelayed(AActor* Looter, AActor* Container, float Delay)
{
	if (!Looter || !Container)
	{
		return;
	}

	UWorld* World = Container->GetWorld();
	if (Delay <= 0.f || !World)
	{
		OpenContainerLoot(Looter, Container);
		return;
	}

	TWeakObjectPtr<AActor> WeakLooter(Looter);
	TWeakObjectPtr<AActor> WeakContainer(Container);

	FTimerHandle Handle;
	World->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateLambda([WeakLooter, WeakContainer]()
	{
		if (WeakLooter.IsValid() && WeakContainer.IsValid())
		{
			OpenContainerLoot(WeakLooter.Get(), WeakContainer.Get());
		}
	}), Delay, false);
}

/** The chest's lid pivot, resolved by component name (see the header on why never by index). */
static USceneComponent* FindChestPivot(AActor* Chest, FName PivotName)
{
	if (!Chest)
	{
		return nullptr;
	}

	for (UActorComponent* Component : Chest->GetComponents())
	{
		USceneComponent* Scene = Cast<USceneComponent>(Component);
		if (Scene && Scene->GetFName() == PivotName)
		{
			return Scene;
		}
	}

	// Blueprint SCS components are instanced under their template name, but a renamed or
	// duplicated component can pick up a numeric suffix; match the prefix rather than fail.
	const FString Prefix = PivotName.ToString();
	for (UActorComponent* Component : Chest->GetComponents())
	{
		USceneComponent* Scene = Cast<USceneComponent>(Component);
		if (Scene && Scene->GetName().StartsWith(Prefix))
		{
			return Scene;
		}
	}

	return nullptr;
}

float UNexusAbilityUILibrary::TickChestLid(AActor* Chest, FName PivotName, float CurrentRoll,
	float TargetRoll, float Speed, float DeltaTime)
{
	USceneComponent* Pivot = FindChestPivot(Chest, PivotName);
	if (!Pivot)
	{
		UE_LOG(LogNexusAbilityUI, Warning, TEXT("TickChestLid: %s has no scene component named %s"),
			*GetNameSafe(Chest), *PivotName.ToString());
		return CurrentRoll;
	}

	// FInterpTo eases out (exponential approach) and returns Target outright when Speed <= 0,
	// which is the snap BeginPlay relies on to pose the lid closed on the first frame.
	const float NewRoll = FMath::FInterpTo(CurrentRoll, TargetRoll, DeltaTime, Speed);

	FRotator Rotation = Pivot->GetRelativeRotation();
	Rotation.Roll = NewRoll;
	Pivot->SetRelativeRotation(Rotation);

	return NewRoll;
}

void UNexusAbilityUILibrary::PlayChestOpenFX(AActor* Chest, USoundBase* Sound, UParticleSystem* DustFX,
	FName PivotName, float SoundPitch)
{
	if (!Chest)
	{
		return;
	}

	const USceneComponent* Pivot = FindChestPivot(Chest, PivotName);
	const FVector FXLocation = Pivot ? Pivot->GetComponentLocation() : Chest->GetActorLocation();

	if (Sound)
	{
		UGameplayStatics::PlaySoundAtLocation(Chest, Sound, Chest->GetActorLocation(), 1.f, SoundPitch);
	}

	if (DustFX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(Chest, DustFX, FXLocation, FRotator::ZeroRotator, true);
	}
}

int32 UNexusAbilityUILibrary::TakeAllLoot(AActor* Looter, UNarrativeInventoryComponent* Source,
	FText& OutError, bool& bOutContainerEmpty)
{
	OutError = FText::GetEmpty();
	bOutContainerEmpty = false;

	if (!Source)
	{
		UE_LOG(LogNexusAbilityUI, Warning, TEXT("TakeAllLoot: no loot source"));
		return 0;
	}

	UNarrativeInventoryComponent* RunInventory = FindPlayerInventory(Looter, RunInventoryName);
	if (!RunInventory)
	{
		return 0;
	}

	// GetItems returns the array by value, so consuming stacks below cannot invalidate this.
	const TArray<UNarrativeItem*> LootItems = Source->GetItems();

	int32 TakenCount = 0;
	for (UNarrativeItem* Item : LootItems)
	{
		if (!IsValid(Item) || Item->GetQuantity() <= 0)
		{
			continue;
		}

		// RequestLootItem refuses the whole request unless the quantity wholly fits, so ask only
		// for what there is room for. NoSpaceReason is the plugin's own player-facing wording.
		FText NoSpaceReason;
		const int32 Space = RunInventory->GetSpaceForItem(Item->GetClass(), NoSpaceReason);
		const int32 Quantity = FMath::Min(Item->GetQuantity(), Space);
		if (Quantity <= 0)
		{
			OutError = NoSpaceReason;
			continue;
		}

		FText ErrorText;
		if (RunInventory->RequestLootItem(Item, ErrorText, Quantity))
		{
			TakenCount += Quantity;
		}
		else
		{
			OutError = ErrorText;
		}
	}

	// Anything left behind (did not fit) keeps the menu open; the caller closes on empty.
	bOutContainerEmpty = Source->GetItems().Num() == 0;

	UE_LOG(LogNexusAbilityUI, Log, TEXT("TakeAllLoot: %s took %d item(s) from %s (empty=%s)"),
		*GetNameSafe(Looter), TakenCount, *GetNameSafe(Source),
		bOutContainerEmpty ? TEXT("true") : TEXT("false"));
	return TakenCount;
}

void UNexusAbilityUILibrary::ClearRunInventory(AActor* PlayerActor)
{
	UNarrativeInventoryComponent* RunInventory = FindPlayerInventory(PlayerActor, RunInventoryName);
	if (!RunInventory)
	{
		return;
	}

	// GetItems returns a copy, so removing while iterating is safe.
	const TArray<UNarrativeItem*> Items = RunInventory->GetItems();
	for (UNarrativeItem* Item : Items)
	{
		if (IsValid(Item))
		{
			RunInventory->RemoveItem(Item);
		}
	}
	RunInventory->SetCurrency(0);

	UE_LOG(LogNexusAbilityUI, Log, TEXT("ClearRunInventory: cleared %d item stack(s), %d remain"),
		Items.Num(), RunInventory->GetItems().Num());
}

bool UNexusAbilityUILibrary::ExtractRunInventoryToStash(AActor* PlayerActor)
{
	UNarrativeInventoryComponent* RunInventory = FindPlayerInventory(PlayerActor, RunInventoryName);
	UNarrativeInventoryComponent* Stash = FindPlayerInventory(PlayerActor, StashName);
	if (!RunInventory || !Stash)
	{
		return false;
	}

	bool bMovedEverything = true;

	const TArray<UNarrativeItem*> Items = RunInventory->GetItems();
	for (UNarrativeItem* Item : Items)
	{
		if (!IsValid(Item))
		{
			continue;
		}

		const int32 Quantity = Item->GetQuantity();

		// Add to the stash first, then consume only what the stash accepted: a rejected item
		// stays in the run inventory instead of being destroyed. bCheckAutoUse=false so
		// depositing a consumable does not use it.
		const FItemAddResult AddResult = Stash->TryAddItemFromClass(Item->GetClass(), Quantity, false);

		if (AddResult.AmountGiven > 0)
		{
			RunInventory->ConsumeItem(Item, AddResult.AmountGiven);
		}

		if (AddResult.AmountGiven < Quantity)
		{
			bMovedEverything = false;
			UE_LOG(LogNexusAbilityUI, Warning, TEXT("ExtractRunInventoryToStash: stash took %d/%d of %s (%s)"),
				AddResult.AmountGiven, Quantity, *Item->GetClass()->GetName(), *AddResult.ErrorText.ToString());
		}
	}

	if (const int32 RunCurrency = RunInventory->GetCurrency())
	{
		Stash->AddCurrency(RunCurrency);
		RunInventory->SetCurrency(0);
	}

	if (!Stash->Save(StashSaveSlot))
	{
		UE_LOG(LogNexusAbilityUI, Error, TEXT("ExtractRunInventoryToStash: failed to save slot '%s'"), StashSaveSlot);
		return false;
	}

	UE_LOG(LogNexusAbilityUI, Log, TEXT("ExtractRunInventoryToStash: stash now %d stack(s), %d currency (complete: %s)"),
		Stash->GetItems().Num(), Stash->GetCurrency(), bMovedEverything ? TEXT("yes") : TEXT("no"));
	return bMovedEverything;
}

bool UNexusAbilityUILibrary::LoadStash(AActor* PlayerActor)
{
	UNarrativeInventoryComponent* Stash = FindPlayerInventory(PlayerActor, StashName);
	if (!Stash)
	{
		return false;
	}

	// Load() returns false for a missing slot without touching the inventory (first run).
	const bool bLoaded = Stash->Load(StashSaveSlot);
	UE_LOG(LogNexusAbilityUI, Log, TEXT("LoadStash: %s (%d stack(s), %d currency)"),
		bLoaded ? TEXT("loaded") : TEXT("no save yet"), Stash->GetItems().Num(), Stash->GetCurrency());
	return bLoaded;
}

/** Sky Crusher only finishes wounded enemies. */
static constexpr float SkyCrusherHealthThreshold = 0.5f;

bool UNexusAbilityUILibrary::CanSkyCrusherExecute(UGameplayAbility* Ability, AActor*& OutTarget)
{
	OutTarget = nullptr;

	if (!Ability)
	{
		UE_LOG(LogNexusAbilityUI, Warning, TEXT("CanSkyCrusherExecute: no ability"));
		return false;
	}

	const AActor* Avatar = Ability->GetAvatarActorFromActorInfo();
	if (!Avatar)
	{
		UE_LOG(LogNexusAbilityUI, Warning, TEXT("CanSkyCrusherExecute: ability has no avatar actor"));
		return false;
	}

	const UNexusLockOnComponent* LockOn = Avatar->FindComponentByClass<UNexusLockOnComponent>();
	if (!LockOn)
	{
		UE_LOG(LogNexusAbilityUI, Warning, TEXT("CanSkyCrusherExecute: %s has no lock-on component"), *Avatar->GetName());
		return false;
	}

	AActor* Target = LockOn->GetLockedTarget();
	if (!Target)
	{
		UE_LOG(LogNexusAbilityUI, Log, TEXT("CanSkyCrusherExecute: rejected, no lock-on target"));
		return false;
	}

	// The boss is immune to the finisher, so the cinematic can never see one.
	static const FSoftClassPath BossClassPath(TEXT("/Game/Enemies/BossEnemy/BP_Enemy_Boss.BP_Enemy_Boss_C"));
	if (const UClass* BossClass = BossClassPath.TryLoadClass<AActor>())
	{
		if (Target->IsA(BossClass))
		{
			UE_LOG(LogNexusAbilityUI, Log, TEXT("CanSkyCrusherExecute: rejected, target %s is the boss"), *Target->GetName());
			return false;
		}
	}
	else
	{
		// Fail open: an unresolvable boss class must not disable the ability outright.
		UE_LOG(LogNexusAbilityUI, Warning, TEXT("CanSkyCrusherExecute: could not load %s, skipping boss check"), *BossClassPath.ToString());
	}

	const IAbilitySystemInterface* TargetASI = Cast<IAbilitySystemInterface>(Target);
	UAbilitySystemComponent* TargetASC = TargetASI ? TargetASI->GetAbilitySystemComponent() : nullptr;
	if (!TargetASC)
	{
		UE_LOG(LogNexusAbilityUI, Log, TEXT("CanSkyCrusherExecute: rejected, target %s has no ability system"), *Target->GetName());
		return false;
	}

	const UBasicAttributeSet* Attributes = TargetASC->GetSet<UBasicAttributeSet>();
	if (!Attributes)
	{
		UE_LOG(LogNexusAbilityUI, Log, TEXT("CanSkyCrusherExecute: rejected, target %s has no BasicAttributeSet"), *Target->GetName());
		return false;
	}

	const float MaxHealth = Attributes->GetMaxHealth();
	if (MaxHealth <= 0.f)
	{
		UE_LOG(LogNexusAbilityUI, Warning, TEXT("CanSkyCrusherExecute: rejected, target %s has MaxHealth %.1f"), *Target->GetName(), MaxHealth);
		return false;
	}

	const float HealthFraction = Attributes->GetHealth() / MaxHealth;
	if (HealthFraction >= SkyCrusherHealthThreshold)
	{
		UE_LOG(LogNexusAbilityUI, Log, TEXT("CanSkyCrusherExecute: rejected, target %s at %.0f%% health"), *Target->GetName(), HealthFraction * 100.f);
		return false;
	}

	UE_LOG(LogNexusAbilityUI, Log, TEXT("CanSkyCrusherExecute: accepted target %s at %.0f%% health"), *Target->GetName(), HealthFraction * 100.f);
	OutTarget = Target;
	return true;
}

void UNexusAbilityUILibrary::ArmSkyCrusher(UGameplayAbility* Ability, AActor* Target)
{
	UNexusFinisherDirector::Arm(Ability, Target, FNexusFinisherProfile::SkyCrusher());
}

void UNexusAbilityUILibrary::ArmBurden(UGameplayAbility* Ability, AActor* Target)
{
	UNexusFinisherDirector::Arm(Ability, Target, FNexusFinisherProfile::Burden());
}

void UNexusAbilityUILibrary::RestoreFromCinematic(UGameplayAbility* Ability)
{
	UNexusFinisherDirector::Restore(Ability);
}
