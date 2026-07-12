import unreal

out = unreal.Paths.project_dir() + "_ai/t3d"
at = unreal.AssetToolsHelpers.get_asset_tools()

mont = [
    "/Game/RetargetedAbilities/Warrior/AM_SwordSweep.AM_SwordSweep",
    "/Game/RetargetedAbilities/Warrior/AM_ShieldSlam.AM_ShieldSlam",
    "/Game/RetargetedAbilities/Warrior/AM_LeapSlash.AM_LeapSlash",
]
at.export_assets(mont, out)

for m in mont:
    a = unreal.load_object(None, m)
    unreal.log("MONT %s len=%.3f" % (a.get_name(), a.get_editor_property("sequence_length")))

# Which montage does each warrior GA use? (CDO AttackMontage)
for g in ["GA_SkyCrusher", "GA_SwordSweep", "GA_ShieldSlam", "GA_LeapSlash",
          "GA_MeleeAttack_AxeSwing"]:
    c = unreal.load_object(None, "/Game/GameplayAbilitySystem/Abilities/%s.%s_C" % (g, g))
    if not c:
        unreal.log("GA %s -> class not found" % g)
        continue
    cdo = unreal.get_default_object(c)
    try:
        am = cdo.get_editor_property("AttackMontage")
        unreal.log("GA %s -> montage=%s" % (g, am.get_name() if am else "None"))
    except Exception as e:
        unreal.log("GA %s -> ERR %s" % (g, e))

unreal.log("DONE3")
