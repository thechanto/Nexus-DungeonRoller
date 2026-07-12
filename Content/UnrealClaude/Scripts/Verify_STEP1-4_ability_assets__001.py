import unreal, re
EAL = unreal.EditorAssetLibrary
AL = unreal.AnimationLibrary

print("VERIFY|BEGIN")

montages = [
    "/Game/RetargetedAbilities/Warrior/AM_ShieldSlam",
    "/Game/RetargetedAbilities/Warrior/AM_SwordSweep",
    "/Game/RetargetedAbilities/Warrior/AM_LeapSlash",
    "/Game/RetargetedAbilities/Warrior/AM_SkyCrusher",
    "/Game/RetargetedAbilities/Mage/AM_ArcaneBolt",
    "/Game/RetargetedAbilities/Mage/AM_CosmicRift",
    "/Game/RetargetedAbilities/Mage/AM_Meteor",
    "/Game/RetargetedAbilities/Mage/AM_Burden",
]
for p in montages:
    try:
        m = EAL.load_asset(p)
        if not m:
            print("VERIFY|MONTAGE|%s|MISSING" % p)
            continue
        parts = []
        try:
            evs = AL.get_animation_notify_events(m)
            for ev in evs:
                nm = "?"
                for prop in ("notify", "notify_state_class"):
                    try:
                        v = ev.get_editor_property(prop)
                        if v:
                            nm = type(v).__name__
                            break
                    except Exception:
                        pass
                parts.append(nm)
        except Exception as ex:
            parts.append("notify-api-err:%s" % ex)
        print("VERIFY|MONTAGE|%s|OK|notifies=%s" % (p, ",".join(parts) if parts else "NONE"))
    except Exception as ex:
        print("VERIFY|MONTAGE|%s|EXC|%s" % (p, ex))

gas = [
    ("GA_ShieldSlam", "AttackMontage", "BaseDamage"),
    ("GA_SwordSweep", "AttackMontage", "BaseDamage"),
    ("GA_LeapSlash", "AttackMontage", "BaseDamage"),
    ("GA_SkyCrusher", "AttackMontage", "BaseDamage"),
    ("GA_ArcaneBolt", "ShootMontage", "DamageMagnitude"),
    ("GA_CosmicRift", "ShootMontage", "DamageMagnitude"),
    ("GA_Meteor", "ShootMontage", "DamageMagnitude"),
    ("GA_Burden", "ShootMontage", "DamageMagnitude"),
]
def gp(o, n):
    try:
        v = o.get_editor_property(n)
        if isinstance(v, unreal.Object):
            return v.get_name()
        return str(v)
    except Exception as ex:
        return "ERR"
for name, mprop, dprop in gas:
    try:
        cls = unreal.load_object(None, "/Game/GameplayAbilitySystem/Abilities/%s.%s_C" % (name, name))
        if not cls:
            print("VERIFY|GA|%s|MISSING" % name)
            continue
        cdo = unreal.get_default_object(cls)
        print("VERIFY|GA|%s|montage=%s|dmg=%s|inputid=%s|proj=%s" % (
            name, gp(cdo, mprop), gp(cdo, dprop), gp(cdo, "AbilityInputID"), gp(cdo, "ProjectileClass")))
    except Exception as ex:
        print("VERIFY|GA|%s|EXC|%s" % (name, ex))

das = [
    ("Warrior", "DA_Ability_Warrior_ShieldSlam"),
    ("Warrior", "DA_Ability_Warrior_SwordSweep"),
    ("Warrior", "DA_Ability_Warrior_LeapSlash"),
    ("Warrior", "DA_Ability_Warrior_SkyCrusher"),
    ("Mage", "DA_Ability_Mage_ArcaneBolt"),
    ("Mage", "DA_Ability_Mage_CosmicRift"),
    ("Mage", "DA_Ability_Mage_Meteor"),
    ("Mage", "DA_Ability_Mage_Burden"),
]
for sub, name in das:
    try:
        cls = unreal.load_object(None, "/Game/GameplayAbilitySystem/Abilities/DataAssets/%s/%s.%s_C" % (sub, name, name))
        if not cls:
            print("VERIFY|DA|%s|MISSING" % name)
            continue
        cdo = unreal.get_default_object(cls)
        sd = cdo.get_editor_property("AbilityData")
        txt = sd.export_text()
        mobj = re.search(r"AbilityClass[^=\(]*=([^,\)]+)", txt)
        print("VERIFY|DA|%s|abilityclass=%s" % (name, mobj.group(1)[:140] if mobj else "EMPTY(%s...)" % txt[:100]))
    except Exception as ex:
        print("VERIFY|DA|%s|EXC|%s" % (name, ex))

print("VERIFY|END")
