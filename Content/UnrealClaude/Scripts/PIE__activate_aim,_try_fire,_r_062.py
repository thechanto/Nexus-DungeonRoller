import unreal
w = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
print("GAME WORLD: %s" % (w.get_name() if w else None))
pawns = unreal.GameplayStatics.get_all_actors_of_class(w, unreal.NexusCharacterBase)
player = None
for p in pawns:
    if "BP_NexusPlayer" in p.get_class().get_name():
        player = p
print("PLAYER: %s" % player)
asc = player.get_component_by_class(unreal.AbilitySystemComponent)
# mana before
for aset in [asc.call_method("GetAttributeSet", args=(unreal.BasicAttributeSet,))]:
    print("MANA BEFORE: %s / %s" % (aset.get_editor_property("mana"), aset.get_editor_property("max_mana")))
# equipped weapon
wm = None
for c in player.get_components_by_class(unreal.ActorComponent):
    if "WeaponsManager" in c.get_class().get_name():
        wm = c
print("WEAPON: %s" % (wm.get_editor_property("EquippedWeapon") if wm else "no manager"))
aoe_cls = unreal.EditorAssetLibrary.load_blueprint_class("/Game/GameplayAbilitySystem/Abilities/GA_AOEAttack")
shoot_cls = unreal.EditorAssetLibrary.load_blueprint_class("/Game/GameplayAbilitySystem/Abilities/GA_ShootProjectile_Ice")
r1 = asc.call_method("TryActivateAbilityByClass", args=(aoe_cls, False))
print("AOE (aim) activate: %s" % r1)
r2 = asc.call_method("TryActivateAbilityByClass", args=(shoot_cls, False))
print("ShootProjectile while aiming: %s (expect False = blocked by tag)" % r2)
print("PIE2 DONE")
