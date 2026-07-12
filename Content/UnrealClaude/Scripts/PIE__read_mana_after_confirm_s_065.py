import unreal
w = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
player = None
for p in unreal.GameplayStatics.get_all_actors_of_class(w, unreal.NexusCharacterBase):
    if "BP_NexusPlayer" in p.get_class().get_name():
        player = p
asc = player.get_component_by_class(unreal.AbilitySystemComponent)
aset = asc.call_method("GetAttributeSet", args=(unreal.BasicAttributeSet,))
print("MANA AFTER CONFIRM: %s" % aset.get_editor_property("mana"))
shoot_cls = unreal.load_class(None, "/Game/GameplayAbilitySystem/Abilities/GA_ShootProjectile_Ice.GA_ShootProjectile_Ice_C")
print("Shoot after aim ended: %s (expect True)" % asc.call_method("TryActivateAbilityByClass", args=(shoot_cls, False)))
print("PIE5 DONE")
