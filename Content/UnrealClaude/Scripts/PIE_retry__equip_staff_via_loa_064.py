import unreal
w = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
player = None
for p in unreal.GameplayStatics.get_all_actors_of_class(w, unreal.NexusCharacterBase):
    if "BP_NexusPlayer" in p.get_class().get_name():
        player = p
wm = None
for c in player.get_components_by_class(unreal.ActorComponent):
    if "WeaponsManager" in c.get_class().get_name():
        wm = c
staff_cls = unreal.load_class(None, "/Game/Weapons/BP_Weapon_Staff.BP_Weapon_Staff_C")
print("STAFF CLASS: %s" % staff_cls)
wm.call_method("GiveWeapon", args=(staff_cls,))
wm.call_method("EquipWeapon", args=(staff_cls,))
print("EQUIPPED: %s" % wm.get_editor_property("EquippedWeapon"))
asc = player.get_component_by_class(unreal.AbilitySystemComponent)
aset = asc.call_method("GetAttributeSet", args=(unreal.BasicAttributeSet,))
print("MANA BEFORE: %s" % aset.get_editor_property("mana"))
aoe_cls = unreal.load_class(None, "/Game/GameplayAbilitySystem/Abilities/GA_AOEAttack.GA_AOEAttack_C")
shoot_cls = unreal.load_class(None, "/Game/GameplayAbilitySystem/Abilities/GA_ShootProjectile_Ice.GA_ShootProjectile_Ice_C")
print("AOE aim activate: %s" % asc.call_method("TryActivateAbilityByClass", args=(aoe_cls, False)))
print("Shoot while aiming: %s  <- LMB primary during aim (expect False, tag-blocked)" % asc.call_method("TryActivateAbilityByClass", args=(shoot_cls, False)))
asc.call_method("TargetConfirm")
print("TargetConfirm sent (V-key path)")
print("PIE4 DONE")
