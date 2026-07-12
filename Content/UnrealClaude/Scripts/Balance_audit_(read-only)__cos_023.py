import unreal

def dump_ge(path):
    cls = unreal.load_blueprint_class(path)
    if not cls:
        print(f"[AUDIT] MISSING GE: {path}")
        return
    cdo = unreal.get_default_object(cls)
    mods = cdo.get_editor_property("modifiers")
    print(f"[AUDIT] === {path.split('/')[-1]} : {len(mods)} modifier(s)")
    for i, m in enumerate(mods):
        try:
            txt = m.export_text()
        except Exception as e:
            txt = f"<export failed: {e}>"
        print(f"[AUDIT]   [{i}] {txt}")

def dump_ga_cost(path):
    cls = unreal.load_blueprint_class(path)
    if not cls:
        print(f"[AUDIT] MISSING GA: {path}")
        return
    cdo = unreal.get_default_object(cls)
    cost = cdo.get_editor_property("cost_gameplay_effect_class")
    print(f"[AUDIT] GA {path.split('/')[-1]} -> CostGE = {cost.get_name() if cost else 'None'}")

FX = "/Game/GameplayAbilitySystem/Effects/"
AB = "/Game/GameplayAbilitySystem/Abilities/"

for ge in ["GE_Cost_Stamina", "GE_Cost_Mana", "GE_SetStats"]:
    dump_ge(FX + ge + "." + ge)

for ga in ["GA_MeleeAttack_AxeCombo", "GA_MeleeAttack_AxeSwing",
           "GA_ShootProjectile_Ice", "GA_ShootProjectile_GreenFire", "GA_AOEAttack",
           "GA_ShieldSlam", "GA_SwordSweep", "GA_LeapSlash", "GA_SkyCrusher",
           "GA_ArcaneBolt", "GA_CosmicRift", "GA_Meteor", "GA_Burden"]:
    dump_ga_cost(AB + ga + "." + ga)

print("[AUDIT] done")
