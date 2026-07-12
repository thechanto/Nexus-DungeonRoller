import unreal

lines = []

def montage_info(path):
    m = unreal.EditorAssetLibrary.load_asset(path)
    lines.append("== MONTAGE %s" % path)
    try:
        skel = m.get_editor_property("skeleton").get_path_name()
    except Exception:
        skel = "?"
    lines.append("  PlayLength=%.3f Skeleton=%s" % (m.get_play_length(), skel))
    for tr in m.get_editor_property("slot_anim_tracks"):
        lines.append("  SLOT=%s" % tr.get_editor_property("slot_name"))
    evs = m.get_editor_property("notifies")
    lines.append("  NumNotifies=%d" % len(evs))
    for ev in evs:
        try:
            txt = str(ev.export_text())
        except Exception as e:
            txt = "EXPORT_FAIL %s" % e
        lines.append("  NOTIFY: %s" % txt[:800])

montage_info("/Game/Animations/Axe/Montage_MeleeAttack_AxeSwing")
montage_info("/Game/Animations/Staff/Montage_Staff_ShootProjectile")

def ga_info(path, props):
    cls = unreal.EditorAssetLibrary.load_blueprint_class(path)
    cdo = unreal.get_default_object(cls)
    lines.append("== GA %s" % path)
    for p in props:
        try:
            v = cdo.get_editor_property(p)
            lines.append("  %s = %s" % (p, v))
        except Exception as e:
            lines.append("  %s ERR %s" % (p, str(e)[:120]))

common = ["ability_input_id", "should_show_in_abilities_bar", "auto_activate_when_granted",
          "cooldown_gameplay_effect_class", "cost_gameplay_effect_class", "ability_tags",
          "RequiresEquippedWeapon"]
melee_props = ["AttackMontage", "DamageEffectClass", "BaseDamage", "BaseDamageMultiplied",
               "DamagePercentIncreasePerLevel"] + common
ga_info("/Game/GameplayAbilitySystem/Abilities/GA_MeleeAttack_AxeSwing", melee_props)
ga_info("/Game/GameplayAbilitySystem/Abilities/GA_MeleeAttack_Base", melee_props)
proj_props = ["ShootMontage", "DamageEffectClass", "DamageMagnitude", "ProjectileClass",
              "ProjectileSpeed", "DamagePercentIncreasePerLevel"] + common
ga_info("/Game/GameplayAbilitySystem/Abilities/GA_ShootProjectile_Ice", proj_props)

anims = [
    "/Game/RetargetedAbilities/Warrior/Ability_Q",
    "/Game/RetargetedAbilities/Warrior/Ability_E",
    "/Game/RetargetedAbilities/Warrior/Ability_R",
    "/Game/RetargetedAbilities/Warrior/Ability_Ultimate",
    "/Game/RetargetedAbilities/Mage/RMB_Cast",
    "/Game/RetargetedAbilities/Mage/CosmicRift",
    "/Game/RetargetedAbilities/Mage/Cosmic_Rift",
    "/Game/RetargetedAbilities/Mage/Burden_slow",
]
for a in anims:
    s = unreal.EditorAssetLibrary.load_asset(a)
    try:
        skel = s.get_editor_property("skeleton").get_path_name()
    except Exception:
        skel = "?"
    lines.append("ANIM %s len=%.3f skel=%s" % (a, s.get_play_length(), skel))

print("INSPECT_BEGIN")
for l in lines:
    print(l)
print("INSPECT_END")
