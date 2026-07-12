import unreal

lines = []

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

export_dir = "C:/Users/chant/OneDrive/Documents/Unreal Projects/Nexus-part-final 5.7/Saved/ClaudeScripts/export"
at = unreal.AssetToolsHelpers.get_asset_tools()
at.export_assets([
    "/Game/Animations/Axe/Montage_MeleeAttack_AxeSwing",
    "/Game/Animations/Staff/Montage_Staff_ShootProjectile",
], export_dir)
lines.append("EXPORTED montages to %s" % export_dir)

print("INSPECT_BEGIN")
for l in lines:
    print(l)
print("INSPECT_END")
