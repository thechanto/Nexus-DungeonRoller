import unreal

DA_PATH = "/Game/GameplayAbilitySystem/Abilities/DataAssets/Warrior/DA_Ability_Warrior_SkyCrusher"
LIB = unreal.NexusAbilityUILibrary

world = unreal.UnrealEditorSubsystem().get_editor_world()
print("WORLD:", world)

da_cls = unreal.EditorAssetLibrary.load_blueprint_class(DA_PATH)
print("DA CLASS:", da_cls)

# --- BEFORE ---
before = LIB.get_assigned_abilities(world)
print("SLOTS BEFORE:", list(before))
unlocked_before = LIB.is_ability_unlocked(world, da_cls)
print("UNLOCKED BEFORE:", unlocked_before)

# AssignAbilityToKeybindSlot refuses abilities that are not in UnlockedAbilities.
if not unlocked_before:
    ok_unlock = LIB.unlock_ability(world, da_cls)
    print("UNLOCK CALLED ->", ok_unlock)
    print("UNLOCKED NOW:", LIB.is_ability_unlocked(world, da_cls))

# --- ASSIGN to keybind slot 4 (key "Four" -> IA_Ability4 -> EAbilityInputID::Ability4) ---
ok = LIB.assign_ability_to_keybind_slot(world, da_cls, 4)
print("ASSIGN slot 4 ->", ok)

# --- VERIFY by re-reading from the save ---
after = LIB.get_assigned_abilities(world)
print("SLOTS AFTER:", list(after))
print("SLOT_FOR_ABILITY:", LIB.get_assigned_slot_for_ability(world, da_cls))
