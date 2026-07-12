import unreal
import os

OUT = "C:/Users/chant/OneDrive/Documents/Unreal Projects/Nexus-part-final 5.7/ClaudeScripts/t3d"
if not os.path.isdir(OUT):
    os.makedirs(OUT)

PATHS = [
    "/Game/RetargetedAbilities/Warrior/AM_SkyCrusher",
    "/Game/RetargetedAbilities/Warrior/AM_SwordSweep",
    "/Game/RetargetedAbilities/Warrior/AM_ShieldSlam",
    "/Game/RetargetedAbilities/Warrior/AM_LeapSlash",
    "/Game/Animations/Axe/Montage_MeleeAttack_AxeSwing",
]

for p in PATHS:
    a = unreal.load_asset(p)
    if not a:
        print("MISS %s" % p)
        continue
    fn = os.path.join(OUT, a.get_name() + ".T3D").replace("\\", "/")
    task = unreal.AssetExportTask()
    task.set_editor_property("object", a)
    task.set_editor_property("filename", fn)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_identical", True)
    task.set_editor_property("prompt", False)
    task.set_editor_property("write_empty_files", False)
    ok = unreal.Exporter.run_asset_export_task(task)
    print("%s -> %s  ok=%s  exists=%s" % (a.get_name(), fn, ok, os.path.isfile(fn)))
print("DONE")
