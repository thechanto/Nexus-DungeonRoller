import unreal
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
lib = unreal.NexusAbilityUILibrary
sky = unreal.load_object(None, "/Game/GameplayAbilitySystem/Abilities/DataAssets/Warrior/DA_Ability_Warrior_SkyCrusher.DA_Ability_Warrior_SkyCrusher_C")
ok = lib.assign_ability_to_slot(world, sky, 1)
print("KB8 restore SkyCrusher slot1 ok=%s assigned=%s" % (ok, [s.split(".")[-1] for s in lib.get_assigned_abilities(world)]))
print("KB8 DONE")