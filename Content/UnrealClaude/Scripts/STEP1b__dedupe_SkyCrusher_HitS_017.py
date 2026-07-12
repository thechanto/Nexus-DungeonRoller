import unreal
import os

AL = unreal.AnimationLibrary
P = "/Game/RetargetedAbilities/Warrior/AM_SkyCrusher"
m = unreal.load_asset(P)

START_CLS = unreal.load_class(None, "/Game/AnimNotifies/AN_HitScanStart.AN_HitScanStart_C")
END_CLS = unreal.load_class(None, "/Game/AnimNotifies/AN_HitScanEnd.AN_HitScanEnd_C")

print("before: %d notifies" % len(AL.get_animation_notify_events(m)))

for nm in ["HitScanStart", "HitScanEnd"]:
    try:
        AL.remove_animation_notify_events_by_name(m, nm)
    except Exception as e:
        print("remove %s: %s" % (nm, e))

print("after purge: %d notifies" % len(AL.get_animation_notify_events(m)))

AL.add_animation_notify_event(m, "1", 3.25, START_CLS)
AL.add_animation_notify_event(m, "1", 3.45, END_CLS)
print("after re-add: %d notifies" % len(AL.get_animation_notify_events(m)))

# drop the now-unused HitScan track we created earlier (keep the montage tidy)
tracks = [str(t) for t in AL.get_animation_notify_track_names(m)]
if "HitScan" in tracks:
    try:
        AL.remove_animation_notify_track(m, "HitScan")
        print("removed empty 'HitScan' track")
    except Exception as e:
        print("track remove: %s" % e)

print("saved=%s" % unreal.EditorAssetLibrary.save_asset(P, only_if_is_dirty=False))

OUT = "C:/Users/chant/OneDrive/Documents/Unreal Projects/Nexus-part-final 5.7/ClaudeScripts/t3d"
task = unreal.AssetExportTask()
task.set_editor_property("object", m)
task.set_editor_property("filename", os.path.join(OUT, "AM_SkyCrusher.T3D").replace("\\", "/"))
task.set_editor_property("automated", True)
task.set_editor_property("replace_identical", True)
task.set_editor_property("prompt", False)
unreal.Exporter.run_asset_export_task(task)
print("re-exported")
