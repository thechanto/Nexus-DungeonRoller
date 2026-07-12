import unreal
EAL = unreal.EditorAssetLibrary
def dump_ga(path):
    cls = EAL.load_blueprint_class(path)
    cdo = unreal.get_default_object(cls)
    print("=== " + path.split("/")[-1] + " ===")
    for prop in ["ability_tags","activation_owned_tags","activation_blocked_tags","activation_required_tags","block_abilities_with_tag","cancel_abilities_with_tag","cost_gameplay_effect_class"]:
        try:
            v = cdo.get_editor_property(prop)
            print("  %s = %s" % (prop, v))
        except Exception as e:
            print("  %s = <ERR %s>" % (prop, e))
for p in ["/Game/GameplayAbilitySystem/Abilities/GA_AOEAttack",
          "/Game/GameplayAbilitySystem/Abilities/GA_ShootProjectile_Ice",
          "/Game/GameplayAbilitySystem/Abilities/GA_ShootProjectile_GreenFire"]:
    try:
        dump_ga(p)
    except Exception as e:
        print("GA DUMP FAIL %s: %s" % (p, e))
# cost GE modifier text via C++ helper (reflection call)
lib = unreal.get_default_object(unreal.NexusAbilityUILibrary)
for gep in ["/Game/GameplayAbilitySystem/Effects/GE_Cost_Mana",
            "/Game/GameplayAbilitySystem/Effects/GE_Cost_Mana_Heavy",
            "/Game/GameplayAbilitySystem/Effects/GE_Cost_Mana_Ability"]:
    try:
        gcls = EAL.load_blueprint_class(gep)
        txt = lib.call_method("GetClassDefaultPropertyText", args=(gcls, "Modifiers"))
        print("=== %s Modifiers ===" % gep.split("/")[-1])
        print(str(txt)[:1200])
    except Exception as e:
        print("GE DUMP FAIL %s: %s" % (gep, e))
print("DBG1 DONE")
