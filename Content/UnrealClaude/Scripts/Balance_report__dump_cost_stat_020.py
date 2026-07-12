import unreal

def ge_dump(bp_path):
    name = bp_path.split("/")[-1]
    cls = unreal.load_object(None, bp_path + "." + name + "_C")
    if not cls:
        print("[GE] %s: CLASS NOT FOUND" % name)
        return
    cdo = unreal.get_default_object(cls)
    try:
        dur = cdo.get_editor_property("duration_policy")
    except Exception as e:
        dur = "ERR:%s" % e
    print("[GE] %s duration=%s" % (name, dur))
    try:
        mods = cdo.get_editor_property("modifiers")
        for i, m in enumerate(mods):
            try:
                attr = m.get_editor_property("attribute")
                op = m.get_editor_property("modifier_op")
                mag = m.get_editor_property("modifier_magnitude")
                print("  mod[%d] attr=%s op=%s mag=%s" % (i, attr.export_text(), op, mag.export_text()))
            except Exception as e:
                print("  mod[%d] ERR %s" % (i, e))
    except Exception as e:
        print("  modifiers ERR %s" % e)
    try:
        execs = cdo.get_editor_property("executions")
        for i, ex in enumerate(execs):
            print("  exec[%d] %s" % (i, ex.export_text()))
    except Exception as e:
        pass

def ga_dump(bp_path):
    name = bp_path.split("/")[-1]
    cls = unreal.load_object(None, bp_path + "." + name + "_C")
    if not cls:
        print("[GA] %s: CLASS NOT FOUND" % name)
        return
    cdo = unreal.get_default_object(cls)
    out = ["[GA] %s" % name]
    for prop, label in [
        ("ability_input_id", "InputID"),
        ("cost_gameplay_effect_class", "CostGE"),
        ("cooldown_gameplay_effect_class", "CooldownGE"),
    ]:
        try:
            v = cdo.get_editor_property(prop)
            if hasattr(v, "get_name"):
                v = v.get_name()
            out.append("%s=%s" % (label, v))
        except Exception:
            pass
    for prop in ["BaseDamage", "DamagePercentIncreasePerLevel", "DamageMagnitude",
                 "ProjectileSpeed", "DamageEffectClass", "RequiresEquippedWeapon",
                 "AttackMontage", "ShootMontage", "ProjectileClass", "Cost", "StaminaCost", "ManaCost"]:
        try:
            v = cdo.get_editor_property(prop)
            if hasattr(v, "get_name"):
                v = v.get_name()
            out.append("%s=%s" % (prop, v))
        except Exception:
            pass
    print(" ".join(out))

print("=== COST / STAT / DAMAGE GE DUMP ===")
for p in [
    "/Game/GameplayAbilitySystem/Effects/GE_Cost_Stamina",
    "/Game/GameplayAbilitySystem/Effects/GE_Cost_Mana",
    "/Game/GameplayAbilitySystem/Effects/GE_Dash_Cost",
    "/Game/GameplayAbilitySystem/Effects/GE_SetStats",
    "/Game/GameplayAbilitySystem/Effects/GE_Damage_Instant",
    "/Game/GameplayAbilitySystem/Effects/GE_DefaultAttributes_Enemy",
    "/Game/GameplayAbilitySystem/Effects/GE_DefaultAttributes_Boss",
    "/Game/GameplayAbilitySystem/Effects/GE_UpgradeStat_Vitality",
    "/Game/GameplayAbilitySystem/Effects/GE_UpgradeStat_Endurance",
    "/Game/GameplayAbilitySystem/Effects/GE_UpgradeStat_Mana",
]:
    ge_dump(p)

print("=== GA CDO DUMP ===")
for p in [
    "/Game/GameplayAbilitySystem/Abilities/GA_MeleeAttack_AxeSwing",
    "/Game/GameplayAbilitySystem/Abilities/GA_MeleeAttack_AxeCombo",
    "/Game/GameplayAbilitySystem/Abilities/GA_MeleeAttack_Base",
    "/Game/GameplayAbilitySystem/Abilities/GA_ShootProjectile_Ice",
    "/Game/GameplayAbilitySystem/Abilities/GA_ShootProjectile_GreenFire",
    "/Game/GameplayAbilitySystem/Abilities/GA_ShootProjectile_Base",
    "/Game/GameplayAbilitySystem/Abilities/GA_AOEAttack",
    "/Game/GameplayAbilitySystem/Abilities/GA_Dash",
    "/Game/GameplayAbilitySystem/Abilities/GA_Shield",
    "/Game/GameplayAbilitySystem/Abilities/GA_ShieldSlam",
    "/Game/GameplayAbilitySystem/Abilities/GA_SwordSweep",
    "/Game/GameplayAbilitySystem/Abilities/GA_LeapSlash",
    "/Game/GameplayAbilitySystem/Abilities/GA_SkyCrusher",
    "/Game/GameplayAbilitySystem/Abilities/GA_ArcaneBolt",
    "/Game/GameplayAbilitySystem/Abilities/GA_CosmicRift",
    "/Game/GameplayAbilitySystem/Abilities/GA_Meteor",
    "/Game/GameplayAbilitySystem/Abilities/GA_Burden",
]:
    ga_dump(p)

print("=== ENEMY DUMP ===")
for p in ["/Game/Enemies/BP_Enemy_Base", "/Game/Enemies/MeleeEnemy/BP_Enemy_Melee",
          "/Game/Enemies/MageEnemy/BP_Enemy_Mage", "/Game/Enemies/BP_Enemy_Ranged",
          "/Game/GameplayAbilitySystem/Characters/BP_NexusEnemy_Base",
          "/Game/GameplayAbilitySystem/Characters/BP_NexusPlayer"]:
    name = p.split("/")[-1]
    cls = unreal.load_object(None, p + "." + name + "_C")
    if not cls:
        print("[EN] %s: CLASS NOT FOUND" % name)
        continue
    cdo = unreal.get_default_object(cls)
    out = ["[EN] %s" % name]
    for prop in ["starting_abilities", "StartingAbilities"]:
        try:
            v = cdo.get_editor_property(prop)
            out.append("StartingAbilities=[%s]" % ",".join([c.get_name() if c else "None" for c in v]))
            break
        except Exception:
            pass
    for prop in ["BaseDamage", "Damage", "AttackDamage", "DefaultAttributesEffect", "DefaultAttributes", "StartingEffects", "StartupEffects"]:
        try:
            v = cdo.get_editor_property(prop)
            if hasattr(v, "get_name"):
                v = v.get_name()
            out.append("%s=%s" % (prop, v))
        except Exception:
            pass
    print(" ".join(out))
print("=== DONE balance dump ===")
