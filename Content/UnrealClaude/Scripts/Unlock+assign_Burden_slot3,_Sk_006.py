import unreal
lib = unreal.NexusAbilityUILibrary
world = unreal.UnrealEditorSubsystem().get_editor_world()
print("world:", world)

def dacls(folder, name):
    p = "/Game/GameplayAbilitySystem/Abilities/DataAssets/%s/%s.%s_C" % (folder, name, name)
    return unreal.load_object(None, p)

burden = dacls("Mage", "DA_Ability_Mage_Burden")
sky    = dacls("Warrior", "DA_Ability_Warrior_SkyCrusher")
print("burden cls:", burden)
print("sky    cls:", sky)

print("unlock burden:", lib.unlock_ability(world, burden))
print("unlock sky   :", lib.unlock_ability(world, sky))
print("is_unlocked burden:", lib.is_ability_unlocked(world, burden))
print("is_unlocked sky   :", lib.is_ability_unlocked(world, sky))

# keybind slot 3 -> index 2, slot 4 -> index 3
print("assign burden idx2:", lib.assign_ability_to_slot(world, burden, 2))
print("assign sky    idx3:", lib.assign_ability_to_slot(world, sky, 3))

print("ASSIGNED:", list(lib.get_assigned_abilities(world)))
