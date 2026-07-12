import unreal
import os

OUT = "C:/Users/chant/OneDrive/Documents/Unreal Projects/Nexus-part-final 5.7/ClaudeScripts/t3d"

bp = unreal.load_asset("/Game/VFX/CameraShakes/BP_CameraShake_Hit_Enemy")
cls = unreal.load_class(None, "/Game/VFX/CameraShakes/BP_CameraShake_Hit_Enemy.BP_CameraShake_Hit_Enemy_C")
cdo = unreal.get_default_object(cls)
print("CDO: %s" % cdo)

for p in ["root_shake_pattern", "camera_shake_scale", "single_instance"]:
    try:
        v = cdo.get_editor_property(p)
        print("  %s = %s" % (p, v))
        if p == "root_shake_pattern" and v:
            print("    pattern class: %s" % v.get_class().get_name())
            print("    pattern text : %s" % v.export_text())
    except Exception as e:
        print("  %s -> %s" % (p, e))

task = unreal.AssetExportTask()
task.set_editor_property("object", bp)
task.set_editor_property("filename", os.path.join(OUT, "BP_CameraShake_Hit_Enemy.T3D").replace("\\", "/"))
task.set_editor_property("automated", True)
task.set_editor_property("replace_identical", True)
task.set_editor_property("prompt", False)
print("export ok=%s" % unreal.Exporter.run_asset_export_task(task))
