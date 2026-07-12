import unreal

at = unreal.AssetToolsHelpers.get_asset_tools()
eal = unreal.EditorAssetLibrary

jobs = [
    ("/Game/RetargetedAbilities/Warrior/Ability_Q",        "/Game/RetargetedAbilities/Warrior", "AM_ShieldSlam"),
    ("/Game/RetargetedAbilities/Warrior/Ability_E",        "/Game/RetargetedAbilities/Warrior", "AM_SwordSweep"),
    ("/Game/RetargetedAbilities/Warrior/Ability_R",        "/Game/RetargetedAbilities/Warrior", "AM_LeapSlash"),
    ("/Game/RetargetedAbilities/Warrior/Ability_Ultimate", "/Game/RetargetedAbilities/Warrior", "AM_SkyCrusher"),
    ("/Game/RetargetedAbilities/Mage/RMB_Cast",            "/Game/RetargetedAbilities/Mage",    "AM_ArcaneBolt"),
    ("/Game/RetargetedAbilities/Mage/CosmicRift",          "/Game/RetargetedAbilities/Mage",    "AM_CosmicRift"),
    ("/Game/RetargetedAbilities/Mage/Cosmic_Rift",         "/Game/RetargetedAbilities/Mage",    "AM_Meteor"),
    ("/Game/RetargetedAbilities/Mage/Burden_slow",         "/Game/RetargetedAbilities/Mage",    "AM_Burden"),
]

results = []
for src, pkg, name in jobs:
    full = "%s/%s" % (pkg, name)
    try:
        if eal.does_asset_exist(full):
            m = eal.load_asset(full)
            results.append("EXISTS %s len=%.3f" % (full, m.get_play_length()))
            continue
        anim = eal.load_asset(src)
        if anim is None:
            results.append("FAIL_LOAD_SRC %s" % src)
            continue
        f = unreal.AnimMontageFactory()
        try:
            f.set_editor_property("source_animation", anim)
        except Exception as e:
            results.append("WARN source_animation: %s" % str(e)[:100])
        try:
            f.set_editor_property("target_skeleton", anim.get_editor_property("skeleton"))
        except Exception as e:
            results.append("WARN target_skeleton: %s" % str(e)[:100])
        m = at.create_asset(name, pkg, unreal.AnimMontage, f)
        if m is None:
            results.append("FAIL_CREATE %s" % full)
            continue
        saved = eal.save_asset(full)
        results.append("CREATED %s len=%.3f saved=%s" % (full, m.get_play_length(), saved))
    except Exception as e:
        results.append("ERROR %s: %s" % (full, str(e)[:200]))

print("STEP1_BEGIN")
for r in results:
    print(r)
print("STEP1_END")
