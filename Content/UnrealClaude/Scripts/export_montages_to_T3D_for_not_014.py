import unreal

OUT = "C:/Users/chant/OneDrive/Documents/Unreal Projects/Nexus-part-final 5.7/ClaudeScripts/t3d"

PATHS = [
    "/Game/RetargetedAbilities/Warrior/AM_SkyCrusher",
    "/Game/RetargetedAbilities/Warrior/AM_SwordSweep",
    "/Game/RetargetedAbilities/Warrior/AM_ShieldSlam",
    "/Game/RetargetedAbilities/Warrior/AM_LeapSlash",
    "/Game/Characters/Warrior/Animations/Montage_MeleeAttack_AxeSwing",
]

assets = []
for p in PATHS:
    a = unreal.load_asset(p)
    if a:
        assets.append(a)
    else:
        print("could not load %s" % p)

task = unreal.AssetExportTask()
unreal.AssetTools.export_assets(assets, OUT)
print("exported %d assets to %s" % (len(assets), OUT))
