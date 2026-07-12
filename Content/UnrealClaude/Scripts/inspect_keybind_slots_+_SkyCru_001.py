import unreal

log = []

try:
    world = unreal.EditorLevelLibrary.get_editor_world()
    assigned = unreal.NexusAbilityUILibrary.get_assigned_abilities(world)
    log.append("ASSIGNED KEYBIND SLOTS (count=%d):" % len(assigned))
    for i, a in enumerate(assigned):
        log.append("  slot index %d (key '%d'): %s" % (i, i + 1, a if a else "<EMPTY>"))
except Exception as e:
    log.append("assigned read FAILED: %r" % (e,))

try:
    ga = unreal.EditorAssetLibrary.load_blueprint_class("/Game/GameplayAbilitySystem/Abilities/GA_SkyCrusher")
    cdo = unreal.get_default_object(ga)
    for p in ["ability_input_id", "should_show_in_abilities_bar", "auto_activate_when_granted"]:
        try:
            log.append("GA_SkyCrusher.%s = %r" % (p, cdo.get_editor_property(p)))
        except Exception as e:
            log.append("GA_SkyCrusher.%s ERR %r" % (p, e))
except Exception as e:
    log.append("GA load FAILED: %r" % (e,))

try:
    dac = unreal.EditorAssetLibrary.load_blueprint_class("/Game/GameplayAbilitySystem/Abilities/DataAssets/Warrior/DA_Ability_Warrior_SkyCrusher")
    dcdo = unreal.get_default_object(dac)
    log.append("---- DA_Ability_Warrior_SkyCrusher CDO ----")
    log.append(dcdo.export_text()[:2000])
except Exception as e:
    log.append("DA export FAILED: %r" % (e,))

print("\n".join(log))
