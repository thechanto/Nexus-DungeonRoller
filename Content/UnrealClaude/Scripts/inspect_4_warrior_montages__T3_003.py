import unreal, os

out_dir = "C:/Users/chant/OneDrive/Documents/Unreal Projects/Nexus-part-final 5.7/Saved/_t3d_montage"
os.makedirs(out_dir, exist_ok=True)

paths = [
    "/Game/RetargetedAbilities/Warrior/AM_SkyCrusher",
    "/Game/RetargetedAbilities/Warrior/AM_SwordSweep",
    "/Game/RetargetedAbilities/Warrior/AM_ShieldSlam",
    "/Game/RetargetedAbilities/Warrior/AM_LeapSlash",
]

for p in paths:
    m = unreal.EditorAssetLibrary.load_asset(p)
    name = p.split("/")[-1]
    if m is None:
        print("LOAD FAILED", p)
        continue

    task = unreal.AssetExportTask()
    task.set_editor_property("object", m)
    task.set_editor_property("filename", os.path.join(out_dir, name + ".T3D"))
    task.set_editor_property("automated", True)
    task.set_editor_property("prompt", False)
    task.set_editor_property("replace_identical", True)
    ok = unreal.Exporter.run_asset_export_task(task)

    try:
        length = m.get_play_length()
    except Exception as e:
        length = "ERR:%s" % e

    print("=== %s  export_ok=%s  play_length=%s" % (name, ok, length))

    try:
        tracks = unreal.AnimationLibrary.get_animation_notify_track_names(m)
        print("    notify_tracks: %s" % (list(tracks),))
    except Exception as e:
        print("    notify_tracks ERR: %s" % e)

    try:
        evs = unreal.AnimationLibrary.get_animation_notify_events(m)
        print("    notify_event_count: %d" % len(evs))
    except Exception as e:
        print("    notify_events ERR: %s" % e)

print("DONE")
